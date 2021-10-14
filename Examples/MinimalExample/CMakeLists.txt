add_executable(MinimalExample main.cpp)

find_package(SFML 2 COMPONENTS graphics REQUIRED)

target_link_libraries(MinimalExample RayTraceEngine)
target_link_libraries(MinimalExample sfml-graphics)

add_custom_command(TARGET MinimalExample POST_BUILD
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/post_build.sh ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Copying data..."
)