cmake_minimum_required(VERSION 3.10)

add_executable(MinimalExample main.cpp)
target_link_libraries(MinimalExample RayTraceEngine)

find_package(SFML 2 COMPONENTS network graphics audio window system REQUIRED)
target_link_libraries(MinimalExample sfml-graphics sfml-audio)

add_custom_command(TARGET MinimalExample POST_BUILD
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/post_build.sh ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Copying data..."
)