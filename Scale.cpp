#include "Scale.h"


/* ----- Pixel Map ----- */

const size_t CoordinateToIndex(const Resolution& res, const Coordinate& coord) {
    return coord.second * res.width + coord.first;
}

const Coordinate IndexToCoordinate(const Resolution& res, const size_t index) {
    return { index % res.width , index / res.width };
}

Pixel GetPixel(PixelData& data, const size_t index, const bool isAbsoluteIndex) {
    const size_t idx = isAbsoluteIndex ? index : ConvertIndex(index);
    return Pixel{data}.subspan(idx, BYTES_PER_PIXEL);
}

ConstPixel GetPixel(const PixelData& data, const size_t index, const bool isAbsoluteIndex) {
    const size_t idx = isAbsoluteIndex ? index : ConvertIndex(index);
    return ConstPixel{data}.subspan(idx, BYTES_PER_PIXEL);
}

void AssignPixel(Pixel& assignee, const ConstPixel& other) {
    for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
        assignee[channel] = other[channel];
    }
}

//void SubtractPixel(Pixel& subFrom, const ConstPixel& sub) {
//    for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
//        subFrom[channel] -= sub[channel];
//    }
//}

void AssignPixel(Thing& assignee, const ConstPixel& other) {
    for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
        assignee[channel] = other[channel];
    }
}

void SubtractPixel(Thing& subFrom, const ConstPixel& sub) {
    for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
        subFrom[channel] -= sub[channel];
    }
}

Thing SubtractPixel(const ConstPixel& subFrom, const ConstPixel& sub) {
    Thing t;
    for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
        t[channel] = subFrom[channel] - sub[channel];
    }
    return t;
}

const size_t ConvertIndex(const size_t index, const bool toAbsoluteIndex) {
    return toAbsoluteIndex ? index * BYTES_PER_PIXEL : index / BYTES_PER_PIXEL;
}


/* --------------------- */

/* ----- Scaler ----- */

PixelData Scaler::Scale(const PixelData& sourceImage,
    const Resolution& sourceResolution, const Resolution& destResolution) {

    // If the resolutions are the same, don't scale
    if (sourceResolution == destResolution) [[unlikely]] {
        return sourceImage;
    }

    // Scale based upon the scale method
    switch (method) {
    case ScaleMethod::NearestNeighbor:
        return NearestNeighbor(sourceImage, sourceResolution, destResolution);
    case ScaleMethod::Bilinear:
        return Bilinear(sourceImage, sourceResolution, destResolution);
    case ScaleMethod::Bicubic:
        return Bicubic(sourceImage, sourceResolution, destResolution);
    case ScaleMethod::Lanczos:
        return Lanczos(sourceImage, sourceResolution, destResolution);
    default:
        return PixelData();
    }

}

PixelData Scaler::Scale(const PixelData& sourceImage,
    const Resolution& sourceResolution, const ScaleRatio& scaleRatio) {

    return Scale(sourceImage, sourceResolution, 
        Resolution { sourceResolution.width * scaleRatio.xRatio, sourceResolution.height * scaleRatio.yRatio });
}

PixelData Scaler::Scale(const PixelData& sourceImage, const Resolution& sourceResolution, const Uint32 scalingFactor) {
    return Scale(sourceImage, sourceResolution, ScaleRatio(scalingFactor, scalingFactor));
}

// Get the ratio in the x and y directions between dest and source images
ScaleRatio Scaler::GetScaleRatio(const Resolution& source, const Resolution& dest) {
    return { (dest.width / (double)source.width), (dest.height / (double)source.height) };
}

Neighbors Scaler::GetNeighbors(const double x, const double y, const PixelData& source,
    const Resolution& src) {

    const int X_MAX_SRC = src.width - 1;
    const int Y_MAX_SRC = src.height - 1;

    const int x_l = floor(x);
    const int x_h = std::min((int)ceil(x), X_MAX_SRC);

    const int y_l = floor(y);
    const int y_h = std::min((int)ceil(y), Y_MAX_SRC);

    return
    {
        PixelAndPos{ GetPixel(source, CoordinateToIndex(src, { x_l, y_l }), false), { x_l, y_l } },
        PixelAndPos{ GetPixel(source, CoordinateToIndex(src, { x_h, y_l }), false), { x_h, y_l } },
        PixelAndPos{ GetPixel(source, CoordinateToIndex(src, { x_l, y_h }), false), { x_l, y_h } },
        PixelAndPos{ GetPixel(source, CoordinateToIndex(src, { x_h, y_h }), false), { x_h, y_h } },
    };

}

