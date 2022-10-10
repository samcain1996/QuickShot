#pragma once

#include "Scale.h"

class ScreenCapture {

private:

    const Resolution NATIVE_RESOLUTION;

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

    

public:

    static Resolution DefaultResolution;
    Resolution GetNativeResolution(const bool Reinit = false);

    void InitializeDisplay();

public:

    /* ---------- Constructors and Destructor ---------- */

    ScreenCapture(const ScreenCapture&);
    ScreenCapture(ScreenCapture&&) = delete;

	ScreenCapture(const Resolution& res = DefaultResolution, const std::optional<ScreenArea>& areaToCapture = std::nullopt);

    ScreenCapture(const Ushort width, const Ushort height);

    ScreenCapture& operator=(const ScreenCapture&) = delete;
    ScreenCapture& operator=(ScreenCapture&&);// = default;

    ~ScreenCapture();

    /* ------------------------------------------------- */

    void Resize(const Resolution& res = DefaultResolution);
    const PixelData& CaptureScreen();

    const PixelData WholeDeal() const;
    const Resolution& GetResolution() const;

    void SaveToFile(std::string filename = "screenshot.bmp") const;
};


