# QuickShot

Library to quickly take screenshots, programatically, on Linux, Windows, and macOS

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

Launch the solution created by CMake

## Use

If this code is used in a separate project and the target OS is NOT Windows then the following must be done:

### Linux

Link the X11 library in your build command. `-lX11`

### macOS

Link the Application Services framework in your build command. `-framework ApplicationServices`
