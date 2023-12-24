cmake_minimum_required(VERSION 3.10)
project(MathVis)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_FLAGS "-O0 -g3")

file(GLOB_RECURSE sources src/**.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})

add_executable(MathVis ${sources})
target_link_libraries(${PROJECT_NAME} ${GTK_LIBRARIES})
