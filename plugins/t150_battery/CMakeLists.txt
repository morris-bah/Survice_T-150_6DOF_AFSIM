# ****************************************************************************
# T-150 UAS Battery Processor Plugin - CMake Build Configuration
#
# Copyright 2025 - Project 005 T-150 UAS Simulation
# ****************************************************************************

set(PLUGIN_NAME t150_battery_processor)

# Collect source files
file(GLOB PLUGIN_SOURCES
    source/*.cpp
    source/*.hpp
)

# Create shared library
add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES})

# Generate export header
include(GenerateExportHeader)
generate_export_header(${PLUGIN_NAME}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}_export.h
)

# Include directories
target_include_directories(${PLUGIN_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AFSIM_INCLUDE_DIR}
    ${AFSIM_INCLUDE_DIR}/wsf
)

# Link with AFSIM libraries
target_link_libraries(${PLUGIN_NAME}
    AFSIM::wsf
)

# Set properties
set_target_properties(${PLUGIN_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    PREFIX ""
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

t150_install_plugin(${PLUGIN_NAME})

# Install grammar file
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/grammar/t150_battery_processor.ag")
    install(FILES
        grammar/t150_battery_processor.ag
        DESTINATION "${AFSIM_GRAMMAR_INSTALL_DIR}"
    )
endif()
