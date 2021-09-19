cmake_minimum_required(VERSION 3.20)
project(MinimalExample)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++20 -O0")

add_executable(MinimalExample main.cpp)

find_library(RAY_TRACE_ENGINE NAMES RayTraceEngine)

target_link_libraries(MinimalExample ${RAY_TRACE_ENGINE})

find_package(SFML 2 COMPONENTS network graphics audio window system REQUIRED)
target_link_libraries(MinimalExample sfml-graphics sfml-audio)