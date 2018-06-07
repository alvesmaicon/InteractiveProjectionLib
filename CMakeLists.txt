cmake_minimum_required(VERSION 2.8.4)
project(interactive_projection)

set(EXEC_NAME sample)

find_package(OpenCV REQUIRED) #OpenCV

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

set(INCLUDE_DIR ./include)

include_directories(${INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} pkg-config --cflags --libs opencv") 

set(SOURCE_FILES
  sample.cpp
  src/interactive_projection.cpp)

add_executable(${EXEC_NAME} ${SOURCE_FILES})

target_link_libraries(${EXEC_NAME} ${OpenCV_LIBS})
