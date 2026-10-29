cmake_minimum_required(VERSION 3.24)
project(t150_uas VERSION 1.0.0 LANGUAGES CXX)

if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "C:/code/AFSIM/AFSIM" CACHE PATH
        "AFSIM runtime installation directory" FORCE)
endif()

add_subdirectory(plugins)

include(CTest)
if(BUILD_TESTING AND WIN32)
    add_test(NAME windows-installed-plugins
        COMMAND "${CMAKE_COMMAND}"
            "-DAFSIM_RUNTIME_DIR=${CMAKE_INSTALL_PREFIX}"
            "-DPLUGIN_BUILD_DIR=${CMAKE_BINARY_DIR}/lib"
            "-DPLUGIN_INSTALL_DIR=${AFSIM_PLUGIN_INSTALL_DIR}"
            "-DSHARED_PLUGIN_INSTALL_DIR=${AFSIM_SHARED_PLUGIN_INSTALL_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_installed_plugins.cmake"
    )
endif()
