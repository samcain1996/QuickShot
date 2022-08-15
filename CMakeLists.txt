cmake_minimum_required(VERSION 3.10)

project(QuickShotDemo)
project(QuickShotBench)

set(CMAKE_CXX_STANDARD 20)

if (APPLE)

find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices REQUIRED)
link_libraries(${APPLICATIONSERVICES_LIBRARY})

elseif (UNIX)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

endif()

add_executable(QuickShotDemo Capture.cpp Demo.cpp)
add_executable(QuickShotBench Capture.cpp bench.cpp)
