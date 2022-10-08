#include "Scale.h"

constexpr const Uint32 ScreenCapture::CalculateBMPFileSize(const Resolution& resolution, const Ushort bitsPerPixel) {
    return ((resolution.width * bitsPerPixel + 31) / 32) * BMP_COLOR_CHANNELS * resolution.height;
}

ScreenCapture::ScreenCapture(const Ushort srcWidth, const Ushort srcHeight, const Ushort destWidth, const Ushort destHeight) {

    _captureResolution.width = srcWidth;
    _captureResolution.height = srcHeight;
	
    _destResolution.width = destWidth;
    _destResolution.height = destHeight;

#if defined(__linux__)

    _display = XOpenDisplay(nullptr);
    _root = DefaultRootWindow(_display);

    XGetWindowAttributes(_display, _root, &_attributes);
    
#endif

#if defined(_WIN32)

    _srcHDC = GetDC(GetDesktopWindow());  // Get the device context of the monitor [1]
    _memHDC = CreateCompatibleDC(_srcHDC);    // Creates a new device context from previous context

    _hDIB = NULL;

#endif

#if defined(__APPLE__)
    
    _colorspace = CGColorSpaceCreateDeviceRGB();

#endif

    ReInitialize(_captureResolution, _destResolution);

}

ScreenCapture::ScreenCapture(const Resolution& CaptureResolution, const Resolution& destResolution) : 
    ScreenCapture(CaptureResolution.width, CaptureResolution.height, destResolution.width, destResolution.height) {}

ScreenCapture::ScreenCapture(const Ushort width, const Ushort height) : ScreenCapture(width, height, width, height) {}

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


ScreenCapture::ScreenCapture(const ScreenCapture& other) : ScreenCapture(other.CaptureResolution(), other.DestResolution()) {}

const Resolution& ScreenCapture::CaptureResolution() const { return _captureResolution; }

const Resolution& ScreenCapture::DestResolution() const { return _destResolution; }

constexpr const BmpFileHeader ScreenCapture::BaseHeader() {

    BmpFileHeader baseHeader {};  // TODO: See how to init all vals to 0

    // Identifies file as bmp
    baseHeader[0] = 0x42;
    baseHeader[1] = 0x4D;

    baseHeader[10] = 0x36;  // Start of pixel data

    // Start of info related to pixel data
    baseHeader[BMP_FILE_HEADER_SIZE] = 0x28;

    baseHeader[BMP_FILE_HEADER_SIZE+12] = 1;

    return baseHeader;
}

const BmpFileHeader ScreenCapture::ConstructBMPHeader(Resolution resolution,
        const Ushort bitsPerPixel) {

    BmpFileHeader header = BaseHeader();

    // Encode file size
    EncodeAsByte(&header[2], resolution.width * resolution.height * 
        BMP_COLOR_CHANNELS + BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE);

    // Encode pixels wide
    EncodeAsByte(&header[4 + BMP_FILE_HEADER_SIZE], resolution.width);

#if !defined(_WIN32)  // Window bitmaps are stored upside down

    resolution.height = -resolution.height;

#endif

    // Encode pixels high
    EncodeAsByte(&header[8 + BMP_FILE_HEADER_SIZE], resolution.height);

#if !defined(_WIN32)  // Window bitmaps are stored upside down

    std::for_each( (header.begin() + BMP_FILE_HEADER_SIZE + 8), (header.begin() + BMP_FILE_HEADER_SIZE + 12), 
        [](char& b) { if ( b == '\0' ) { b = (char)255; } });

#endif

    header[BMP_FILE_HEADER_SIZE + 14] = bitsPerPixel;
	
    return header;
	
}

constexpr const size_t ScreenCapture::TotalSize() const {
    return _captureSize + BMP_HEADER_SIZE;
}

void ScreenCapture::ReSize(const Resolution& resolution) { ReInitialize(resolution); }
void ScreenCapture::ReSize(const Resolution& sourceRes, const Resolution& destRes) { ReInitialize(sourceRes, destRes); }

void ScreenCapture::ReInitialize(const Resolution& captureResolution, const Resolution& destResolution) {

    _captureResolution = captureResolution;
	
    _destResolution = destResolution;

    _captureSize = CalculateBMPFileSize(_captureResolution, _bitsPerPixel);
	
    _header = ConstructBMPHeader(_destResolution, _bitsPerPixel);

    _pixelData = ImageData(_captureSize, '\0');

#if defined(_WIN32)

    // Recreate bitmap with new dimensions
    DeleteObject(_hScreen);
    _hScreen = CreateCompatibleBitmap(_srcHDC, _captureResolution.width, _captureResolution.height);

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

    _context = CGBitmapContextCreate(_pixelData.data(), _destResolution.width, _destResolution.height, 
        8, _destResolution.width * BMP_COLOR_CHANNELS, _colorspace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

#endif

}

void ScreenCapture::ReInitialize(const Resolution& resolution) { ReInitialize(resolution, resolution); }

const ImageData ScreenCapture::WholeDeal() const {

    ImageData wholeDeal(_header.begin(), _header.end());
    PixelData destPixelData;
	
    Scaler::Scale(_pixelData.data(), destPixelData, _captureResolution, _destResolution);
    std::copy(destPixelData, destPixelData + CalculateBMPFileSize(_destResolution), std::back_inserter(wholeDeal));

    return wholeDeal;

}

const ImageData& ScreenCapture::CaptureScreen() {

#if defined(_WIN32)

    BitBlt(_memHDC, 0, 0, _captureResolution.width, _captureResolution.height,
        _srcHDC, 0, 0, SRCCOPY);

    GetObject(_hScreen, sizeof BITMAP, &_screenBMP);

    // Should be legal because BITMAPINFO has no padding, all its data members are aligned.
    GetDIBits(_memHDC, _hScreen, 0,
        (UINT)_screenBMP.bmHeight,
        _pixelData.data(),
        (BITMAPINFO*)(&ScreenCapture::ConstructBMPHeader(_captureResolution)[BMP_FILE_HEADER_SIZE]), DIB_RGB_COLORS);

#elif defined(__APPLE__)

    _image = CGDisplayCreateImage(CGMainDisplayID());
    CGContextDrawImage(_context, CGRectMake(0, 0, _captureResolution.width, _captureResolution.height), _image);

#elif defined(__linux__)

    _image = XGetImage(_display, _root, 0, 0, _captureResolution.width, _captureResolution.height, AllPlanes, ZPixmap);
    _pixelData = ImageData(_image->data, _image->data + _captureSize);

#endif

    return _pixelData;
}

void ScreenCapture::SaveToFile(std::string filename) const {

    // Add file extension if not present
    if (filename.find(".bmp") == std::string::npos) {
        filename += ".bmp";
    }

	// Save image to disk
    const ImageData entireImage = WholeDeal();
    std::ofstream(filename, std::ios::binary).write(entireImage.data(), entireImage.size());

}

