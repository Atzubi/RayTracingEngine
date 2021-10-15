add_executable(MinimalExample main.cpp)

if (WIN32)
    set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}../../../SFML-2.5.1/lib/cmake/SFML")
endif ()

find_package(SFML 2 COMPONENTS graphics REQUIRED)

target_link_libraries(MinimalExample RayTraceEngine)
target_link_libraries(MinimalExample sfml-graphics)

add_custom_command(TARGET MinimalExample POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/post_build.sh ${CMAKE_CURRENT_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Copying data..."
        )

if (WIN32)
    add_custom_command(TARGET MinimalExample POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/lib/RayTraceEngine.dll"
            $<TARGET_FILE_DIR:MinimalExample>
            COMMENT "Copying RayTraceEngine.dll..."
            )

    add_custom_command(TARGET MinimalExample POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}../../../SFML-2.5.1/bin"
            $<TARGET_FILE_DIR:MinimalExample>
            COMMENT "Copying SFML dlls..."
            )
endif ()