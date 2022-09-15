# QuickShot

Quickly take screenshots, programatically, on Linux, Windows, and macOS

# TODO

Compile to shared library

## Building

Building is only needed to view demos

```
mkdir build
cd build
cmake ../
```

## Demo

A demo is included with QuickShot to demonstrate its basic functionality

### Linux and macOS

```
make
```

### Windows

Launch the solution created by CMake

## Use

If this code is used in a separate project and the target OS is NOT Windows then the following must be done:

### Linux

Link the X11 library in your build command. `-lX11`

### macOS

Link the Application Services framework in your build command. `-framework ApplicationServices`


1. Include "Capture.h" in your headers and "Capture.cpp" in your source files
2. Instantiate a `ScreenCapture` object. The default resolution of a screen shot is 1920x1080. The default resolution can be adjusted in Capture.h. 
    a. A resolution other than the default resolution can be used by providing the width and height in the object constructor
3. To capture the current screen, call the `CaptureScreen` function which returns an vector representing the pixeldata of the screen. (Note: CaptureScreen does not return any metadata or header, just the raw pixels)
4. To capture the screen in the bitmap format, call the `WholeDeal` function. This function MUST be called after `CaptureScreen`
5. To save a captured image to disk, call the `SaveToFile` function after capturing the screen.