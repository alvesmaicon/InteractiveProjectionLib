# Minimum version of cmake
cmake_minimum_required(VERSION 2.8)

# Project name
project(InteractiveProjectionLib)


# OpenCV is required for build
find_package(OpenCV REQUIRED)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

if(CMAKE_VERSION VERSION_LESS "2.8.11")
  # Add OpenCV headers location to your include paths
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()

#  Include directories
include_directories ("${PROJECT_BINARY_DIR}")

# Adding shared library
add_library(iplib SHARED ${PROJECT_SOURCE_DIR}/src/source.cpp)

# Adding static library
#add_library(iplib_static STATIC ${PROJECT_SOURCE_DIR}/src/source.cpp)

# Link iplib with OpenCV libraries
target_link_libraries(iplib ${OpenCV_LIBS})

# Adding the install targets
install (TARGETS iplib EXPORT iplib DESTINATION bin)
install(EXPORT iplib NAMESPACE ip_ DESTINATION lib/iplib)
install (FILES include/iplib.hpp DESTINATION include/iplib)






