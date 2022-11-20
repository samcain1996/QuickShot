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

	Resolution sourceRes = ScreenCapture::NativeResolution();
	Resolution targetRes = RES_4K;

    ScreenCapture screen(sourceRes);  // If no resolution is specified, ScreenCapture::DefaultResolution is used
	
	auto image = screen.CaptureScreen();
	screen.SaveToFile("Original");

	auto header = ConstructBMPHeader(targetRes, 32);

	Scaler::scaleMethod = ScaleMethod::NearestNeighbor;
	auto scaled = Scaler::Scale(image, sourceRes, targetRes);

	std::ofstream file("NearestNeighbor.bmp", std::ios::binary);
	file.write(header.data(), header.size());
	file.write(scaled.data(), scaled.size());
	file.close();

	Scaler::scaleMethod = ScaleMethod::Bilinear;
	scaled = Scaler::Scale(image, sourceRes, targetRes);

	file = std::ofstream("Bilinear.bmp", std::ios::binary);
	file.write(header.data(), header.size());
	file.write(scaled.data(), scaled.size());

    return 0;
}
