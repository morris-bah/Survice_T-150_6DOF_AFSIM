# ****************************************************************************
# T-150 UAS VRS Processor Plugin - Source CMake Configuration
#
# Copyright 2025 - Project 005 T-150 UAS Simulation
# ****************************************************************************

include(GenerateExportHeader)
include(swdev_project)

FILE(GLOB SRCS *.cpp *.hpp)

add_library(${PROJECT_NAME} ${SRCS})
generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          "${PROJECT_BINARY_DIR}/source")

target_link_libraries(${PROJECT_NAME} ${WSF_LIBS})
swdev_warning_level(${PROJECT_NAME})
swdev_plugin_install(${PROJECT_NAME} wsf_plugins)
