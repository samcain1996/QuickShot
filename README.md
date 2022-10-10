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
