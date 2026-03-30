cmake_minimum_required(VERSION 3.10)

project(test)
set(CMAKE_CXX_STANDARD 11)

# 启用 OpenMP 支持
find_package(OpenMP REQUIRED)

set(OpenCV_DIR ${PROJECT_SOURCE_DIR}/include/opencv/build)

file(GLOB_RECURSE SOURCES "src/*.cpp")

include_directories(${PROJECT_SOURCE_DIR}/src/face)
include_directories(${PROJECT_SOURCE_DIR}/src/camera)
include_directories(${PROJECT_SOURCE_DIR}/src/ippg)
include_directories(${PROJECT_SOURCE_DIR}/src/controller)

find_package(OpenCV REQUIRED)

add_executable(test ${SOURCES})

# 链接 OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(test PUBLIC OpenMP::OpenMP_CXX)
endif()

target_link_libraries(test PRIVATE
    ${OpenCV_LIBS})

