add_library(RayTraceEngine SHARED RayEngine.cpp Pipeline/PipelineImplement.cpp Object/TriangleMeshObject.cpp Object/Instance.cpp "Engine Node/EngineNode.h" "Engine Node/EngineNode.cpp" "Acceleration Structures/DBVHv2.h" "Data Management/DataManagementUnitV2.h" "Data Management/DataManagementUnitV2.cpp" "Acceleration Structures/DBVHv2.cpp")

target_include_directories(RayTraceEngine PRIVATE .)
set_target_properties(RayTraceEngine PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

# For MacOS Framework
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message(STATUS "MacOS detected, PUBLIC_HEADER has been set.")
    set_target_properties(RayTraceEngine PROPERTIES PUBLIC_HEADER "include/RayTraceEngine/RayTraceCore.h;include/RayTraceEngine/RayEngine.h;include/RayTraceEngine/MissShader.h;include/RayTraceEngine/HitShader.h;include/RayTraceEngine/OcclusionShader.h;include/RayTraceEngine/PierceShader.h;include/RayTraceEngine/RayGeneratorShader.h;include/RayTraceEngine/Object.h;include/RayTraceEngine/Pipeline.h;include/RayTraceEngine/Shader.h;include/RayTraceEngine/BasicStructures.h;include/RayTraceEngine/TriangleMeshObject.h")
endif ()

# Compiler optimisations
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message(STATUS "Linux detected, enabled compiler optimisations..")
    set(CMAKE_CXX_FLAGS "-pthread -O0 -march=native")
endif ()

configure_file(RayTraceEngine.pc.in RayTraceEngine.pc @ONLY)
