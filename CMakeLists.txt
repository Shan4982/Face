cmake_minimum_required(VERSION 3.10)

project(test)
set(CMAKE_CXX_STANDARD 11)

set(OpenCV_DIR "E:/astu/opencv/build")
#set(SQLite_DIR "G:\\head\\sqlite-amalgamation\\sqlite-amalgamation-3500400")
file(GLOB_RECURSE SOURCES "src/*.cpp")

include_directories(${PROJECT_SOURCE_DIR}/src/face)
include_directories(${PROJECT_SOURCE_DIR}/src/camera)

find_package(OpenCV REQUIRED)
set(libs ${OpenCV_LIBS}
        )

include_directories(${SQLite3_INCLUDE_DIRS})

add_executable(test ${SOURCES})

target_link_libraries(test PRIVATE ${libs})
