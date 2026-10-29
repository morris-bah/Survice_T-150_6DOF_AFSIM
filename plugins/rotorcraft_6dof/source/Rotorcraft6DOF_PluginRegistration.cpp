// ****************************************************************************
// Rotorcraft 6DOF Mover - Plugin Registration
//
// Entry points for AFSIM plugin loading.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "rotorcraft_6dof_export.h"
#include "Rotorcraft6DOF_Registration.hpp"

#include <memory>
#include <iostream>

#include "WsfPlugin.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"

namespace
{
    struct PluginLoadVerifier
    {
        PluginLoadVerifier()
        {
            std::cout << "[ROTORCRAFT_6DOF] ===== PLUGIN .SO FILE LOADED (global constructor) =====" << std::endl;
        }
    };
    static PluginLoadVerifier sVerifier;
}

extern "C"
{
    ROTORCRAFT_6DOF_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
    {
        aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                   WSF_PLUGIN_API_MINOR_VERSION,
                                   WSF_PLUGIN_API_SYSTEMABI_STRING);
    }

    ROTORCRAFT_6DOF_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
    {
        std::cout << "[ROTORCRAFT_6DOF] WsfPluginSetup() called!" << std::endl;
        if (!aApplication.ExtensionIsRegistered("rotorcraft_6dof_registration"))
        {
            std::cout << "[ROTORCRAFT_6DOF] Registering extension" << std::endl;
            aApplication.RegisterFeature("rotorcraft_6dof");
            aApplication.RegisterExtension(
                "rotorcraft_6dof_registration",
                std::make_unique<WsfDefaultApplicationExtension<rotorcraft_6dof::Rotorcraft6DOF_Registration>>());
        }
        else
        {
            std::cout << "[ROTORCRAFT_6DOF] Extension already registered" << std::endl;
        }
    }
}
