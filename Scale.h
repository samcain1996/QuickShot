#pragma once

#include "Capture.h"

// X and Y positions of a pixel
using Coordinate = std::pair<Ushort, Ushort>;

struct Pixel {
    char rgba[BMP_COLOR_CHANNELS];

    Pixel() { std::memset(rgba, '\0', BMP_COLOR_CHANNELS); }
	Pixel(const char red, const char green, const char blue, const char alpha) : rgba{ red, green, blue, alpha } {}
    Pixel(const char channels[BMP_COLOR_CHANNELS]) { std::memcpy(rgba, channels, BMP_COLOR_CHANNELS); }

   Pixel(const Pixel& other) { std::memcpy(rgba, other.rgba, BMP_COLOR_CHANNELS); };
   Pixel(Pixel&& other) {
       std::memcpy(rgba, other.rgba, BMP_COLOR_CHANNELS);
       std::memset(other.rgba, '\0', BMP_COLOR_CHANNELS);
   };

   Pixel& operator=(const Pixel& other) { std::memcpy(rgba, other.rgba, BMP_COLOR_CHANNELS); return *this; };
};

using PixelList = std::vector<Pixel>;

struct PixelMap {
	
    PixelList pixels;
    Resolution res;

	const size_t ImageSize() const { return pixels.size() * BMP_COLOR_CHANNELS; }
	
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

    static const bool Scale(const char* sourceImage, char*& scaled,
        const Resolution& sourceResolution, const Resolution& destResolution);

private:

    // Class shouldn't be instantiated
    Scaler() = delete;
    ~Scaler() = delete;

    // Convert a bitmap to a list of pixels
    const static PixelMap BitmapToPixelMap(const char* image, const Resolution& res);

	// Convert a list of pixels to a pre-allocated bitmap
    const static void ConvertFromPixelMap(const PixelMap& map, char*& image);

    // Convert a list of pixels to a new bitmap
    static char* const ConvertFromPixelMap(const PixelMap& map);

    // Get the ratio in the x-direction between dest and source images
	static const double ScaleRatioX(const Resolution& source, const Resolution& dest);
	
    // Get the ratio in the y-direction between dest and source images
    static const double ScaleRatioY(const Resolution& source, const Resolution& dest);
	// Get the ratio in the x and y directions between dest and source images
	static const ScaleRatio GetScaleRatio(const Resolution& source, const Resolution& dest);

    /* ----- Scaling Function ----- */

	// Upscale using nearest neighbor technique
    const static bool NearestNeighbor(const char* source, char*& upscaled, const Resolution& src, const Resolution& dest);
	
    // TODO: Implement other scaling methods
    const static bool Bilinear(const char* const source, char* upscaled, const Resolution& src, const Resolution& dest);
    const static bool Bicubic(const char* const source, char* upscaled, const Resolution& src, const Resolution& dest);
    const static bool Lanczos(const char* const source, char* upscaled, const Resolution& src, const Resolution& dest);

};