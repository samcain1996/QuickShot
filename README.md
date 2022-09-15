# QuickShot

Quickly take screenshots, programatically, on Linux, Windows, and macOS

## Building

Create a new directory to build the project in

```
mkdir build
cd build
```

If you want just the library execute the following:

```
cmake ../
```

If you also want a demo of the basic functionality then execute the following:

```
cmake -DDEMO=ON ../
```

### Linux and macOS

After CMake is finished, run `make` to create the library and/or demo executable

### Windows

Launch the solution located in the build directory

## Use

1. Include "Capture.h" in your source files
2. Instantiate a `ScreenCapture` object. The default resolution of a screen shot is 1920x1080. The default resolution can be adjusted in Capture.h. 
    a. A resolution other than the default resolution can be used by providing the width and height in the object constructor
3. To capture the current screen, call the `CaptureScreen` function which returns an vector representing the pixeldata of the screen. (Note: CaptureScreen does not return any metadata or header, just the raw pixels)
4. To capture the screen in the bitmap format, call the `WholeDeal` function. This function MUST be called after `CaptureScreen`
5. To save a captured image to disk, call the `SaveToFile` function after capturing the screen.
