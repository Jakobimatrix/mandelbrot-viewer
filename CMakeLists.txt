cmake_minimum_required(VERSION 3.10.2)
project(mandelbroetchen VERSION 1.0.0)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

add_subdirectory(src/base)
add_subdirectory(src/spline)
add_subdirectory(src/mandelbrot)
add_subdirectory(src/display)
add_subdirectory(src/executables)
