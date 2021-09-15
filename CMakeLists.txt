cmake_minimum_required(VERSION 3.10)
project(RayTraceEngine VERSION 1.0.0 DESCRIPTION "This library provides a toolset for raytracing on cpu and gpu")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17 -O0")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

include(GNUInstallDirs)

find_package(Threads)

add_library(RayTraceEngine SHARED src/RayEngine.cpp src/Pipeline/PipelineImplement.cpp src/Object/TriangleMeshObject.cpp "src/Data Management/DataManagementUnit.cpp" "src/Acceleration Structures/DBVH.cpp" src/Object/Instance.cpp)

set_target_properties(RayTraceEngine PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1 PUBLIC_HEADER "API/RayTraceCore.h;API/RayEngine.h;API/MissShader.h;API/HitShader.h;API/OcclusionShader.h;API/PierceShader.h;API/RayGeneratorShader.h;API/Object.h;API/Pipeline.h;API/Shader.h;API/BasicStructures.h;API/TriangleMeshObject.h")

configure_file(RayTraceEngine.pc.in RayTraceEngine.pc @ONLY)
target_include_directories(RayTraceEngine PRIVATE .)

install(TARGETS RayTraceEngine LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/RayTraceEngine.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)