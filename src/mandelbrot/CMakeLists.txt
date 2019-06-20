find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(OpenCV REQUIRED)

# Define the name of the base library and all source files belonging to it
add_library(
  mandelbrot_lib
  src/mandelbrot/mandelbrot.cpp)

target_link_libraries(mandelbrot_lib 
  base_lib_header_only
  base_lib
  spline_lib)

# define the target links: specify how the libs shall be included.
target_include_directories(mandelbrot_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
