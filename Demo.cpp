#include <iostream>
#include <string>
#include "Capture.h"

int main(int argc, char** argv) {

    if (argc != 1 && argc != 3) {
        std::cerr << "Too many command line arguments, expects 0 or 2\n";
        return 1;
    }

    int width = argc == 3 ? std::atoi(argv[1]) : ScreenCapture::DefaultResolution.width;
    int height = argc == 3 ? std::atoi(argv[2]) : ScreenCapture::DefaultResolution.height;

    // Source and dest resolutions
    Resolution resolution = { width, height };

    Resolution highResolution = RES_4K;
    Resolution lowResolution = RES_480;

    // Initialize with resolution of 1920x1080
    ScreenCapture screen(resolution);
	
    // Capture the pixel data of the screen
    screen.CaptureScreen();

    std::string filename = std::to_string(resolution.width) + "x" + std::to_string(resolution.height) + ".bmp";
    
    // Save unscaled ScreenCapture to disk
    screen.SaveToFile(filename);
    std::cout << "Saved " << filename << " to disk\n";
    
    screen.Resize(highResolution);
    filename = std::to_string(highResolution.width) + "x" + std::to_string(highResolution.height) + ".bmp";
	
        screen.CaptureScreen();
    screen.SaveToFile(filename);
    std::cout << "Saved " << filename << " to disk\n";

    screen.Resize(lowResolution);
    filename = std::to_string(lowResolution.width) + "x" + std::to_string(lowResolution.height) + ".bmp";
	
       screen.CaptureScreen();
    screen.SaveToFile(filename);
    std::cout << "Saved " << filename << " to disk\n";

    return 0;
}