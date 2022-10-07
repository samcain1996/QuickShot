#include "Capture.h"

enum class ScaleMethod {
    NearestNeighbor,
    Bilinear,
    Bicubic,
    Lanczos
};

using Coordinate = std::pair<Ushort, Ushort>;

struct Pixel {
    char rgba[BMP_COLOR_CHANNELS];

    Pixel() { std::memset(rgba, '\0', BMP_COLOR_CHANNELS); }
	Pixel(char red, char green, char blue, char alpha) : rgba{ red, green, blue, alpha } {}
    Pixel(char channels[BMP_COLOR_CHANNELS]) { std::memcpy(rgba, channels, BMP_COLOR_CHANNELS); }

};

using PixelList = std::vector<Pixel>;
// using PixelSet = std::unordered_set<Pixel>;

struct PixelMap {
    PixelList pixels;
    Resolution res;

	const size_t& ImageSize = pixels.size() * BMP_COLOR_CHANNELS;
	
    PixelMap() = delete;
    PixelMap(const Resolution& res) : res(res), pixels(ScreenCapture::CalculateBMPFileSize(res) / BMP_COLOR_CHANNELS) {}

    const Pixel* const GetPixel(const Coordinate& coord) const {
        if (coord.first < 0 || coord.second < 0) { return nullptr; }
        else if (coord.first >= res.width || coord.second >= res.height) { return nullptr; }

        return &pixels[coord.second * res.width + coord.first];
    }

    const size_t GetPixelIndex(const Coordinate& coord) const {
        return coord.second * res.width + coord.first;
    }

	const Coordinate GetCoordinate(const size_t index) const {
        return { index % res.width , index / res.width };
	}

    const PixelList GetNeighbors(const Coordinate& coord) const {
    
        PixelList neighbors;
		
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				if (x == 0 && y == 0) { continue; }
                const Pixel* const pixel = GetPixel({ coord.first + x, coord.second + y });
				
				if (pixel != nullptr) { neighbors.push_back(*pixel); }
			}
		}

        return neighbors;
    
    }
	
    static const Coordinate GetCoordinate(const Resolution& res, const size_t index) {
        return { index % res.width , index / res.width };
    }
	
    static inline const size_t ConvertPixelIdxToByteIdx(const size_t pixelIdx, const size_t channels = BMP_COLOR_CHANNELS) {
		return pixelIdx * channels;
    }

    static inline const size_t ConvertByteIdxToPixelIdx(const size_t byteIdx, const size_t channels = BMP_COLOR_CHANNELS) {
		return byteIdx / channels;
    }
};

using ScaleRatio = std::pair<double, double>;

const size_t distance(const Coordinate& a, const Coordinate& b) {
	return std::sqrt(std::pow(a.first - b.first, 2) + std::pow(a.second - b.second, 2));
}

class Upscaler {

public:

    static inline ScaleMethod scaleMethod = ScaleMethod::NearestNeighbor;

    static inline const bool Upscale(char* sourceImage, char*& upscaled,
        const Resolution& sourceResolution, const Resolution& destResolution) {

        // If the resolutions are the same, don't scale
        if (sourceResolution == destResolution) [[unlikely]] {
            std::memcpy(upscaled, sourceImage, ScreenCapture::CalculateBMPFileSize(sourceResolution));
            return true;
        }
		
		// If source resolution is larger, don't scale (for now)
        else if (sourceResolution > destResolution) [[unlikely]] { return false; }

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

public:

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

    const static inline void ConvertFromPixelMap(const PixelMap& map, char*& image, 
        const bool isWindows = false) {
		
        if (!isWindows) { return; }

        for (size_t pixelIdx = 0; pixelIdx < map.pixels.size(); pixelIdx++) {
			const Pixel& pixel = map.pixels[pixelIdx];
			
			image[pixelIdx * BMP_COLOR_CHANNELS + 0] = pixel.rgba[0];
            image[pixelIdx * BMP_COLOR_CHANNELS + 1] = pixel.rgba[1];
            image[pixelIdx * BMP_COLOR_CHANNELS + 2] = pixel.rgba[2];
            image[pixelIdx * BMP_COLOR_CHANNELS + 3] = pixel.rgba[3];
        }
    }

    static inline char* const ConvertFromPixelMap(const PixelMap& map, const bool isWindows = false) {

		char* image = new char[map.pixels.size() * BMP_COLOR_CHANNELS];

        ConvertFromPixelMap(map, image, isWindows);

        return image;
    }

	static inline const double ScaleRatioX(const Resolution& source, const Resolution& dest) {
		return (double)dest.width / (double)source.width;
	}
	
    static inline const double ScaleRatioY(const Resolution& source, const Resolution& dest) {
        return (double)dest.height / (double)source.height;
    }

	static inline const ScaleRatio GetScaleRatio(const Resolution& source, const Resolution& dest) {
		return std::make_pair(ScaleRatioX(source, dest), ScaleRatioY(source, dest));
	}
	
    const static inline bool NearestNeighbor(char* source, char*& upscaled, const Resolution& src, const Resolution& dest) {

        PixelMap sourcePixels = BitmapToPixelMap(source, src, true);
        PixelMap destPixels(dest);

		const ScaleRatio& ratio = GetScaleRatio(src, dest);

        for (size_t destPixelIdx = 0; destPixelIdx < destPixels.pixels.size(); ++destPixelIdx) {
            const Coordinate& coord = destPixels.GetCoordinate(destPixelIdx);

            const Coordinate& mappedCoord = { coord.first / ratio.first, coord.second / ratio.second };
			const auto temp = *sourcePixels.GetPixel(mappedCoord);
            destPixels.pixels[destPixelIdx] = temp;
		
        }

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