#include "Scale.h"


/* ----- Pixel Map ----- */

// Convert 1-D index to 2-D coordinate
const size_t PixelMap::GetPixelIndex(const Resolution& res, const Coordinate& coord) {
    return coord.second * res.width + coord.first;
}

// Convert 2-D coordinate to 1-D index
const Coordinate PixelMap::GetCoordinate(const Resolution& res, const size_t index) {
    return { index % res.width , index / res.width };
}

const size_t PixelMap::GetDistance(const Coordinate& coord1, const Coordinate& coord2) {
    return sqrt(pow(coord1.first - coord2.first, 2) + pow(coord1.second - coord2.second, 2));
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
        switch (scaleMethod) {
        [[likely]] case ScaleMethod::NearestNeighbor:
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

// Get the ratio in the x-direction between dest and source images
const double Scaler::ScaleRatioX(const Resolution& source, const Resolution& dest) {
    return (dest.width - 1) / (double)(source.width - 1);
}

// Get the ratio in the y-direction between dest and source images
const double Scaler::ScaleRatioY(const Resolution& source, const Resolution& dest) {
    return (dest.height - 1) / (double)(source.height - 1);
}
// Get the ratio in the x and y directions between dest and source images
const ScaleRatio Scaler::GetScaleRatio(const Resolution& source, const Resolution& dest) {
    return std::make_pair(ScaleRatioX(source, dest), ScaleRatioY(source, dest));
}

// Upscale using nearest neighbor technique
PixelData Scaler::NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest) {

    PixelData scaled(CalculateBMPFileSize(dest));
    const auto& [scaleX, scaleY] = GetScaleRatio(src, dest);

    for (size_t byteIdx = 0; byteIdx < scaled.size(); byteIdx += BMP_COLOR_CHANNELS) {

        // Convert pixel index to x,y coordinates
        const auto& [destX, destY] = PixelMap::GetCoordinate(dest, byteIdx / BMP_COLOR_CHANNELS);
		auto scaledPixel = PixelMap::GetPixel(scaled, byteIdx);

        // Scale the coordinates
        const Coordinate mappedCoord = std::make_pair(destX / scaleX, destY / scaleY);

        // Convert the coordinates to index
        const size_t indexToMap = PixelMap::GetPixelIndex(src, mappedCoord) * BMP_COLOR_CHANNELS;
		auto sourcePixel = PixelMap::GetPixel(source, indexToMap);

		scaledPixel[0] = sourcePixel[0];
		scaledPixel[1] = sourcePixel[1];
		scaledPixel[2] = sourcePixel[2];
		scaledPixel[3] = sourcePixel[3];
    }

    // Convert to byte pointer
    return scaled;
}

// TODO: Implement other scaling methods
PixelData Scaler::Bilinear(const PixelData& source, const Resolution& src, const Resolution& dest) {

    PixelData scaled(CalculateBMPFileSize(dest));
    const auto& [scaleX, scaleY] = GetScaleRatio(src, dest);
    
    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += BMP_COLOR_CHANNELS) {

        // Each pixel is multiple bytes long. Each element in PixelMap is 
        // 1 byte so the 'pixelIndex' needs to be converted to the 'true' index
        const size_t pixelIndex = PixelMap::ToPixelIdx(absIndex);

        // Get the pixel's X and Y coordinates
        const auto& [scaledX, scaledY] = PixelMap::GetCoordinate(dest, pixelIndex);

        // Location of current pixel if it was in source image
        double x = scaledX / scaleX;
        double y = scaledY / scaleY;

        // Nearest pixels X-values and their weight in relation to current pixel
        double x_l = floor(scaledX / scaleX);
        double x_h = ceil(scaledX / scaleX);

        double xl_weight = 1 - (x - x_l) / scaleX;
        double xh_weight = 1 - xl_weight;

        // Nearest pixels Y-values and their weight in relation to current pixel
        double y_l = floor(scaledY / scaleY);
        double y_h = ceil(scaledY / scaleY);

        double yl_weight = 1 - (y - y_l) / scaleY;
        double yh_weight = 1 - yl_weight;

        const auto p_ll = PixelMap::GetPixel(source, PixelMap::GetPixelIndex(src, { x_l, y_l }), false);
        const auto p_lh = PixelMap::GetPixel(source, PixelMap::GetPixelIndex(src, { x_l, y_h }), false);
        const auto p_hl = PixelMap::GetPixel(source, PixelMap::GetPixelIndex(src, { x_h, y_l }), false);
        const auto p_hh = PixelMap::GetPixel(source, PixelMap::GetPixelIndex(src, { x_h, y_h }), false);

        char r1 = xl_weight * p_ll[0] + xh_weight * p_hl[0];
        char r2 = xl_weight * p_lh[0] + xh_weight * p_hh[0];

        char g1 = xl_weight * p_ll[1] + xh_weight * p_hl[1];
        char g2 = xl_weight * p_lh[1] + xh_weight * p_hh[1];

        char b1 = xl_weight * p_ll[2] + xh_weight * p_hl[2];
        char b2 = xl_weight * p_lh[2] + xh_weight * p_hh[2];

        char a1 = xl_weight * p_ll[3] + xh_weight * p_hl[3];
        char a2 = xl_weight * p_lh[3] + xh_weight * p_hh[3];

        auto scaledPixel = PixelMap::GetPixel(scaled, absIndex);

        scaledPixel[0] = r1 * yl_weight + r2 * yh_weight;
        scaledPixel[1] = g1 * yl_weight + g2 * yh_weight;
        scaledPixel[2] = b1 * yl_weight + b2 * yh_weight;
        scaledPixel[3] = a1 * yl_weight + a2 * yh_weight;

    }

    return scaled;
}



PixelData Scaler::Bicubic(const PixelData& source, const Resolution& src, const Resolution& dest) {
    return PixelData();
}
PixelData Scaler::Lanczos(const PixelData& source, const Resolution& src, const Resolution& dest) {
    return PixelData();
}

/* ------------------ */