// Upscale using nearest neighbor technique
PixelData Scaler::NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest) {

    PixelData scaled(CalculateBMPFileSize(dest));
    const auto [scaleX, scaleY] = GetScaleRatio(src, dest);

    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += BYTES_PER_PIXEL) {

        // Convert pixel index to x,y coordinates
        const auto [destX, destY] = IndexToCoordinate(dest, ConvertIndex(absIndex, false));
		Pixel scaledPixel = GetPixel(scaled, absIndex);

        // Scale the coordinates
        const Coordinate mappedCoord = { destX / scaleX, destY / scaleY };

        // Convert the coordinates to index
        const size_t indexToMap = CoordinateToIndex(src, mappedCoord);
		ConstPixel sourcePixel = GetPixel(source, indexToMap, false);

        // Set scaledPixel equal to corresponding sourcePixel 
        AssignPixel(scaledPixel, sourcePixel);
    }

    return scaled;
}


PixelData Scaler::Bilinear(const PixelData& source, const Resolution& src, const Resolution& dest) {

    using enum Neighbor;

    // Init new image and get scale between new and source
    PixelData scaled(CalculateBMPFileSize(dest));
    const auto [destX, destY] = GetScaleRatio(src, dest);

    const int X_MAX_SRC = src.width - 1;
    const int Y_MAX_SRC = src.height - 1;
    
    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += BYTES_PER_PIXEL) {

        // Get the pixel's X and Y coordinates
        const auto [scaledX, scaledY] = IndexToCoordinate(dest, ConvertIndex(absIndex, false));
        const double x = scaledX / destX;
        const double y = scaledY / destY;

        // Create new pixel from neighboring 4 pixels
        Pixel scaledPixel = GetPixel(scaled, absIndex);
        Neighbors neighbors = GetNeighbors(x, y, source, src);

        const int x_l = floor(x);
        const int y_l = std::min((int)ceil(y), Y_MAX_SRC);

        const double xl_weight = 1 - (x - x_l) / X_MAX_SRC;
        const double xh_weight = 1 - xl_weight;

        const double yl_weight = 1 - (y - y_l) / Y_MAX_SRC;
        const double yh_weight = 1 - yl_weight;

        for (size_t channel = 0; channel < NUM_COLOR_CHANNELS; ++channel) {
            scaledPixel[channel] = 
                (xl_weight * neighbors[(int)TopLeft].first[channel] + xh_weight * neighbors[(int)TopRight].first[channel]) * yl_weight +
                (xl_weight * neighbors[(int)BottomLeft].first[channel] + xh_weight * neighbors[(int)BottomRight].first[channel]) * yh_weight;
        }

    }

    return scaled;
}

