# ==============================================================================
# libSFM customer C++ example
#
# Prerequisites (install manually — see ../README.md for the version matrix):
#   - libsfm-dev (Debian package supplied by PLAIF)
#   - libopencv-dev
#
# Build:
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build -j
#
# Run (from anywhere — paths are resolved relative to the project root):
#   ./build/stereo_example       # reads ../input/, writes ../output/
#   ./build/stereo_example 1     # use GPU 1
# ==============================================================================
cmake_minimum_required(VERSION 3.20)
project(lib-sfm-cpp-examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(sfm REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

add_executable(stereo_example src/stereo_example.cpp)

target_link_libraries(stereo_example
    PRIVATE
        sfm::sfm
        ${OpenCV_LIBS}
)

target_include_directories(stereo_example
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

# Resolve input/ and output/ relative to this project's root so the binary can
# be launched from any working directory.
target_compile_definitions(stereo_example
    PRIVATE
        EXAMPLE_PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/.."
)
