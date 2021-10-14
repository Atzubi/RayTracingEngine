add_library(RayTraceEngine SHARED RayEngine.cpp Pipeline/PipelineImplement.cpp Object/TriangleMeshObject.cpp "Data Management/DataManagementUnit.cpp" "Acceleration Structures/DBVH.cpp" Object/Instance.cpp)

target_include_directories(RayTraceEngine PRIVATE .)
set_target_properties(RayTraceEngine PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

# For MacOS Framework
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(RayTraceEngine PROPERTIES PUBLIC_HEADER "include/RayTraceEngine/RayTraceCore.h;include/RayTraceEngine/RayEngine.h;include/RayTraceEngine/MissShader.h;include/RayTraceEngine/HitShader.h;include/RayTraceEngine/OcclusionShader.h;include/RayTraceEngine/PierceShader.h;include/RayTraceEngine/RayGeneratorShader.h;include/RayTraceEngine/Object.h;include/RayTraceEngine/Pipeline.h;include/RayTraceEngine/Shader.h;include/RayTraceEngine/BasicStructures.h;include/RayTraceEngine/TriangleMeshObject.h")
endif()

configure_file(RayTraceEngine.pc.in RayTraceEngine.pc @ONLY)