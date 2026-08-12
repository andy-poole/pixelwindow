
add_executable(basic main.cpp)
target_include_directories(basic PRIVATE ../common)

# Include PixelWindow
target_link_libraries(basic
    PRIVATE
    PixelWindow::Headers
)
