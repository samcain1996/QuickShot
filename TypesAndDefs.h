#include <span>
#include <cmath>
#include <array>
#include <vector>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <functional>

#if defined(_WIN32)

#define NOMINMAX

#include <Windows.h>
#include <ShellScalingApi.h>

#elif defined(__APPLE__)

#include <ApplicationServices/ApplicationServices.h>

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

using Ushort = std::uint16_t;
using Uint32 = std::uint32_t;

using MyByte = char;
constexpr const MyByte MAX_MYBYTE_VAL = static_cast<MyByte>(255);

using ByteSpan = std::span<MyByte, 4>;

constexpr const int ONE_BYTE = 8;
constexpr const int HALF_BYTE = 4;

// Convert base 10 number to base 256
constexpr void EncodeAsByte(ByteSpan encodedNumber, const Uint32 numberToEncode) {

    encodedNumber[3] = (numberToEncode >> 24) & 0xFF;
    encodedNumber[2] = (numberToEncode >> 16) & 0xFF;
    encodedNumber[1] = (numberToEncode >> 8) & 0xFF;
    encodedNumber[0] = (numberToEncode) & 0xFF;

}

// BMP Constants
constexpr const Ushort BMP_FILE_HEADER_SIZE = 14;
constexpr const Ushort BMP_INFO_HEADER_SIZE = 40;
constexpr const Ushort BMP_HEADER_SIZE = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE;

// Pixel Constants
constexpr const Ushort NUM_COLOR_CHANNELS = 4;
constexpr const Ushort BITS_PER_CHANNEL = 8;

// Types
using BmpFileHeader = std::array<MyByte, BMP_HEADER_SIZE>;
using PixelData = std::vector<MyByte>;

/*------------------RESOLUTIONS--------------------*/

struct Resolution {
    int width = 0;
    int height = 0;

    constexpr Resolution(const int x, const int y) : width(x), height(y) {}
    constexpr Resolution(const double x, const double y) : width(x), height(y) {}

    double AspectRatio() const { return width / (double)height; }

    Resolution operator*(const int factor) const {

        const size_t targetArea = width * height * factor;

        const int newHeight = sqrt(width * height * factor / AspectRatio());
        const int newWidth = AspectRatio() * newHeight;

        return { newWidth, newHeight };
    }

    Resolution& operator*=(const int factor) {
        const Resolution newResolution = *this * factor;

        return *this;
    }

    Resolution operator/(const int divisor) const {

        const size_t targetArea = width * height / divisor;

        const int newHeight = sqrt(width * height / (divisor * AspectRatio()));
        const int newWidth = AspectRatio() * newHeight;

        return { newWidth, newHeight };
    }

    Resolution& operator/=(const int divisor) {

        Resolution newResolution = *this / divisor;

        return *this;
    }

    bool operator==(const Resolution& other) const {
        return width == other.width && height == other.height;
    }

    bool operator<(const Resolution& other) const {
        return width < other.width || height < other.height;
    }
    bool operator>(const Resolution& other) const {
        return !(*this == other || *this < other);
    }
    
};

// Low definition
constexpr static const Resolution RES_144 = { 256, 144 };
constexpr static const Resolution RES_DEBUG = { 256, 144 };

// Standard definition
constexpr static const Resolution RES_480 = { 640, 480 };

// High definition
constexpr static const Resolution RES_720 = { 1280, 720 };
constexpr static const Resolution RES_1080 = { 1920, 1080 };
constexpr static const Resolution RES_1440 = { 2560, 1440 };
constexpr static const Resolution RES_2K = { 2560, 1440 };
constexpr static const Resolution RES_4K = { 3840, 2160 };

/*--------------------------------------------------*/

struct ScreenArea {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    ScreenArea() = default;
    ScreenArea(const int left, const int right, const int top, const int bottom) :
        left(left), right(right), top(top), bottom(bottom) {}
	ScreenArea(const Resolution& res) : right(res.width), bottom(res.height) {}
    ScreenArea(const Resolution& res, const int xOffset, const int yOffset) :
        left(xOffset), right(xOffset + res.width), top(yOffset), bottom(yOffset + res.height) {}

    // Total area of screen being captured
    int Area() const { return ( right - left ) * ( bottom - top ); }

    // One ScreenArea is smaller than another if its Area is less
    bool operator<(const ScreenArea& other) const {
        return Area() < other.Area();
    }

    operator Resolution() { return { (right - left), (bottom - top) }; }
};

static const inline Uint32 CalculateBMPFileSize(const Resolution& resolution, const Ushort bitsPerPixel = 32) {
    return ((resolution.width * bitsPerPixel + 31) / 32) * NUM_COLOR_CHANNELS * resolution.height;
};


static constexpr const BmpFileHeader BaseHeader() {

    BmpFileHeader baseHeader {};  // All values init to '\0'?

    // Identifies file as bmp
    baseHeader[0] = 0x42;
    baseHeader[1] = 0x4D;

    baseHeader[10] = 0x36;  // Offset to start of pixel data

    // Start of info related to pixel data
    baseHeader[BMP_FILE_HEADER_SIZE] = 0x28;

    baseHeader[BMP_FILE_HEADER_SIZE+12] = 1;

    return baseHeader;
}

// Create a simple BITMAPFILEHEADER and BITMAPINFOHEADER as 1, 54-byte array
static const inline BmpFileHeader ConstructBMPHeader(const Resolution& resolution,
        const Ushort bitsPerPixel = 32) {

    using HeaderIter = BmpFileHeader::iterator;
    const int filesizeOffset = 2;
    const int filesize = CalculateBMPFileSize(resolution, bitsPerPixel);

    int modifier = 1;

    BmpFileHeader header = BaseHeader();

    HeaderIter filesizeIter = header.begin() + filesizeOffset;
    HeaderIter widthIter = header.begin() + BMP_FILE_HEADER_SIZE + sizeof(filesize);
    HeaderIter heightIter = header.begin() + BMP_FILE_HEADER_SIZE + sizeof(filesize) + sizeof(resolution.width);
	
    // Encode file size
    EncodeAsByte(ByteSpan(filesizeIter, HALF_BYTE), filesize + BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE);

    // Encode pixels wide
    EncodeAsByte(ByteSpan(widthIter, HALF_BYTE), resolution.width);

#if !defined(_WIN32)  // Window bitmaps are stored upside down

    modifier = -modifier;

#endif

    // Encode pixels high
    EncodeAsByte(ByteSpan(heightIter, HALF_BYTE), modifier * resolution.height);

#if !defined(_WIN32)  // Window bitmaps are stored upside down

    std::for_each( (header.begin() + BMP_FILE_HEADER_SIZE + 8), (header.begin() + BMP_FILE_HEADER_SIZE + 12), 
        [](MyByte& b) { if ( b == '\0' ) { b = (MyByte)MAX_MYBYTE_VAL; } });

#endif

    header[BMP_FILE_HEADER_SIZE + 14] = bitsPerPixel;
	
    return header;
	
}