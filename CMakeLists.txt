cmake_minimum_required(VERSION 3.22)
project(pixelwindow)

option(BUILD_DEMOS OFF)

# PixelWindow header library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

# PixelWindow requires C++17
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
find_package(X11 REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE X11::X11)
endif()

# Alias for external projects
add_library(PixelWindow::Headers ALIAS ${PROJECT_NAME})

if(BUILD_DEMOS)
add_subdirectory(demos)
endif()
