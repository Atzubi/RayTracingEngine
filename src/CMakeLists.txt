cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

find_package(Threads)

add_library(atzubi-rtengine SHARED RayEngine.cpp Pipeline/PipelineImplement.cpp Object/TriangleMeshObject.cpp "Data Management/DataManagementUnit.cpp" "Acceleration Structures/DBVH.cpp" Object/Instance.cpp)

set_target_properties(atzubi-rtengine PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1 PUBLIC_HEADER "include/RayTraceCore.h;include/RayEngine.h;include/MissShader.h;include/HitShader.h;include/OcclusionShader.h;include/PierceShader.h;include/RayGeneratorShader.h;include/Object.h;include/Pipeline.h;include/Shader.h;include/BasicStructures.h;include/TriangleMeshObject.h")

configure_file(atzubi-rtengine.pc.in atzubi-rtengine.pc @ONLY)
target_include_directories(atzubi-rtengine PRIVATE .)
target_include_directories(atzubi-rtengine PRIVATE ../)

install(TARGETS atzubi-rtengine LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/atzubi-rtengine.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)