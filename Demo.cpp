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

const std::string nameFile(const Resolution& resolution,const std::string& additional = "") {
	
    return std::to_string(resolution.width) + "x" + std::to_string(resolution.height) + additional + ".bmp";
	
}

int main(int argc, char** argv) {

	Resolution targetRes = RES_4K;
	Resolution sourceRes = RES_1080;

	int xOffset = 150, yOffset = 200;
	ScreenArea captureArea(xOffset, RES_720.width + xOffset,
		yOffset, RES_720.height + yOffset);
    ScreenCapture screen(sourceRes, captureArea);

	auto image = screen.CaptureScreen();
	screen.SaveToFile("Original" + nameFile(screen.GetResolution()));

	Scaler::method = Scaler::ScaleMethod::NearestNeighbor;
	auto scaled = Scaler::Scale(image, sourceRes, targetRes);
	screen.SaveToFile(scaled, targetRes, "NearestNeighbor" + nameFile(targetRes));

	Scaler::method = Scaler::ScaleMethod::Bilinear;
	scaled = Scaler::Scale(image, sourceRes, targetRes);
	screen.SaveToFile(scaled, targetRes, "Bilinear" + nameFile(targetRes));

    return 0;
}
