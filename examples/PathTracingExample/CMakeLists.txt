add_executable(PathTracingExample main.cpp)

find_package(SFML 2 COMPONENTS graphics REQUIRED)

target_link_libraries(PathTracingExample RayTraceEngine sfml-graphics)
include_directories(${SFML_INCLUDE_DIR})
target_include_directories(PathTracingExample PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_custom_command(TARGET PathTracingExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Data/"
        "$<TARGET_FILE_DIR:PathTracingExample>/Data"
        COMMENT "Copying data..."
        )

if (WIN32)
    add_custom_command(TARGET PathTracingExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Data/"
        "${CMAKE_CURRENT_BINARY_DIR}/Data"
        COMMENT "Copying data..."
        )

    add_custom_command(TARGET PathTracingExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:RayTraceEngine>
            $<TARGET_FILE_DIR:PathTracingExample>
            COMMENT "Copying RayTraceEngine.dll..."
        )

    add_custom_command(TARGET PathTracingExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SFML_DIR}/../../../bin"
            $<TARGET_FILE_DIR:PathTracingExample>
            COMMENT "Copying SFML dlls..."
        )
endif ()