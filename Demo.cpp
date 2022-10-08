#include <iostream>
#include "Scale.h"

int main(int argc, char** argv) {

    if (argc != 1 && argc != 3) {
        std::cerr << "Too many command line arguments, expects 0 or 2\n";
        return 1;
    }

    Ushort width = argc == 3 ? std::atoi(argv[1]) : ScreenCapture::DefaultResolution.width;
    Ushort height = argc == 3 ? std::atoi(argv[2]) : ScreenCapture::DefaultResolution.height;

    // Source and dest resolutions
    Resolution captureResolution = RES_720;
    Resolution scaledResolution{width, height};

    // Initialize with resolution of 1920x1080
    ScreenCapture screen(captureResolution, scaledResolution);

    // Get the header of bmp with current resolution
    BmpFileHeader header = screen.ConstructBMPHeader(screen.DestResolution());

    // Capture the pixel data of the screen
    ImageData img = screen.CaptureScreen();
    char* upscaled = new char[ScreenCapture::CalculateBMPFileSize(scaledResolution)];
    Scaler::Upscale(img.data(), upscaled, captureResolution, scaledResolution);

    std::ofstream imageFile("manual_image_save.bmp", std::ios::binary);
    imageFile.write(header.data(), header.size());
    imageFile.write(upscaled, ScreenCapture::CalculateBMPFileSize(scaledResolution));

    // Save ScreenCapture to disk
    screen.SaveToFile("TestScreenshot.bmp");

    return 0;
}