#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
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

const std::string NameImage(const Resolution& resolution, const bool isOriginal = false) {

	std::string imageName;

	switch (Scaler::method) {
	case Scaler::ScaleMethod::NearestNeighbor:
		imageName = "NearestNeighbor";
		break;
	case Scaler::ScaleMethod::Bilinear:
		imageName = "Bilinear";
		break;
	case Scaler::ScaleMethod::Bicubic:
		imageName = "Bicubic";
		break;
	default:
		 imageName = "Unknown";
	}

	if (isOriginal) { imageName = "Original"; }

	imageName += std::to_string(resolution.width) + "x" + std::to_string(resolution.height);
	return imageName;
}

int main(int argc, char** argv) {

	Resolution sourceRes = RES_720;
	Resolution targetRes = RES_1080;

	int xOffset = 0, yOffset = 0;
	ScreenArea captureArea(ScreenCapture::NativeResolution(), xOffset, yOffset);
	ScreenCapture screen(sourceRes);

	PixelData image = screen.CaptureScreen();
	screen.SaveToFile(NameImage(screen.GetResolution(), true));

	//Scaler::method = Scaler::ScaleMethod::NearestNeighbor;
	//PixelData scaled = Scaler::Scale(image, sourceRes, targetRes);
	//screen.SaveToFile(scaled, targetRes, NameImage(targetRes));

	//Scaler::method = Scaler::ScaleMethod::Bilinear;
	//scaled = Scaler::Scale(image, sourceRes, targetRes);
	//screen.SaveToFile(scaled, targetRes, NameImage(targetRes));
	Scaler::method = Scaler::ScaleMethod::Bicubic;
	auto begin = std::chrono::high_resolution_clock::now();
	auto scaled = Scaler::Scale(image, sourceRes, targetRes);
	auto end = std::chrono::high_resolution_clock::now();

	std::cout << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << std::endl;

	screen.SaveToFile(scaled, targetRes, NameImage(targetRes));


    return 0;
}
