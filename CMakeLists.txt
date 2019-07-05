cmake_minimum_required(VERSION 3.10)
project(RayTraceCore VERSION 1.0.0 DESCRIPTION "This library provides a toolset for raytracing on cpu and gpu")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17 ")#-Ofast -march=native")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

include(GNUInstallDirs)

find_package(Threads)
find_package(OpenCL REQUIRED)

add_library(RayTraceCore SHARED API/RayEngine.h #"src/Data Management Unit/DataManagementUnit.cpp" "src/Data Management Unit/HBVH/HBVH.cpp" "src/Data Management Unit/HBVH/SceneData.cpp"
        src/RayEngine.cpp src/Pipeline/Pipeline.cpp API/RayGeneratorShader.h API/OcclusionShader.h API/PierceShader.h API/HitShader.h API/MissShader.h API/RayTraceCore.h API/ControlShader.h src/Pipeline/Shader/Data.h)

set_target_properties(RayTraceCore PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1 PUBLIC_HEADER "API/RayTraceCore.h;API/RayEngine.h;API/MissShader.h;API/HitShader.h;API/OcclusionShader.h;API/PierceShader.h;API/RayGeneratorShader.h")

configure_file(RayTraceCore.pc.in RayTraceCore.pc @ONLY)
target_include_directories(RayTraceCore PRIVATE .)
install(TARGETS RayTraceCore LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/RayTraceCore.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)