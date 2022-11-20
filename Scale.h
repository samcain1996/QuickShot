#pragma once

#include "TypesAndDefs.h"
#include <cmath>
#include <algorithm>


// X and Y positions of a pixel
using Coordinate = std::pair<Ushort, Ushort>;
using Pixel = std::span<char>;
using ConstPixel = std::span<const char>;

namespace PixelMap {

    // Convert 1-D index to 2-D coordinate
    static const size_t GetPixelIndex(const Resolution& res, const Coordinate& coord);

    // Convert 2-D coordinate to 1-D index
    static const Coordinate GetCoordinate(const Resolution& res, const size_t index);

    static const size_t GetDistance(const Coordinate& coord1, const Coordinate& coord2);

    static const size_t ToPixelIdx(const size_t absoluteIdx);
    static const size_t ToAbsoluteIdx(const size_t pixelIdx);

    static const Pixel GetPixel(PixelData& data, const size_t index, const bool absoluteIndex = true);

    static const ConstPixel GetPixel(const PixelData& data, const size_t index, const bool absoluteIndex = true);

};

// Scale between two images in x and y directions
using ScaleRatio = std::pair<double, double>;

// Supported scaling methods
enum class ScaleMethod {
    NearestNeighbor,
    Bilinear,
    Bicubic,
    Lanczos
};

class Scaler {

public:

    static inline ScaleMethod scaleMethod = ScaleMethod::NearestNeighbor;

    static PixelData Scale(const PixelData& sourceImage,
        const Resolution& sourceResolution, const Resolution& destResolution);

private:

    // Class shouldn't be instantiated
    Scaler() = delete;
    ~Scaler() = delete;

    // Get the ratio in the x-direction between dest and source images
    static const double ScaleRatioX(const Resolution& source, const Resolution& dest);

    // Get the ratio in the y-direction between dest and source images
    static const double ScaleRatioY(const Resolution& source, const Resolution& dest);
    // Get the ratio in the x and y directions between dest and source images
    static const ScaleRatio GetScaleRatio(const Resolution& source, const Resolution& dest);

    /* ----- Scaling Function ----- */

    // Upscale using nearest neighbor technique
    static PixelData NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest);
    static PixelData Bilinear(const PixelData& source, const Resolution& src, const Resolution& dest);

    // TODO: Implement other scaling methods
    static PixelData Bicubic(const PixelData& source, const Resolution& src, const Resolution& dest);
    static PixelData Lanczos(const PixelData& source, const Resolution& src, const Resolution& dest);

};