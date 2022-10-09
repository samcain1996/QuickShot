#include "Capture.h"

Resolution ScreenCapture::DefaultResolution = RES_1080;

ScreenCapture::ScreenCapture(const Ushort width, const Ushort height) {

    _resolution.width = width;
    _resolution.height = height;

#if defined(__linux__)

    _display = XOpenDisplay(nullptr);
    _root = DefaultRootWindow(_display);

    XGetWindowAttributes(_display, _root, &_attributes);
    
#endif

#if defined(_WIN32)
	
    SetProcessDPIAware();  // Needed to ensure correct resolution

    _srcHDC = GetDC(GetDesktopWindow());      // Get the device context of the monitor [1]
    _memHDC = CreateCompatibleDC(_srcHDC);    // Creates a new device context from previous context

	SetStretchBltMode(_memHDC, HALFTONE);     // Set the stretching mode to halftone

    _hDIB = NULL;

	// Capture the entire screen by default
    _captureArea.right = GetSystemMetrics(SM_CXSCREEN);
    _captureArea.bottom = GetSystemMetrics(SM_CYSCREEN);

#endif

#if defined(__APPLE__)
    
    _colorspace = CGColorSpaceCreateDeviceRGB();
    
#endif

    Resize(_resolution);

}

ScreenCapture::ScreenCapture(const Resolution& res) : ScreenCapture(res.width, res.height) {}

ScreenCapture::~ScreenCapture() {

#if defined(_WIN32)

    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    DeleteObject(_hScreen);

    ReleaseDC(NULL, _srcHDC);
    DeleteObject(_memHDC);

#elif defined(__APPLE__)

    CGImageRelease(_image);
    CGContextRelease(_context);
    CGColorSpaceRelease(_colorspace);

#elif defined(__linux__)

    XDestroyImage(_image);
    XCloseDisplay(_display);

#endif

}


ScreenCapture::ScreenCapture(const ScreenCapture& other) : ScreenCapture(other.GetResolution()) {}

const Resolution& ScreenCapture::GetResolution() const { return _resolution; }



constexpr const size_t ScreenCapture::TotalSize() const {
    return _captureSize + BMP_HEADER_SIZE;
}

void ScreenCapture::Resize(const Resolution& resolution) {

    _resolution = resolution;

    _captureSize = CalculateBMPFileSize(_resolution, _bitsPerPixel);
    _header = ConstructBMPHeader(_resolution, _bitsPerPixel);

    _pixelData = PixelData(_captureSize, '\0');

#if defined(_WIN32)

    // Recreate bitmap with new dimensions
    DeleteObject(_hScreen);
    _hScreen = CreateCompatibleBitmap(_srcHDC, _resolution.width, _resolution.height);

    SelectObject(_memHDC, _hScreen);  // Select bitmap into DC [2]

    // Free _hDIB and re-lock
    GlobalUnlock(_hDIB);
    GlobalFree(_hDIB);

    _hDIB = GlobalAlloc(GHND, _captureSize);
    (char*)GlobalLock(_hDIB);

#elif defined(__APPLE__)

    // Ok to call on null objects
    CGImageRelease(_image);
    CGContextRelease(_context); 

    _context = CGBitmapContextCreate(_pixelData.data(), _resolution.width, _resolution.height, 
        8, _resolution.width * BMP_COLOR_CHANNELS, _colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

#endif

}

const PixelData ScreenCapture::WholeDeal() const {
	
    PixelData wholeDeal(_header.begin(), _header.end());
	std::copy(_pixelData.begin(), _pixelData.end(), std::back_inserter(wholeDeal));

    return wholeDeal;
}

const PixelData& ScreenCapture::CaptureScreen() {

#if defined(_WIN32)

    // Resize to target resolution
    StretchBlt(_memHDC, 0, 0, _resolution.width, _resolution.height,
        _srcHDC, 0, 0, _captureArea.right - _captureArea.left, 
        _captureArea.bottom - _captureArea.top, SRCCOPY);

    GetObject(_hScreen, sizeof BITMAP, &_screenBMP);

    // Store screen data in _pixelData
    // Should be legal because BITMAPINFO has no padding, all its data members are aligned.
    GetDIBits(_memHDC, _hScreen, 0,
        (UINT)_screenBMP.bmHeight,
        _pixelData.data(),
        (BITMAPINFO*)(&_header[BMP_FILE_HEADER_SIZE]), DIB_RGB_COLORS);

#elif defined(__APPLE__)

    _image = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(_context, CGRectMake(0, 0, _resolution.width, _resolution.height), _image);

#elif defined(__linux__)

    const Resolution captureRes = std::min(MAX_RESOLUTION(), _resolution);  // BUG: If MAX_RESOLUTION() is second, the program crashes lol

    _image = XGetImage(_display, _root, 0, 0, captureRes.width, captureRes.height, AllPlanes, ZPixmap);
    _pixelData = PixelData(_image->data, _image->data + _captureSize);

    // if (needsScaling) {
        
        
    //    Scaler::Scale(_pixelData, _resolution, captureRes);
    // }

#endif

    return _pixelData;
}

void ScreenCapture::SaveToFile(std::string filename) const {

    // Add file extension if not present
    if (filename.find(".bmp") == std::string::npos) {
        filename += ".bmp";
    }

	// Save image to disk
    const PixelData entireImage = WholeDeal();
    std::ofstream(filename, std::ios::binary).write(entireImage.data(), entireImage.size());

}

