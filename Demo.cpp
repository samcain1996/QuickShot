#include <iostream>
#include "Scale.h"

int main(int argc, char** argv) {

    if (argc != 1 && argc != 3) {
        std::cerr << "Too many command line arguments, expects 0 or 2\n";
        return 1;
    }

    int width = argc == 3 ? std::atoi(argv[1]) : ScreenCapture::DefaultResolution.width;
    int height = argc == 3 ? std::atoi(argv[2]) : ScreenCapture::DefaultResolution.height;

    // Source and dest resolutions
    Resolution captureResolution = RES_1080;

    Resolution higherResolution = RES_4K;
    Resolution lowerResolution = RES_480;

    Scaler::scaleMethod = ScaleMethod::NearestNeighbor;  // Currently the only one implemented

    // Initialize with resolution of 1920x1080
    ScreenCapture screen(captureResolution);

    // Capture the pixel data of the screen
    screen.CaptureScreen();
    
    // Save unscaled ScreenCapture to disk
    screen.SaveToFile("unscaled.bmp");
    
    screen.ReSize(captureResolution, higherResolution);
    screen.CaptureScreen();
    // Save unscaled ScreenCapture to disk
    screen.SaveToFile("upscaled.bmp");

    screen.ReSize(captureResolution, lowerResolution);
    screen.CaptureScreen();
    screen.SaveToFile("downscaled.bmp");

    return 0;
}