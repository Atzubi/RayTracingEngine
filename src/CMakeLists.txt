# Get all source files in the src directory and its subdirectories
file(GLOB_RECURSE LIBRARY_SOURCES ./*.cpp)
file(GLOB_RECURSE LIBRARY_HEADERS ./*.h)
file(GLOB_RECURSE INTERFACE_HEADERS ../include/*.h)

add_library(RayTraceEngine SHARED ${LIBRARY_SOURCES})

target_sources(RayTraceEngine PRIVATE ${LIBRARY_HEADERS} ${INTERFACE_HEADERS})
target_include_directories(RayTraceEngine PRIVATE .)
set_target_properties(RayTraceEngine PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

# For MacOS Framework
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message(STATUS "MacOS detected, PUBLIC_HEADER has been set.")
    set_target_properties(RayTraceEngine PROPERTIES PUBLIC_HEADER "include/RayTraceEngine/RayTraceCore.h;include/RayTraceEngine/RayEngine.h;include/RayTraceEngine/MissShader.h;include/RayTraceEngine/HitShader.h;include/RayTraceEngine/OcclusionShader.h;include/RayTraceEngine/PierceShader.h;include/RayTraceEngine/RayGeneratorShader.h;include/RayTraceEngine/Object.h;include/RayTraceEngine/Pipeline.h;include/RayTraceEngine/Shader.h;include/RayTraceEngine/BasicStructures.h;include/RayTraceEngine/TriangleMeshObject.h;include/RayTraceEngine/Scene.h")
endif ()

# For Linux Framework
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message(STATUS "Linux detected, linking pthread")
    set(CMAKE_CXX_FLAGS "-pthread")
endif ()

configure_file(RayTraceEngine.pc.in RayTraceEngine.pc @ONLY)
