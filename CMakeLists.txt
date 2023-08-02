cmake_minimum_required(VERSION 3.16)

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()


project(
    ircc
    VERSION 0.6.0
    DESCRIPTION "Barebones IRC client"
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(NOT FRONTEND_LIBRARY)
    set(FRONTEND_LIBRARY frontend-ncurses)
    message("FRONTEND_LIBRARY undefined, defaulting to '${FRONTEND_LIBRARY}'")
endif()


add_subdirectory(src)
