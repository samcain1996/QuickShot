cmake_minimum_required(VERSION 3.10)

#project(QuickShotDemo)
project(libQuickShot VERSION 1.0.0 DESCRIPTION "Programatically take screenshot")

set(CMAKE_CXX_STANDARD 20)

if (APPLE)

find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices REQUIRED)
link_libraries(${APPLICATIONSERVICES_LIBRARY})

elseif (UNIX)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

endif()

#add_executable(QuickShotDemo Capture.cpp Demo.cpp)
add_library(libQuickShot SHARED Capture.cpp)

set_target_properties(libQuickShot PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER Capture.h)
target_include_directories(libQuickShot PRIVATE .)
install(TARGETS libQuickShot
    LIBRARY DESTINATION .
    PUBLIC_HEADER DESTINATION .)

#install(FILES ${CMAKE_BINARY_DIR}/linQuickShot.pc
#    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
