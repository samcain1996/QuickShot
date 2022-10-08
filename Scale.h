#include "Capture.h"

// Supported scaling methods
enum class ScaleMethod {
    NearestNeighbor,
    Bilinear,
    Bicubic,
    Lanczos
};

// X and Y positions of a pixel
using Coordinate = std::pair<Ushort, Ushort>;

struct Pixel {
    char rgba[BMP_COLOR_CHANNELS];

    Pixel() { std::memset(rgba, '\0', BMP_COLOR_CHANNELS); }
	Pixel(char red, char green, char blue, char alpha) : rgba{ red, green, blue, alpha } {}
    Pixel(char channels[BMP_COLOR_CHANNELS]) { std::memcpy(rgba, channels, BMP_COLOR_CHANNELS); }
};

using PixelList = std::vector<Pixel>;

struct PixelMap {
	
    PixelList pixels;
    Resolution res;

	const size_t& ImageSize = pixels.size() * BMP_COLOR_CHANNELS;
	
    PixelMap() = delete;
    PixelMap(const Resolution& res) : res(res), pixels(ScreenCapture::CalculateBMPFileSize(res) / BMP_COLOR_CHANNELS) {}

	PixelMap(const PixelMap& other) : res(other.res), pixels(other.pixels) {}
	PixelMap(PixelMap&& other) noexcept { res = std::move(other.res); pixels = std::move(other.pixels); }
	
    const Pixel* const GetPixel(const Coordinate& coord) const {
		// If coordinate is on edge of image, return nullptr
        if (coord.first < 0 || coord.second < 0) { return nullptr; }
        else if (coord.first >= res.width || coord.second >= res.height) { return nullptr; }

        return &pixels[coord.second * res.width + coord.first];
    }

	// Convert 1-D index to 2-D coordinate
    const size_t GetPixelIndex(const Coordinate& coord) const {
        return coord.second * res.width + coord.first;
    }

	// Convert 2-D coordinate to 1-D index
	const Coordinate GetCoordinate(const size_t index) const {
        return { index % res.width , index / res.width };
	}
	
    // Convert 2-D coordinate to 1-D index
    static const Coordinate GetCoordinate(const Resolution& res, const size_t index) {
        return { index % res.width , index / res.width };
    }

};

// Scale between two images in x and y directions
using ScaleRatio = std::pair<double, double>;

class Scaler {

public:

    static inline ScaleMethod scaleMethod = ScaleMethod::NearestNeighbor;

    static inline const bool Upscale(char* sourceImage, char*& upscaled,
        const Resolution& sourceResolution, const Resolution& destResolution) {

        // If the resolutions are the same, don't scale
        if (sourceResolution == destResolution) [[unlikely]] {
            std::memcpy(upscaled, sourceImage, ScreenCapture::CalculateBMPFileSize(sourceResolution));
            return true;
        }

        const ScaleRatio& ratio = GetScaleRatio(sourceResolution, destResolution);
		
		// Scale based upon the scale method
        switch (scaleMethod) {
		[[likely]] case ScaleMethod::NearestNeighbor:
			return NearestNeighbor(sourceImage, upscaled, sourceResolution, destResolution);
		case ScaleMethod::Bilinear:
			return Bilinear(sourceImage, upscaled, ratio);
		case ScaleMethod::Bicubic:
			return Bicubic(sourceImage, upscaled, ratio);
		case ScaleMethod::Lanczos:
			return Lanczos(sourceImage, upscaled, ratio);
        default:
            return false;
        }

    }

private:

    // Convert a bitmap to a list of pixels
    const static inline PixelMap BitmapToPixelMap(char* image, const Resolution& res, 
        const bool isWindows = false) {
        
        if (!isWindows) { return PixelMap(res); }

        PixelMap pixelMap(res);

        for (size_t pixelIdx = 0; pixelIdx < pixelMap.ImageSize; pixelIdx += BMP_COLOR_CHANNELS) {
            pixelMap.pixels[pixelIdx / BMP_COLOR_CHANNELS] = Pixel(&image[pixelIdx]);
        }

        return pixelMap;
    
    }

	// Convert a list of pixels to a pre-allocated bitmap
    const static inline void ConvertFromPixelMap(const PixelMap& map, char*& image, 
        const bool isWindows = false) {
		
		if (!isWindows) { return; }  // TODO: Implement for Linux and Mac

        for (size_t pixelIdx = 0; pixelIdx < map.pixels.size(); pixelIdx++) {
			const Pixel& pixel = map.pixels[pixelIdx];
			
			image[pixelIdx * BMP_COLOR_CHANNELS + 0] = pixel.rgba[0];
            image[pixelIdx * BMP_COLOR_CHANNELS + 1] = pixel.rgba[1];
            image[pixelIdx * BMP_COLOR_CHANNELS + 2] = pixel.rgba[2];
            image[pixelIdx * BMP_COLOR_CHANNELS + 3] = pixel.rgba[3];
        }
    }

    // Convert a list of pixels to a new bitmap
    static inline char* const ConvertFromPixelMap(const PixelMap& map, const bool isWindows = false) {

		char* image = new char[map.pixels.size() * BMP_COLOR_CHANNELS];

        ConvertFromPixelMap(map, image, isWindows);

        return image;
    }

    // Get the ratio in the x-direction between dest and source images
	static inline const double ScaleRatioX(const Resolution& source, const Resolution& dest) {
		return (double)dest.width / (double)source.width;
	}
	
    // Get the ratio in the y-direction between dest and source images
    static inline const double ScaleRatioY(const Resolution& source, const Resolution& dest) {
        return (double)dest.height / (double)source.height;
    }

	// Get the ratio in the x and y directions between dest and source images
	static inline const ScaleRatio GetScaleRatio(const Resolution& source, const Resolution& dest) {
		return std::make_pair(ScaleRatioX(source, dest), ScaleRatioY(source, dest));
	}
	
	// Upscale using nearest neighbor technique
    const static inline bool NearestNeighbor(char* source, char*& upscaled, const Resolution& src, const Resolution& dest) {

        PixelMap sourcePixels = BitmapToPixelMap(source, src, true);
        PixelMap destPixels(dest);

		const auto& [ratioX, ratioY] = GetScaleRatio(src, dest);

        for (size_t destPixelIdx = 0; destPixelIdx < destPixels.pixels.size(); ++destPixelIdx) {
			
            // Get the coordinates of current pixel
            const auto& [destX, destY] = destPixels.GetCoordinate(destPixelIdx);

			// Find corresponding pixel in source image
            const Coordinate& mappedCoord = std::make_pair(destX / ratioX, destY / ratioY);

			const Pixel* const pPixel = sourcePixels.GetPixel(mappedCoord);

            // Match the current pixel data with the pixel data of the mapped source pixel
            destPixels.pixels[destPixelIdx] = *pPixel;
		
        }
    
        // Convert to byte pointer
		upscaled = ConvertFromPixelMap(destPixels, true);

        return true;
    }
	
    // TODO: Implement other scaling methods
    const static inline bool Bilinear(const char* const source, char* upscaled, const ScaleRatio& scaleRatio) {
        return false;
    }
    const static inline bool Bicubic(const char* const source, char* upscaled, const ScaleRatio& scaleRatio) {
        return false;
    }
    const static inline bool Lanczos(const char* const source, char* upscaled, const ScaleRatio& scaleRatio) {
        return false;
    }

};