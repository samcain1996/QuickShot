#pragma once

#include <array>
#include <vector>
#include <cstring>
#include <fstream>

#if defined(_WIN32)

#include <Windows.h>
#include <ShellScalingApi.h>

#elif defined(__APPLE__)

#include "Scale.h"
#include <ApplicationServices/ApplicationServices.h>

#elif defined(__linux__)

#include "Scale.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

using Ushort = std::uint16_t;
using Uint32 = std::uint32_t;

constexpr void EncodeAsByte(char encodedNumber[4], const Uint32 numberToEncode) {

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
    Ushort width;
    Ushort height;

    bool operator==(const Resolution& other) const {
        return width == other.width && height == other.height;
    }
};

// Low definition
constexpr const Resolution RES_144 = { 256, 144 };
constexpr const Resolution RES_DEBUG = { 256, 144 };

// Standard definition
constexpr const Resolution RES_480 = { 640, 480 };

// High definition
constexpr const Resolution RES_720 = { 1280, 720 };
constexpr const Resolution RES_1080 = { 1920, 1080 };

constexpr const Resolution RES_1440 = { 2560, 1440 };
constexpr const Resolution RES_2K = { 2560, 1440 };

constexpr const Resolution RES_4K = { 3840, 2160 };

/*--------------------------------------------------*/

struct ScreenArea {
    size_t left = 0;
    size_t right = 0;
    size_t top = 0;
	size_t bottom = 0;
};

class ScreenCapture {

public:

    static const BmpFileHeader ConstructBMPHeader(Resolution resolution = DefaultResolution,
        const Ushort bitsPerPixel = 32);  // Initializes values for bitmap header

    constexpr static const Uint32 CalculateBMPFileSize(const Resolution& resolution, const Ushort bitsPerPixel = 32);

private:

    Resolution _resolution = DefaultResolution;
    ScreenArea _captureArea;
	
	// Header needed to create a valid bitmap file
    BmpFileHeader _header {};

    // Buffer holding screen capture 
    PixelData _pixelData {};

    Uint32 _captureSize = 0;
    Uint32 _bitsPerPixel = 32;

#if defined(_WIN32)

    HDC _srcHDC; // Device context of source
    HDC _memHDC; // Device context of destination

    // Windows bitmap data
    HBITMAP _hScreen;
    BITMAP _screenBMP;
    HANDLE _hDIB;

#endif

#if defined(__APPLE__)

    CGColorSpace* _colorspace = nullptr;
    CGContext* _context = nullptr;
    CGImage* _image = nullptr;

#elif defined (__linux__) 

    Display* _display = nullptr;
    Window _root {};
    XWindowAttributes _attributes = { 0 };
    XImage* _image = nullptr;

#endif

    constexpr static const BmpFileHeader BaseHeader();

public:

    static Resolution DefaultResolution;

public:

    /* ---------- Constructors and Destructor ---------- */

    ScreenCapture(const ScreenCapture&);
    ScreenCapture(ScreenCapture&&) = delete;

    ScreenCapture(const Resolution& res = DefaultResolution);

    ScreenCapture(const Ushort width, const Ushort height);

    ScreenCapture& operator=(const ScreenCapture&) = delete;
    ScreenCapture& operator=(ScreenCapture&&) = delete;

    ~ScreenCapture();

    /* ------------------------------------------------- */

    void Resize(const Resolution& res = DefaultResolution);
    const PixelData& CaptureScreen();

    const PixelData WholeDeal() const;
    constexpr const size_t TotalSize() const;
    const Resolution& GetResolution() const;

    void SaveToFile(std::string filename = "screenshot.bmp") const;
};


