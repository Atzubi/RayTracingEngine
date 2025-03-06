add_executable(ObjExample main.cpp)

file(GLOB OTHER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../Utils/*.cpp"
                        "${CMAKE_CURRENT_SOURCE_DIR}/../Utils/*.h"
                        "${CMAKE_CURRENT_SOURCE_DIR}/../Shaders/*.cpp"
                        "${CMAKE_CURRENT_SOURCE_DIR}/../Shaders/*.h")
target_sources(ObjExample PRIVATE ${OTHER_SOURCES})

find_package(SFML 2 COMPONENTS graphics REQUIRED)

target_link_libraries(ObjExample RayTraceEngine sfml-graphics)
include_directories(${SFML_INCLUDE_DIR})
target_include_directories(ObjExample PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_custom_command(TARGET ObjExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Data/"
        "$<TARGET_FILE_DIR:ObjExample>/Data"
        COMMENT "Copying data..."
        )

if (WIN32)
    add_custom_command(TARGET ObjExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Data/"
        "${CMAKE_CURRENT_BINARY_DIR}/Data"
        COMMENT "Copying data..."
        )

    add_custom_command(TARGET ObjExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:RayTraceEngine>
            $<TARGET_FILE_DIR:ObjExample>
            COMMENT "Copying RayTraceEngine.dll..."
        )

    add_custom_command(TARGET ObjExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SFML_DIR}/../../../bin"
            $<TARGET_FILE_DIR:ObjExample>
            COMMENT "Copying SFML dlls..."
        )
endif ()