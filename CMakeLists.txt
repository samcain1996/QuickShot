cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(DEMO OFF CACHE BOOL "Build demo")

if (DEMO)
project(QuickShotDemo)
add_executable(QuickShotDemo Capture.cpp Demo.cpp)
endif()

project(QuickShot VERSION 1.0.0 DESCRIPTION "Programatically take screenshot")
add_library(QuickShot SHARED Capture.cpp)

if (APPLE)

find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices REQUIRED)
link_libraries(${APPLICATIONSERVICES_LIBRARY})

elseif (UNIX)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

endif()

set_target_properties(QuickShot PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER Capture.h)

target_include_directories(QuickShot PRIVATE .)

install(TARGETS QuickShot
    LIBRARY DESTINATION .
    PUBLIC_HEADER DESTINATION .)
