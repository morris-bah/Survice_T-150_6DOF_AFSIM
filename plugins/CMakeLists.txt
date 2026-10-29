# ****************************************************************************
# T-150 UAS Plugins - Top-level CMake Build Configuration
#
# Builds custom C++ plugins for the T-150 UAS simulation against
# an installed AFSIM distribution.
#
# Supports both Linux (.so) and Windows (.dll) builds.
#
# Copyright 2025 - Project 005 T-150 UAS Simulation
# ****************************************************************************

cmake_minimum_required(VERSION 3.16)
project(t150_plugins VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard (AFSIM 2.2505 requires C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific AFSIM default paths
if(WIN32)
    # Windows default AFSIM installation paths (adjust as needed)
    set(AFSIM_DEFAULT_ROOT "C:/code/AFSIM/AFSIM")
else()
    # Linux/Unix default path
    set(AFSIM_DEFAULT_ROOT "/opt/afsim2.2505")
endif()

set(AFSIM_ROOT "${AFSIM_DEFAULT_ROOT}" CACHE PATH "AFSIM installation directory")
list(APPEND CMAKE_PREFIX_PATH "${AFSIM_ROOT}/lib/cmake/afsim")

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()
find_package(afsim REQUIRED)

# The Windows distribution places executables directly in the install prefix.
if(WIN32)
    set(AFSIM_PLUGIN_INSTALL_DIR "mission_plugins" CACHE STRING
        "Mission plugin directory relative to CMAKE_INSTALL_PREFIX")
    set(AFSIM_SHARED_PLUGIN_INSTALL_DIR "wsf_plugins" CACHE STRING
        "Shared plugin directory relative to CMAKE_INSTALL_PREFIX")
    set(AFSIM_GRAMMAR_INSTALL_DIR "grammar" CACHE STRING
        "Grammar directory relative to CMAKE_INSTALL_PREFIX")
else()
    set(AFSIM_PLUGIN_INSTALL_DIR "lib/wsf_plugins")
    set(AFSIM_GRAMMAR_INSTALL_DIR "bin/grammar")
endif()

function(t150_install_plugin target)
    install(TARGETS ${target}
        LIBRARY DESTINATION "${AFSIM_PLUGIN_INSTALL_DIR}"
        RUNTIME DESTINATION "${AFSIM_PLUGIN_INSTALL_DIR}"
    )
    if(WIN32)
        install(FILES "$<TARGET_FILE:${target}>"
            DESTINATION "${AFSIM_SHARED_PLUGIN_INSTALL_DIR}")
    endif()
endfunction()

message(STATUS "==============================================")
message(STATUS "T-150 UAS Plugins Build Configuration")
message(STATUS "==============================================")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "AFSIM Version: ${afsim_VERSION}")
message(STATUS "AFSIM Runtime: ${AFSIM_ROOT}")
message(STATUS "Configurations: ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "==============================================")

# Set output directory for built plugins
# On Windows, DLLs go to bin/, on Linux .so files go to lib/
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Windows-specific settings
if(WIN32)
    # Ensure DLLs are placed alongside import libraries
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

    # Multi-config generators (Visual Studio) need per-config output dirs
    foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} ${CMAKE_BINARY_DIR}/lib)
    endforeach()
endif()

# Add each plugin subdirectory
add_subdirectory(t150_battery)
add_subdirectory(t150_vrs)
add_subdirectory(t150_propulsion)
add_subdirectory(t150_slung_load)
add_subdirectory(t150_weapon)
add_subdirectory(rotorcraft_6dof)

message(STATUS "")
message(STATUS "T-150 Plugins configured:")
message(STATUS "  - t150_battery (Battery SOC/energy modeling)")
message(STATUS "  - t150_vrs (Vortex Ring State detection)")
message(STATUS "  - t150_propulsion (Electric propulsion system)")
message(STATUS "  - t150_slung_load (Slung load pendulum dynamics)")
message(STATUS "  - t150_weapon (Weapon/launcher recoil physics)")
message(STATUS "  - rotorcraft_6dof (6DOF physics-based rotorcraft mover)")
message(STATUS "")
