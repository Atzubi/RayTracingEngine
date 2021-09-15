cmake_minimum_required(VERSION 3.20)
project(MinimalExample)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++20 -O0 -lsfml-graphics -lsfml-window -lsfml-system")

add_executable(MinimalExample main.cpp)

target_link_libraries(MinimalExample RayTraceEngine)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED network audio graphics window system)
include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(MinimalExample ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})