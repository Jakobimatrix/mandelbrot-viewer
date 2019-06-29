find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Define the name of the base library and all source files belonging to it
add_library(
  display_lib
  src/display/display.cpp
  src/display/displayOpenCV.cpp)

target_link_libraries(display_lib 
  base_lib_header_only
  base_lib
  mandelbrot_lib
  timer_lib)

# define the target links: specify how the libs shall be included.
target_include_directories(display_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

