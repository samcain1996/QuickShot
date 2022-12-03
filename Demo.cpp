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

const std::string imageDimensions(const Resolution& resolution) {
	return std::to_string(resolution.width) + "x" + std::to_string(resolution.height);
}

int main(int argc, char** argv) {

	Resolution sourceRes = ScreenCapture::NativeResolution();
	Resolution targetRes = sourceRes / 2;

	int xOffset = 250, yOffset = 100;
	ScreenArea captureArea(RES_720, xOffset, yOffset);
    ScreenCapture screen(sourceRes/*, captureArea*/);

	PixelData image = screen.CaptureScreen();
	std::string name = "Original" + imageDimensions(screen.GetResolution());
	screen.SaveToFile(name);

	Scaler::method = Scaler::ScaleMethod::NearestNeighbor;
	PixelData scaled = Scaler::Scale(image, sourceRes, targetRes);
	name = "NearestNeighbor" + imageDimensions(targetRes);
	screen.SaveToFile(scaled, targetRes, name);

	Scaler::method = Scaler::ScaleMethod::Bilinear;
	scaled = Scaler::Scale(image, sourceRes, targetRes);
	name = "Bilinear" + imageDimensions(targetRes);
	screen.SaveToFile(scaled, targetRes, "Bilinear" + imageDimensions(targetRes));

    return 0;
}
