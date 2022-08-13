#include <iostream>
#include "Capture.h"

void CaptureScreenVector(ScreenCapture& screen) {

    // Capture the pixel data only
    ImageData imageData = screen.GetImageData();

    // Capture the piel data with bmp header
    ImageData fullImage = screen.WholeDeal();

}

void CaptureScreenArray(ScreenCapture& screen) {

    // Capture only the pixel data only
    char* imageData = nullptr;
    size_t imageDataSize = screen.GetImageData(imageData);

    // Capture the pixel data with bmp header
    char* fullImage = nullptr;
    size_t fullImageSize = screen.WholeDeal(fullImage);

    // Must manually free memory using this method
    delete[] imageData;
    delete[] fullImage;
}

int main(int argc, char** argv) {

    if (argc != 1 && argc != 3) {
        std::cerr << "Too many command line arguments, expects 0 or 2\n";
        return 1;
    }

    int width = argc == 3 ? std::atoi(argv[1]) : ScreenCapture::DefaultResolution.width;
    int height = argc == 3 ? std::atoi(argv[2]) : ScreenCapture::DefaultResolution.height;

    // Initialize with resolution of 1920x1080
    ScreenCapture screen(width, height);

    // Capture the screen content
    screen.CaptureScreen();

    // Save ScreenCapture to disk
    screen.SaveToFile("TestScreenshot.bmp");

    return 0;
}