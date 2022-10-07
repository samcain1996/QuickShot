#include "Capture.h"

enum class ScaleMethod {
    NearestNeighbor,
    Bilinear,
    Bicubic,
    Lanczos
};

struct Pixel {
    char rgba[BMP_COLOR_CHANNELS];

    Pixel() { std::memset(rgba, '\0', 4); }
	Pixel(char red, char green, char blue, char alpha) : rgba{ red, green, blue, alpha } {}
    Pixel(char channels[BMP_COLOR_CHANNELS]) { std::memcpy(rgba, channels, BMP_COLOR_CHANNELS); }

    char* ConvertToBytes() { return &rgba[0]; }
};

using PixelList = std::vector<Pixel>;

struct PixelMap {
    PixelList pixels;
    Resolution res;

	const size_t& ImageSize = pixels.size() * BMP_COLOR_CHANNELS;
	
    PixelMap() = delete;
    PixelMap(const Resolution& res) : res(res), pixels(ScreenCapture::CalculateBMPFileSize(res) / BMP_COLOR_CHANNELS) {}

    static inline const size_t ConvertPixelIdxToByteIdx(const size_t pixelIdx, const size_t channels = BMP_COLOR_CHANNELS) {
		return pixelIdx * channels;
    }

    static inline const size_t ConvertByteIdxToPixelIdx(const size_t byteIdx, const size_t channels = BMP_COLOR_CHANNELS) {
		return byteIdx / channels;
    }
};

class Upscaler {

public:

    static inline ScaleMethod scaleMethod = ScaleMethod::NearestNeighbor;

    static inline const bool Upscale(const char* const sourceImage, char* upscaled,
        const Resolution& sourceResolution, const Resolution& destResolution) {

        // If the resolutions are the same, don't scale
        if (sourceResolution == destResolution) [[unlikely]] {
            std::memcpy(upscaled, sourceImage, ScreenCapture::CalculateBMPFileSize(sourceResolution));
            return true;
        }
		
		// If source resolution is larger, don't scale (for now)
        else if (sourceResolution > destResolution) [[unlikely]] { return false; }
		
		// Scale based upon the scale method
        switch (scaleMethod) {
		[[likely]] case ScaleMethod::NearestNeighbor:
			return NearestNeighbor(sourceImage, upscaled, sourceResolution, destResolution);
		case ScaleMethod::Bilinear:
			return Bilinear(sourceImage, upscaled, sourceResolution, destResolution);
		case ScaleMethod::Bicubic:
			return Bicubic(sourceImage, upscaled, sourceResolution, destResolution);
		case ScaleMethod::Lanczos:
			return Lanczos(sourceImage, upscaled, sourceResolution, destResolution);
            default:
                return false;
        }


    }

public:

    // Convert a bitmap to a list of pixels
    const static inline PixelMap BitmapToPixelMap(char* image, const Resolution& res, 
        const Ushort channels = 4, const bool isWindows = false) {
        
        if (!isWindows) { return PixelMap(res); }

        PixelMap pixelMap(res);

        for (size_t pixelIdx = 0; pixelIdx < pixelMap.ImageSize; pixelIdx += channels) {
			
            pixelMap.pixels[pixelIdx / channels] = Pixel(&image[pixelIdx]);
        }

        return pixelMap;
    
    }

    const static inline void ConvertFromPixelMap(PixelMap& map, char* image, 
        const Ushort channels = 4, const bool isWindows = false) {
		
        if (!isWindows) { return; }

        for (size_t pixelIdx = 0; pixelIdx < map.pixels.size(); pixelIdx++) {
			const Pixel& pixel = map.pixels[pixelIdx];
			
			image[pixelIdx * channels + 0] = pixel.rgba[0];
            image[pixelIdx * channels + 1] = pixel.rgba[1];
            image[pixelIdx * channels + 2] = pixel.rgba[2];
            image[pixelIdx * channels + 3] = pixel.rgba[3];
        }
    }

    const static inline char* ConvertFromPixelMap(PixelMap& map,
        const Ushort channels = 4, const bool isWindows = false) {

        if (!isWindows) { return nullptr; }

		char* image = new char[map.pixels.size() * channels];

        for (size_t pixelIdx = 0; pixelIdx < map.pixels.size(); pixelIdx++) {
            const Pixel& pixel = map.pixels[pixelIdx];

            for (size_t channelIdx = 0; channelIdx < channels; channelIdx++) {
                image[pixelIdx * channels + channelIdx] = pixel.rgba[channelIdx];
            }
        }
    }
	
    const static inline bool NearestNeighbor(const char* const source, char* upscaled, const Resolution& srcRes, const Resolution& destRes) {
        return false;
    }
    const static inline bool Bilinear(const char* const source, char* upscaled, const Resolution& srcRes, const Resolution& destRes) {
        return false;
    }
    const static inline bool Bicubic(const char* const source, char* upscaled, const Resolution& srcRes, const Resolution& destRes) {
        return false;
    }
    const static inline bool Lanczos(const char* const source, char* upscaled, const Resolution& srcRes, const Resolution& destRes) {
        return false;
    }

};