PixelData Scaler::Bicubic(const PixelData& source, const Resolution& src, const Resolution& dest) {
    
    using enum Neighbor;

    // Init new image and get scale between new and source
    PixelData scaled(CalculateBMPFileSize(dest));
    const auto [destX, destY] = GetScaleRatio(src, dest);

    const int X_MAX_SRC = src.width - 1;
    const int Y_MAX_SRC = src.height - 1;

    static const Matrix4d multMat1 {
        { 1, 0, 0, 0 },
        { 0, 0, 1, 0 },
        { -3, 3, -2, -1 },
        { 2, -2, 1, 1 }
    };

    static const Matrix4d multMat2 {
        { 1, 0, -3, 2 },
        { 0, 0, 3, -2 },
        { 0, 1, -2, 1 },
        { 0, 0, -1, 1 }
    };

    static const Thing EmptyPixel = { 0, 0, 0, 0 };
    
    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += BYTES_PER_PIXEL) {

        const size_t pixelIndex = ConvertIndex(absIndex, false);

        const auto [scaledX, scaledY] = IndexToCoordinate(dest, pixelIndex);
        const double x = scaledX / destX;
        const double y = scaledY / destY;

        // 4 neighboring pixels ( p_xy )
        Neighbors neighbors = GetNeighbors(x, y, source, src);
        std::array < PixelAndPos, BYTES_PER_PIXEL > xDerivs, yDerivs, xyDerivs;
        int ix = 0, iy = 0, ixy = 0;

        // Calculate derivatives
        std::for_each(neighbors.begin(), neighbors.end(), [X_MAX_SRC, &source, &src, &xDerivs, &ix](const auto& pixelAndPos) {

            const auto& [pixelX, pixelY] = pixelAndPos.second;
            
            const ConstPixel nextPixel = (pixelX >= X_MAX_SRC) ? EmptyPixel : GetPixel(source, CoordinateToIndex(src, { pixelX + 1, pixelY }));
            const ConstPixel prevPixel = (pixelX <= 0) ? EmptyPixel : GetPixel(source, CoordinateToIndex(src, { pixelX - 1, pixelY }));

            xDerivs[ix++] = { SubtractPixel(nextPixel, prevPixel), pixelAndPos.second};
        });
        
        std::for_each(neighbors.begin(), neighbors.end(), [Y_MAX_SRC, &source, &src, &yDerivs, &iy](const auto& pixelAndPos) {

            const auto& [pixelX, pixelY] = pixelAndPos.second;

            const ConstPixel nextPixel = (pixelY >= Y_MAX_SRC) ? EmptyPixel : GetPixel(source, CoordinateToIndex(src, { pixelX, pixelY + 1 }));
            const ConstPixel prevPixel = (pixelY <= 0) ? EmptyPixel : GetPixel(source, CoordinateToIndex(src, {pixelX, pixelY - 1}));

            yDerivs[iy++] = { SubtractPixel(nextPixel, prevPixel), pixelAndPos.second };
        });

        std::for_each(xDerivs.begin(), xDerivs.end(), [Y_MAX_SRC, &source, &src, &yDerivs, &xyDerivs, &ixy](const auto& pixelAndPos) {

            const Coordinate& pos = pixelAndPos.second;
            // Find eq yDeriv
            bool found = false;
            for (const auto& res : yDerivs) { if (res.second.first == pos.first && res.second.second == pos.second) { found = true; break; } }
            if (found) {
                const ConstPixel nextPixel = (pos.second >= Y_MAX_SRC) ? EmptyPixel : GetPixel(source, CoordinateToIndex(src, { pos.first, pos.second + 1 }));
                const ConstPixel prevPixel = (pos.second <= 0) ? EmptyPixel : GetPixel(source, CoordinateToIndex(src, { pos.first, pos.second - 1 }));

                xyDerivs[ixy++] = { SubtractPixel(nextPixel, prevPixel), pixelAndPos.second};
            }

        });


        const std::function<Matrix4d(const int)> functionMatrix = [&neighbors, &xDerivs, &yDerivs, &xyDerivs](const int index) {
            return Matrix4d {
                { (double)neighbors[(int)TopLeft].first[index],  (double)neighbors[(int)BottomLeft].first[index],  (double)yDerivs[(int)TopLeft].first[index],    (double)yDerivs[(int)BottomLeft].first[index]},
                { (double)neighbors[(int)TopRight].first[index], (double)neighbors[(int)BottomRight].first[index], (double)yDerivs[(int)TopRight].first[index],   (double)yDerivs[(int)BottomRight].first[index]},
                { (double)xDerivs[(int)TopLeft].first[index],    (double)xDerivs[(int)BottomLeft].first[index],    (double)xyDerivs[(int)TopLeft].first[index],   (double)xyDerivs[(int)BottomLeft].first[index]},
                { (double)xDerivs[(int)TopRight].first[index],   (double)xDerivs[(int)BottomRight].first[index],   (double)xyDerivs[(int)TopRight].first[index],  (double)xyDerivs[(int)BottomRight].first[index]},
            };
        };

        std::array<Matrix4d, BYTES_PER_PIXEL> coefficients;
        for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
            coefficients[channel] = multMat1 * functionMatrix(channel) * multMat2;
        }

        const double xNormal = scaledX / (dest.width - 1);
        const double yNormal = scaledY / (dest.height - 1);
        const Matrix<double, 1, 4> xVec{ 1, xNormal, pow(xNormal, 2), pow(xNormal, 3) };
        const Matrix<double, 4, 1> yVec{ 1, yNormal, pow(yNormal, 2), pow(yNormal, 3) };

        Pixel scaledPixel = GetPixel(scaled, absIndex);

        for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
            scaledPixel[channel] = (int)(xVec * coefficients[channel] * yVec);
        }

    }



    return scaled;
}


// TODO: Implement other scaling methods
PixelData Scaler::Lanczos(const PixelData& source, const Resolution& src, const Resolution& dest) {
    return PixelData();
}

/* ------------------ */