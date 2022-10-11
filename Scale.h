#pragma once

#include "TypesAndDefs.h"


// X and Y positions of a pixel
using Coordinate = std::pair<Ushort, Ushort>;
using RGBA = std::array<char, BMP_COLOR_CHANNELS>;

struct Pixel {
    RGBA rgba;

    Pixel(); 
    Pixel(const char red, const char green, const char blue, const char alpha);
    Pixel(const char channels[BMP_COLOR_CHANNELS]);

    Pixel(const Pixel& other);
    Pixel(Pixel&& other) noexcept;

    Pixel& operator=(const Pixel& other);
};

using PixelList = std::vector<Pixel>;

struct PixelMap {
	
    PixelList pixels;
    Resolution res;

	const size_t ImageSize() const { return CalculateBMPFileSize(res); }
	
    PixelMap() = delete;
    PixelMap(const Resolution& res);

	PixelMap(const PixelMap& other);
	PixelMap(PixelMap&& other) noexcept;
	
    const Pixel* const GetPixel(const Coordinate& coord) const;

	// Convert 1-D index to 2-D coordinate
    const size_t GetPixelIndex(const Coordinate& coord) const;

	// Convert 2-D coordinate to 1-D index
	const Coordinate GetCoordinate(const size_t index) const;
	
    // Convert 2-D coordinate to 1-D index
    static const Coordinate GetCoordinate(const Resolution& res, const size_t index);
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

    // Convert a bitmap to a list of pixels
    const static PixelMap BitmapToPixelMap(const PixelData& image, const Resolution& res);

    // Convert a list of pixels to a new bitmap
    static PixelData ConvertFromPixelMap(const PixelMap& map);

    // Get the ratio in the x-direction between dest and source images
	static const double ScaleRatioX(const Resolution& source, const Resolution& dest);
	
    // Get the ratio in the y-direction between dest and source images
    static const double ScaleRatioY(const Resolution& source, const Resolution& dest);
	// Get the ratio in the x and y directions between dest and source images
	static const ScaleRatio GetScaleRatio(const Resolution& source, const Resolution& dest);

    /* ----- Scaling Function ----- */

	// Upscale using nearest neighbor technique
    static PixelData NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest);
	
    // TODO: Implement other scaling methods
    static PixelData Bilinear(const PixelData& source, const Resolution& src, const Resolution& dest);
    static PixelData Bicubic(const PixelData& source, const Resolution& src, const Resolution& dest);
    static PixelData Lanczos(const PixelData& source, const Resolution& src, const Resolution& dest);

};