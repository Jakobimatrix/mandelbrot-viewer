find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

include_directories( ${Boost_INCLUDE_DIRS} )

add_executable(mandelbroetchen_start src/main.cpp)

list(APPEND mandelbroetchen_start_SOURCES
  PRIVATE base_lib_header_only
  PRIVATE base_lib
  PRIVATE mandelbrot_lib
  PRIVATE display_lib
  Eigen3::Eigen
  ${OpenCV_LIBS}
  ${Boost_LIBRARIES}
)

target_link_libraries(mandelbroetchen_start ${mandelbroetchen_start_SOURCES})
