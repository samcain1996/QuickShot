#pragma once

#include "TypesAndDefs.h"

// X and Y positions of a pixel
using Coordinate = std::pair<Ushort, Ushort>;

using Pixel = std::span<Byte>;
using ConstPixel = std::span<const Byte>;

namespace PixelMap {

    // Convert 1-D index to 2-D coordinate
    static const size_t GetPixelIndex(const Resolution& res, const Coordinate& coord);

    // Convert 2-D coordinate to 1-D index
    static const Coordinate GetCoordinate(const Resolution& res, const size_t index);

    // Convert individual byte to index of pixel map
    static const size_t ToPixelIdx(const size_t absoluteIdx);

    // Convert index of pixel to index of individual byte
    static const size_t ToAbsoluteIdx(const size_t pixelIdx);

    // Returns pixel at index of data
    static const Pixel GetPixel(PixelData& data, const size_t index, const bool absoluteIndex = true);

    // Returns a const pixel at index of data
    static const ConstPixel GetPixel(const PixelData& data, const size_t index, const bool absoluteIndex = true);

};

// Scale between two images in x and y directions ( new / old )
struct ScaleRatio {
    double xRatio = 1;
    double yRatio = 1;

    ScaleRatio(const double x, const double y) : xRatio(x), yRatio(y) {}
    ScaleRatio(const std::pair<double, double>& ratio) : xRatio(ratio.first), yRatio(ratio.second) {}
    ScaleRatio(const Resolution& ratio) : xRatio(ratio.width), yRatio(ratio.height) {}
};


class Scaler {

public:

    // Supported scaling methods
    enum class ScaleMethod {
        NearestNeighbor,
        Bilinear,
        Bicubic,  // Not implemented
        Lanczos   // Not implemented
    };

    static inline ScaleMethod method = ScaleMethod::NearestNeighbor;

    static PixelData Scale(const PixelData& sourceImage,
        const Resolution& sourceResolution, const Resolution& destResolution);

    static PixelData Scale(const PixelData& sourceImage,
        const Resolution& sourceResolution, const ScaleRatio& scaleRatio);

private:

    // Class shouldn't be instantiated, it is static
    Scaler() = delete;
    ~Scaler() = delete;

    // Get the ratio in the x and y directions between dest and source images
    static const ScaleRatio GetScaleRatio(const Resolution& source, const Resolution& dest);

    /* ----- Scaling Function ----- */

    // Upscale using nearest neighbor ( blockiest results )
    static PixelData NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest);
    // Upscale by linearly interpolating pixel values ( blurry )
    static PixelData Bilinear(const PixelData& source, const Resolution& src, const Resolution& dest);

    // TODO: Implement other scaling methods
    static PixelData Bicubic(const PixelData& source, const Resolution& src, const Resolution& dest);
    static PixelData Lanczos(const PixelData& source, const Resolution& src, const Resolution& dest);

};