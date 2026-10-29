# ****************************************************************************
# T-150 UAS Battery Processor Plugin - Source CMake Configuration
#
# Copyright 2025 - Project 005 T-150 UAS Simulation
# ****************************************************************************

# Outside includes
include(GenerateExportHeader)
include(swdev_project)

# Add all source files
FILE(GLOB SRCS *.cpp *.hpp)

# Optionally add grammar file if we have one
# wsf_grammar_file(SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../grammar/t150_battery.ag")

# Add the library with sources
add_library(${PROJECT_NAME} ${SRCS})

# Generate header with the __declspec export macro defined
generate_export_header(${PROJECT_NAME})

# The includes for our export
target_include_directories(${PROJECT_NAME} PUBLIC
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          "${PROJECT_BINARY_DIR}/source")

# Link with required WSF libs
target_link_libraries(${PROJECT_NAME} ${WSF_LIBS})

# Set the project warning level
swdev_warning_level(${PROJECT_NAME})

# Install the plugin
swdev_plugin_install(${PROJECT_NAME} wsf_plugins)
