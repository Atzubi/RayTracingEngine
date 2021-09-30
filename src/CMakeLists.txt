add_library(RayTraceEngine SHARED RayEngine.cpp Pipeline/PipelineImplement.cpp Object/TriangleMeshObject.cpp "Data Management/DataManagementUnit.cpp" "Acceleration Structures/DBVH.cpp" Object/Instance.cpp)

target_include_directories(RayTraceEngine PRIVATE .)
set_target_properties(RayTraceEngine PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1 PUBLIC_HEADER "API/RayTraceCore.h;API/RayEngine.h;API/MissShader.h;API/HitShader.h;API/OcclusionShader.h;API/PierceShader.h;API/RayGeneratorShader.h;API/Object.h;API/Pipeline.h;API/Shader.h;API/BasicStructures.h;API/TriangleMeshObject.h")

configure_file(RayTraceEngine.pc.in RayTraceEngine.pc @ONLY)
