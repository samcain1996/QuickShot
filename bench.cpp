#include <chrono>
#include <iostream>
#include "Capture.h"

int main() {

    ScreenCapture screen(RES_1080);

    
    long long avgDuration = 0;

    int trials = 100;
    for (int trial = 0; trial < trials; trial++) {

        auto start = std::chrono::high_resolution_clock::now();

        screen.CaptureScreen();

        auto end = std::chrono::high_resolution_clock::now();

        avgDuration += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    avgDuration /= trials;

    std::cout << avgDuration << " microseconds\n";

    screen.SaveToFile();

}