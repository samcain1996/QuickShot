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

	unsigned int width = 1920;
	unsigned int height = 1080;

    ScreenCapture screen(width, height);  // If no resolution is specified, ScreenCapture::DefaultResolution is used
	
	auto image = screen.CaptureScreen();

	screen.SaveToFile("original");

	Scaler::scaleMethod = ScaleMethod::Bilinear;
	image = Scaler::Scale(image, RES_1080, RES_4K);

	auto header = ConstructBMPHeader(RES_4K, 32);
	std::ofstream file("upscaled.bmp", std::ios::binary);
	file.write(header.data(), header.size());
	file.write(image.data(), image.size());

    return 0;
}
