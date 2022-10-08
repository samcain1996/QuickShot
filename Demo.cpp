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

    // Initialize with resolution of 1920x1080
    ScreenCapture screen(captureResolution, higherResolution);

    // Get the header of bmp with scaled resolution
    BmpFileHeader header = screen.ConstructBMPHeader(screen.DestResolution());

    // Capture the pixel data of the screen
    ImageData img = screen.CaptureScreen();
    
    // Save unscaled ScreenCapture to disk
    screen.SaveToFile("unscaled.bmp");
	
    // Scale to target resolution
    char* upscaled;

    Scaler::scaleMethod = ScaleMethod::NearestNeighbor;  // Currently the only one implemented
    Scaler::Upscale(img.data(), upscaled, captureResolution, higherResolution);

    // Manual save (scaled image)
    std::ofstream imageFile("scaled.bmp", std::ios::out);
    imageFile.write(header.data(), header.size());
    imageFile.write(upscaled, ScreenCapture::CalculateBMPFileSize(higherResolution));

    delete[] upscaled;  // Free memory

    return 0;
}