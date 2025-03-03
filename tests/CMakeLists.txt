project(tests)

add_executable(Tests src/test_bvh.cpp)

target_link_libraries(Tests PRIVATE Catch2 RayTraceEngine)

if (WIN32)
    add_custom_command(TARGET Tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:RayTraceEngine>
            $<TARGET_FILE_DIR:Tests>
            COMMENT "Copying RayTraceEngine.dll..."
        )
endif()