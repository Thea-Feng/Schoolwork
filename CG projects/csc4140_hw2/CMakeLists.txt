cmake_minimum_required(VERSION 3.10)

project(MVP)

find_package(Eigen3 CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

add_executable(MVP)
target_sources(MVP PRIVATE main.cpp rasterizer.cpp Triangle.cpp)
target_compile_features(MVP PRIVATE cxx_std_17)
target_link_libraries(MVP PRIVATE Eigen3::Eigen opencv_highgui)