cmake_minimum_required(VERSION 3.0.0)
project(SolidumEngine VERSION 0.1.0)

add_compile_options(-std=c++17)

include_directories("Solidum/Containers/include")
include_directories("Solidum/EngineCore/include")
include_directories("SolidumAPI/include")

file(GLOB_RECURSE SOURCES *.cpp)

add_executable(SolidumEngine main.cpp)

set(CMAKE_BUILD_TYPE Debug)
