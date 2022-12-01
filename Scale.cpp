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

const size_t PixelMap::ToPixelIdx(const size_t absoluteIdx) { return absoluteIdx / BMP_COLOR_CHANNELS; }

const size_t PixelMap::ToAbsoluteIdx(const size_t pixelIdx) { return pixelIdx * BMP_COLOR_CHANNELS; }

const Pixel PixelMap::GetPixel(PixelData& data, const size_t index, const bool absoluteIndex) {
    const size_t idx = absoluteIndex ? index : PixelMap::ToAbsoluteIdx(index);
    return Pixel{data}.subspan(idx, BMP_COLOR_CHANNELS);
}

const ConstPixel PixelMap::GetPixel(const PixelData& data, const size_t index, const bool absoluteIndex) {
    const size_t idx = absoluteIndex ? index : PixelMap::ToAbsoluteIdx(index);
    return ConstPixel{data}.subspan(idx, BMP_COLOR_CHANNELS);
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

    return Scaler::Scale(sourceImage, sourceResolution, 
        Resolution { int(sourceResolution.width * scaleRatio.xRatio), int(sourceResolution.height * scaleRatio.yRatio)});

}

// Get the ratio in the x and y directions between dest and source images
const ScaleRatio Scaler::GetScaleRatio(const Resolution& source, const Resolution& dest) {
    return { (dest.width / (double)source.width), (dest.height / (double)source.height) };
}

// Upscale using nearest neighbor technique
PixelData Scaler::NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest) {

    PixelData scaled(CalculateBMPFileSize(dest));
    const auto& [scaleX, scaleY] = GetScaleRatio(src, dest);

    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += BMP_COLOR_CHANNELS) {

        // Convert pixel index to x,y coordinates
        const auto& [destX, destY] = PixelMap::GetCoordinate(dest, PixelMap::ToPixelIdx(absIndex));
		Pixel scaledPixel = PixelMap::GetPixel(scaled, absIndex);

        // Scale the coordinates
        const Coordinate mappedCoord = { destX / scaleX, destY / scaleY };

        // Convert the coordinates to index
        const size_t indexToMap = PixelMap::GetPixelIndex(src, mappedCoord);
		ConstPixel sourcePixel = PixelMap::GetPixel(source, indexToMap, false);

		scaledPixel[0] = sourcePixel[0];
		scaledPixel[1] = sourcePixel[1];
		scaledPixel[2] = sourcePixel[2];
		scaledPixel[3] = sourcePixel[3];
    }

    // Convert to byte pointer
    return scaled;
}


PixelData Scaler::Bilinear(const PixelData& source, const Resolution& src, const Resolution& dest) {

    // Init new image and get scale between new and source
    PixelData scaledImg(CalculateBMPFileSize(dest));
    const auto& [scaleX, scaleY] = GetScaleRatio(src, dest);

    const int X_MAX_SRC = src.width - 1;
    const int Y_MAX_SRC = src.height - 1;
    
    for (size_t absIndex = 0; absIndex < scaledImg.size(); absIndex += BMP_COLOR_CHANNELS) {

        // Each pixel is multiple bytes long. Each element in PixelMap is 
        // 1 byte so the 'pixelIndex' needs to be converted to the 'true' index
        const size_t pixelIndex = PixelMap::ToPixelIdx(absIndex);

        // Get the pixel's X and Y coordinates
        const auto& [scaledImgX, scaledImgY] = PixelMap::GetCoordinate(dest, pixelIndex);

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
        const ConstPixel p_ll = PixelMap::GetPixel(source, PixelMap::GetPixelIndex(src, { x_l, y_l }), false);
        const ConstPixel p_lh = PixelMap::GetPixel(source, PixelMap::GetPixelIndex(src, { x_l, y_h }), false);
        const ConstPixel p_hl = PixelMap::GetPixel(source, PixelMap::GetPixelIndex(src, { x_h, y_l }), false);
        const ConstPixel p_hh = PixelMap::GetPixel(source, PixelMap::GetPixelIndex(src, { x_h, y_h }), false);

        // Create new pixel from neighboring 4 pixels
        Pixel scaledImgPixel = PixelMap::GetPixel(scaledImg, absIndex);

        for (size_t channel = 0; channel < BMP_COLOR_CHANNELS; ++channel) {
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