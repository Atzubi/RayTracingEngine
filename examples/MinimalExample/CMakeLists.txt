add_executable(MinimalExample main.cpp)

find_package(SFML 2 COMPONENTS graphics REQUIRED)

target_link_libraries(MinimalExample RayTraceEngine sfml-graphics)
include_directories(${SFML_INCLUDE_DIR})
target_include_directories(MinimalExample PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")

if (WIN32)
    add_custom_command(TARGET MinimalExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:RayTraceEngine>
            $<TARGET_FILE_DIR:MinimalExample>
            COMMENT "Copying RayTraceEngine.dll..."
        )

    add_custom_command(TARGET MinimalExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SFML_DIR}/../../../bin"
            $<TARGET_FILE_DIR:MinimalExample>
            COMMENT "Copying SFML dlls..."
        )
endif ()