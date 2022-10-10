#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include "Capture.h"

const std::string percentageOfScreenCaptured(const Resolution& nativeRes, const ScreenArea& capturedArea) {
	
	const double totalPixels = nativeRes.width * nativeRes.height;
	const double pixelsCaptured = (capturedArea.right - capturedArea.left) * (capturedArea.bottom - capturedArea.top);

    const auto percentage = ( pixelsCaptured / totalPixels ) * 100 ;

	// Truncate to 2 decimal places
    std::stringstream formatted;
    formatted << std::fixed << std::setprecision(2) << percentage;

    return formatted.str();
}

const std::string nameFile(const Resolution& resolution,const std::string& additional = "") {
	
    return std::to_string(resolution.width) + "x" + std::to_string(resolution.height) + additional + ".bmp";
	
}

int main(int argc, char** argv) {

    if (argc != 1 && argc != 3) {
        std::cerr << "Too many command line arguments, expects 0 or 2\n";
        return 1;
    }

	// Set width and height from command line if available, 
    // otherwise use default resolution ( Configurable in Capture.h )
    Ushort width = argc == 3 ? std::atoi(argv[1]) : ScreenCapture::DefaultResolution.width;
    Ushort height = argc == 3 ? std::atoi(argv[2]) : ScreenCapture::DefaultResolution.height;

    Resolution resolution = { width, height };

    ScreenCapture screen(resolution);  // If not resolution is specified, ScreenCapture::DefaultResolution is used
	
    const Resolution nativeResolution = screen.GetNativeResolution();
	Resolution lowResolution = RES_480;  // List of predefined resolutions in Capture.h

    screen.CaptureScreen();  // Take a screenshot
	
	// Save screenshot
	std::string filename = nameFile(screen.GetResolution());  // Current image resolution
    screen.SaveToFile(filename);
    std::cout << "Saved " << filename << " to disk\n";

	
	// Again but with a lower resolution
	
    screen.Resize(lowResolution);

    screen.CaptureScreen();
	
    filename = nameFile(screen.GetResolution());
    screen.SaveToFile(filename);
    std::cout << "Saved " << filename << " to disk\n";

	
	// Capture only portion of the entire screen
    
    ScreenArea areaToCapture = { 0, nativeResolution.width / 2, 0, nativeResolution.height / 2 };

    screen = ScreenCapture(nativeResolution, areaToCapture);
    screen.CaptureScreen();

	filename = nameFile(screen.GetResolution(), "res_" + 
        percentageOfScreenCaptured(nativeResolution, areaToCapture) + "%_of_entire_screen");
	
	screen.SaveToFile(filename);
	std::cout << "Saved " << filename << " to disk\n";

    return 0;
}