cmake_minimum_required(VERSION 3.22)
project(basic)

# Global CMake Configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project Paths
set(PRJ_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(DEMOS_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# Include Directories
set(TARGET_INC_DIRS ${SRC_DIR} ${DEMOS_DIR}/common ${PRJ_DIR}/include)

# Sources
set(TARGET_SRCS ${SRC_DIR}/main.cpp)

# Output Executable
add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${TARGET_INC_DIRS})
target_sources(${PROJECT_NAME} PRIVATE ${TARGET_SRCS})

if(UNIX)
target_link_libraries(${PROJECT_NAME} PRIVATE X11)
endif()
