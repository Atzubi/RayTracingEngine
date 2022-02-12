cmake_minimum_required(VERSION 3.10)

project(RayTraceEngine VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "This library provides a toolset for raytracing on cpu and gpu")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ATZUBI_RTENGINE_IS_MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(ATZUBI_RTENGINE_IS_MAIN_PROJECT ON)
endif ()

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    set(BUILD_SHARED_LIBS TRUE)
endif ()


# installation directories
include(GNUInstallDirs)

include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${CMAKE_SOURCE_DIR}/include)
# this may also include the source tree for the examples which is undesirable
include_directories(${CMAKE_SOURCE_DIR}/src)

#set(ATZUBI_RTENGINE_INSTALL_DOC_DIR "${CMAKE_INSTALL_DOCDIR}/atzubi/rtengine" CACHE STRING "The installation doc directory")
set(ATZUBI_RTENGINE_INSTALL_CMAKE_DIR "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake" CACHE STRING "The installation cmake directory")


add_subdirectory(src lib)
add_library(atzubi::rtengine ALIAS RayTraceEngine)
target_include_directories(RayTraceEngine INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

# examples
option(ATZUBI_RTENGINE_BUILD_EXAMPLES "Build example programs" 0)
if (ATZUBI_RTENGINE_BUILD_EXAMPLES)
    add_subdirectory(Examples/MinimalExample example/MinimalExample)
endif ()

option(ATZUBI_RTENGINE_INSTALL "Generate the install target" ${ATZUBI_RTENGINE_IS_MAIN_PROJECT})
if (ATZUBI_RTENGINE_INSTALL)
    include(CMakePackageConfigHelpers)

    # Make package findable
    configure_package_config_file(.cmake/${PROJECT_NAME}-config.cmake.in ${PROJECT_NAME}-config.cmake
            INSTALL_DESTINATION ${ATZUBI_RTENGINE_INSTALL_CMAKE_DIR}
            NO_CHECK_REQUIRED_COMPONENTS_MACRO
            NO_SET_AND_CHECK_MACRO
            )

    # Enable version checks in find_package
    write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake COMPATIBILITY SameMajorVersion)

    # install and export target
    install(TARGETS RayTraceEngine EXPORT ${PROJECT_NAME}-targets)
    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
            DESTINATION ${ATZUBI_RTENGINE_INSTALL_CMAKE_DIR}
            )
    install(EXPORT ${PROJECT_NAME}-targets
            NAMESPACE atzubi::
            DESTINATION ${ATZUBI_RTENGINE_INSTALL_CMAKE_DIR}
            )

    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    #install(FILES LICENSE DESTINATION ${ATZUBI_RTENGINE_INSTALL_DOC_DIR})


    export(EXPORT ${PROJECT_NAME}-targets
            FILE ${RayTraceEngine_BINARY_DIR}/${PROJECT_NAME}-targets.cmake
            NAMESPACE atzubi::
            )
endif ()
