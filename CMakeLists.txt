cmake_minimum_required(VERSION 3.10)

project(test)
set(CMAKE_CXX_STANDARD 11)

set(OpenCV_DIR ${PROJECT_SOURCE_DIR}/include/opencv/build)
set(SQLite3_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/sqlite3/include)
set(SQLite3_LIBRARY ${PROJECT_SOURCE_DIR}/include/sqlite3/lib/sqlite3.lib)

file(GLOB_RECURSE SOURCES "src/*.cpp")


include_directories(${PROJECT_SOURCE_DIR}/src/face)
include_directories(${PROJECT_SOURCE_DIR}/src/camera)
include_directories(${PROJECT_SOURCE_DIR}/src/database)
include_directories(${PROJECT_SOURCE_DIR}/src/ippg)
include_directories(${PROJECT_SOURCE_DIR}/src/controller)

find_package(OpenCV REQUIRED)
find_package(SQLite3 REQUIRED)



add_executable(test ${SOURCES})

target_link_libraries(test PRIVATE
    ${OpenCV_LIBS}
    SQLite::SQLite3)