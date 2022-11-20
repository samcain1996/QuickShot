#include <array>
#include <vector>
#include <cstring>
#include <fstream>
#include <span>

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
using ByteSpan = std::span<char, 4>;

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
constexpr const Ushort BMP_COLOR_CHANNELS = 4;

// Types
using BmpFileHeader = std::array<char, BMP_HEADER_SIZE>;

using PixelData = std::vector<char>;

/*------------------RESOLUTIONS--------------------*/

struct Resolution {
    int width = 0;
    int height = 0;

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
	ScreenArea(const Resolution& res) : right(res.width), bottom(res.height) {}
    operator Resolution() { return { (right - left), (bottom - top) }; }
};

static const inline Uint32 CalculateBMPFileSize(const Resolution& resolution, const Ushort bitsPerPixel = 32) {
    return ((resolution.width * bitsPerPixel + 31) / 32) * BMP_COLOR_CHANNELS * resolution.height;
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


static const inline BmpFileHeader ConstructBMPHeader(Resolution resolution,
        const Ushort bitsPerPixel) {

    BmpFileHeader header = BaseHeader();

	BmpFileHeader::iterator filesizeIter = header.begin() + 2;
    BmpFileHeader::iterator widthIter = header.begin() + BMP_FILE_HEADER_SIZE + 4;
    BmpFileHeader::iterator heightIter = header.begin() + BMP_FILE_HEADER_SIZE + 8;
	
    // Encode file size
    EncodeAsByte(ByteSpan(filesizeIter, 4), resolution.width * resolution.height *
        BMP_COLOR_CHANNELS + BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE);

    // Encode pixels wide
    EncodeAsByte(ByteSpan(widthIter, 4), resolution.width);

#if !defined(_WIN32)  // Window bitmaps are stored upside down

    resolution.height = -resolution.height;

#endif

    // Encode pixels high
    EncodeAsByte(ByteSpan(heightIter, 4), resolution.height);

#if !defined(_WIN32)  // Window bitmaps are stored upside down

    std::for_each( (header.begin() + BMP_FILE_HEADER_SIZE + 8), (header.begin() + BMP_FILE_HEADER_SIZE + 12), 
        [](char& b) { if ( b == '\0' ) { b = (char)255; } });

#endif

    header[BMP_FILE_HEADER_SIZE + 14] = bitsPerPixel;
	
    return header;
	
}