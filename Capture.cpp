#include "Capture.h"
#include <functional>

Resolution ScreenCapture::DefaultResolution = ScreenCapture::NativeResolution();

Resolution ScreenCapture::NativeResolution(const bool Reinit) {
    
    static auto retrieveRes = []() {
		
#if defined(_WIN32)
		
        SetProcessDPIAware();
        return Resolution{ (Ushort)GetSystemMetrics(SM_CXSCREEN), (Ushort)GetSystemMetrics(SM_CYSCREEN) };
		
#elif defined(__linux__)

        XGetWindowAttributes(_display, _root, &_attributes);
        return Resolution{ (Ushort)_attributes.width, (Ushort)_attributes.height };
		
#elif defined(__APPLE__)

        const auto mainDisplayId = CGMainDisplayID();
        return Resolution{ (Ushort)CGDisplayPixelsWide(mainDisplayId), (Ushort)CGDisplayPixelsHigh(mainDisplayId) };
		
#endif

    };
	
    static Resolution NATIVE_RESOLUTION = retrieveRes();

    if (Reinit) { retrieveRes(); }

    return NATIVE_RESOLUTION;
	
}

ScreenCapture::ScreenCapture(const Ushort width, const Ushort height) {

    _resolution.width = width;
    _resolution.height = height;

    // Capture the entire screen by default	
    _captureArea.right = NativeResolution().width;
    _captureArea.bottom = NativeResolution().height;

#if defined(_WIN32)

    _srcHDC = GetDC(GetDesktopWindow());      // Get the device context of the monitor [1]
    _memHDC = CreateCompatibleDC(_srcHDC);    // Creates a new device context from previous context

    SetStretchBltMode(_memHDC, HALFTONE);     // Set the stretching mode to halftone

    _hDIB = NULL;

#elif defined(__APPLE__)

    _colorspace = CGColorSpaceCreateDeviceRGB();

#endif

    Resize(_resolution);

}


ScreenCapture::ScreenCapture(const Resolution& res, const std::optional<ScreenArea>& areaToCapture) : ScreenCapture(res.width, res.height) {

    if (areaToCapture.has_value()) {
        const auto& area = areaToCapture.value();
        Crop(area);
    }

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

#endif

}


ScreenCapture::ScreenCapture(const ScreenCapture& other) : ScreenCapture(other.GetResolution()) {}

const Resolution& ScreenCapture::GetResolution() const { return _resolution; }

void ScreenCapture::Resize(const Resolution& resolution) {

    _resolution = std::min<Resolution>(NativeResolution(), resolution);

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

void ScreenCapture::Crop(const ScreenArea& area) { _captureArea = area; }

const PixelData ScreenCapture::WholeDeal() const {
	
    PixelData wholeDeal(_header.begin(), _header.end());
	std::copy(_pixelData.begin(), _pixelData.end(), std::back_inserter(wholeDeal));

    return wholeDeal;
}

const PixelData& ScreenCapture::CaptureScreen() {

    const Resolution& captureAreaRes = _captureArea;

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

	_image = CGDisplayCreateImageForRect(CGMainDisplayID(), CGRectMake(0, 0, _resolution.width, _resolution.height));
    CGContextDrawImage(_context, CGRectMake(_captureArea.left, - (int)(_captureArea.bottom - _resolution.height),
        captureAreaRes.width, captureAreaRes.height), _image);

#elif defined(__linux__)

    _image = XGetImage(_display, _root, NativeResolution().width - captureAreaRes.width, 
        NativeResolution().height - captureAreaRes.height, captureAreaRes.width, captureAreaRes.height, 
        AllPlanes, ZPixmap);   

    _pixelData = PixelData(_image->data, _image->data + CalculateBMPFileSize(captureAreaRes));
    _pixelData = Scaler::Scale(_pixelData, captureAreaRes, _resolution);
        
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