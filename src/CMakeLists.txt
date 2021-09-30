cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

find_package(Threads)

add_library(RayTraceEngine SHARED RayEngine.cpp Pipeline/PipelineImplement.cpp Object/TriangleMeshObject.cpp "Data Management/DataManagementUnit.cpp" "Acceleration Structures/DBVH.cpp" Object/Instance.cpp)

set_target_properties(RayTraceEngine PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1 PUBLIC_HEADER "include/RayTraceCore.h;include/RayEngine.h;include/MissShader.h;include/HitShader.h;include/OcclusionShader.h;include/PierceShader.h;include/RayGeneratorShader.h;include/Object.h;include/Pipeline.h;include/Shader.h;include/BasicStructures.h;include/TriangleMeshObject.h")

configure_file(RayTraceEngine.pc.in RayTraceEngine.pc @ONLY)
target_include_directories(RayTraceEngine PRIVATE .)
target_include_directories(RayTraceEngine PRIVATE ../)

install(TARGETS RayTraceEngine LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/RayTraceEngine.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)