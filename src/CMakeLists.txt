add_library(RayTraceEngine SHARED RayEngine.cpp Pipeline/PipelineImplement.cpp Object/TriangleMeshObject.cpp "Data Management/DataManagementUnit.cpp" "Acceleration Structures/DBVH.cpp" Object/Instance.cpp)

target_include_directories(RayTraceEngine PRIVATE .)
set_target_properties(RayTraceEngine PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

# For MacOS Framework
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(RayTraceEngine PROPERTIES PUBLIC_HEADER "include/API/RayTraceCore.h;include/API/RayEngine.h;include/API/MissShader.h;include/API/HitShader.h;include/API/OcclusionShader.h;include/API/PierceShader.h;include/API/RayGeneratorShader.h;include/API/Object.h;include/API/Pipeline.h;include/API/Shader.h;include/API/BasicStructures.h;include/API/TriangleMeshObject.h")
endif()

configure_file(RayTraceEngine.pc.in RayTraceEngine.pc @ONLY)
