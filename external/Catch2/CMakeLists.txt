project(Catch2)
add_library(Catch2 STATIC Catch2/catch_amalgamated.hpp Catch2/catch_amalgamated.cpp)
target_include_directories(Catch2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})