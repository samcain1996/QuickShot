#include "Scale.h"

/* ----- Pixel Map ----- */

const size_t CoordinateToIndex(const Resolution& res, const Coordinate& coord) {
    return coord.second * res.width + coord.first;
}

const Coordinate IndexToCoordinate(const Resolution& res, const size_t index) {
    return { index % res.width , index / res.width };
}

const Pixel GetPixel(PixelData& data, const size_t index, const bool isAbsoluteIndex) {
    const size_t idx = isAbsoluteIndex ? index : ConvertIndex(index);
    return Pixel{data}.subspan(idx, NUM_COLOR_CHANNELS);
}

const ConstPixel GetPixel(const PixelData& data, const size_t index, const bool isAbsoluteIndex) {
    const size_t idx = isAbsoluteIndex ? index : ConvertIndex(index);
    return ConstPixel{data}.subspan(idx, NUM_COLOR_CHANNELS);
}

void AssignPixel(Pixel& assignee, const ConstPixel& other) {
    for (size_t channel = 0; channel < NUM_COLOR_CHANNELS; ++channel) {
        assignee[channel] = other[channel];
    }
}

const size_t ConvertIndex(const size_t index, const bool toAbsoluteIndex) {
    return toAbsoluteIndex ? index * NUM_COLOR_CHANNELS : index / NUM_COLOR_CHANNELS;
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
const ScaleRatio Scaler::GetScaleRatio(const Resolution& source, const Resolution& dest) {
    return { (dest.width / (double)source.width), (dest.height / (double)source.height) };
}

// Upscale using nearest neighbor technique
PixelData Scaler::NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest) {

    PixelData scaled(CalculateBMPFileSize(dest));
    const auto& [scaleX, scaleY] = GetScaleRatio(src, dest);

    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += NUM_COLOR_CHANNELS) {

        // Convert pixel index to x,y coordinates
        const auto& [destX, destY] = IndexToCoordinate(dest, ConvertIndex(absIndex, false));
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

    // Init new image and get scale between new and source
    PixelData scaledImg(CalculateBMPFileSize(dest));
    const auto& [scaleX, scaleY] = GetScaleRatio(src, dest);

    const int X_MAX_SRC = src.width - 1;
    const int Y_MAX_SRC = src.height - 1;
    
    for (size_t absIndex = 0; absIndex < scaledImg.size(); absIndex += NUM_COLOR_CHANNELS) {

        // Each pixel is multiple bytes long. Each element in PixelMap is 
        // 1 byte so the 'pixelIndex' needs to be converted to the 'true' index
        const size_t pixelIndex = ConvertIndex(absIndex, false);

        // Get the pixel's X and Y coordinates
        const auto& [scaledImgX, scaledImgY] = IndexToCoordinate(dest, pixelIndex);

        // Location of current pixel if it was in source image
        double x = scaledImgX / scaleX;
        double y = scaledImgY / scaleY;

        // Nearest pixels X-values and their weight in relation to current pixel
        int x_l = floor(scaledImgX / scaleX);  
        int x_h = std::min((int)ceil(scaledImgX / scaleX), X_MAX_SRC); 

        double xl_weight = 1 - (x - x_l) / X_MAX_SRC;
        double xh_weight = 1 - xl_weight;

        // Nearest pixels Y-values and their weight in relation to current pixel
        int y_l = floor(scaledImgY / scaleY);
        int y_h = std::min((int)ceil(scaledImgY / scaleY), Y_MAX_SRC);

        double yl_weight = 1 - (y - y_l) / Y_MAX_SRC;
        double yh_weight = 1 - yl_weight;

        // 4 neighboring pixels ( p_xy )
        const ConstPixel p_ll = GetPixel(source, CoordinateToIndex(src, { x_l, y_l }), false);
        const ConstPixel p_lh = GetPixel(source, CoordinateToIndex(src, { x_l, y_h }), false);
        const ConstPixel p_hl = GetPixel(source, CoordinateToIndex(src, { x_h, y_l }), false);
        const ConstPixel p_hh = GetPixel(source, CoordinateToIndex(src, { x_h, y_h }), false);

        // Create new pixel from neighboring 4 pixels
        Pixel scaledImgPixel = GetPixel(scaledImg, absIndex);

        for (size_t channel = 0; channel < NUM_COLOR_CHANNELS; ++channel) {
            scaledImgPixel[channel] = (xl_weight * p_ll[channel] + xh_weight * p_hl[channel]) * yl_weight +
                (xl_weight * p_lh[channel] + xh_weight * p_hh[channel]) * yh_weight;
        }

    }

    return scaledImg;
}


// TODO: Implement other scaling methods
PixelData Scaler::Bicubic(const PixelData& source, const Resolution& src, const Resolution& dest) {
    return PixelData();
}
PixelData Scaler::Lanczos(const PixelData& source, const Resolution& src, const Resolution& dest) {
    return PixelData();
}

/* ------------------ */