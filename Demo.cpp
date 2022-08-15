//#include <iostream>
//#include "Capture.h"
//
//int main(int argc, char** argv) {
//
//    if (argc != 1 && argc != 3) {
//        std::cerr << "Too many command line arguments, expects 0 or 2\n";
//        return 1;
//    }
//
//    int width = argc == 3 ? std::atoi(argv[1]) : ScreenCapture::DefaultResolution.width;
//    int height = argc == 3 ? std::atoi(argv[2]) : ScreenCapture::DefaultResolution.height;
//
//    // Initialize with resolution of 1920x1080
//    ScreenCapture screen(width, height);
//
//    // Capture the screen content
//    ImageData img = screen.CaptureScreen();
//
//    std::ofstream file("test.bmp", std::ios::binary);
//    file.write(screen.ConstructBMPHeader(screen.ImageResolution()).data(), 
//        BMP_HEADER_SIZE);
//    file.write(img.data(), img.size());
//
//    // Save ScreenCapture to disk
//    //screen.SaveToFile("TestScreenshot.bmp");
//
//    return 0;
//}