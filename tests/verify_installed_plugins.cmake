set(plugins
    rotorcraft_6dof
    t150_battery_processor
    t150_propulsion
    t150_slung_load
    t150_vrs
    t150_weapon
)

foreach(plugin IN LISTS plugins)
    set(built "${PLUGIN_BUILD_DIR}/${plugin}.dll")
    if(NOT EXISTS "${built}")
        message(FATAL_ERROR "Build the Release plugins first: missing ${built}")
    endif()
    file(SHA256 "${built}" expected_hash)
    foreach(directory IN ITEMS "${PLUGIN_INSTALL_DIR}" "${SHARED_PLUGIN_INSTALL_DIR}")
        set(installed "${AFSIM_RUNTIME_DIR}/${directory}/${plugin}.dll")
        if(NOT EXISTS "${installed}")
            message(FATAL_ERROR "Run the windows-install build preset: missing ${installed}")
        endif()
        file(SHA256 "${installed}" installed_hash)
        if(NOT installed_hash STREQUAL expected_hash)
            message(FATAL_ERROR "Installed DLL is stale: ${installed}")
        endif()
    endforeach()
endforeach()

execute_process(
    COMMAND "${AFSIM_RUNTIME_DIR}/mission.exe" -no-system-log -help
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    TIMEOUT 30
)
if(NOT result STREQUAL "0")
    message(FATAL_ERROR "AFSIM plugin loading failed (${result}):\n${output}\n${errors}")
endif()

string(REGEX MATCH "Plugins Loaded:.*Extensions Included:" loaded_plugins "${output}")
foreach(plugin IN LISTS plugins)
    string(FIND "${loaded_plugins}" "${plugin}.dll" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "AFSIM did not load ${plugin}.dll:\n${output}\n${errors}")
    endif()
endforeach()
message(STATUS "All six installed plugins match the build and load into AFSIM.")
