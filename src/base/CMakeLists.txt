find_package (Eigen3 3.3 REQUIRED NO_MODULE)

# define a header only library with key INTERFACE 
# By specifying INTERFACE as the second parameter to add_library, 
# we are no longer allowed to provide source files since the library 
# is not meant to generate any build output.
add_library(base_lib_header_only INTERFACE)
# If PRIVATE is specified for a certain option/property, 
# then that option/property will only impact the current target. 
# If PUBLIC is specified, then the option/property impacts 
# both the current target and any others that link to it. 
# If INTERFACE is specified, then the option/property does not impact 
# the current target but will propagate to other targets that link to it.

# Define the name of the base library and all source files belonging to it
# Only provide the .cpp files here
add_library(
  base_lib
  src/base/conversations.cpp
  src/base/planarTransformation.cpp
  src/base/randomGenerators.cpp)

# link base_lib_header_only to base_lib
target_link_libraries(base_lib
  base_lib_header_only
  homography_lib)

# define the target links: specify how the libs shall be included.
target_include_directories(base_lib_header_only INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(base_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
