// ****************************************************************************
// T-150 UAS Weapon Processor Plugin - Plugin Registration
//
// Entry points for AFSIM plugin loading.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "t150_weapon_export.h"
#include "WeaponRegistration.hpp"

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
            std::cout << "[T150_WEAPON] ===== PLUGIN .SO FILE LOADED (global constructor) =====" << std::endl;
        }
    };
    static PluginLoadVerifier sVerifier;
}

extern "C"
{
    //! Plugin version check
    T150_WEAPON_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
    {
        aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                   WSF_PLUGIN_API_MINOR_VERSION,
                                   WSF_PLUGIN_API_SYSTEMABI_STRING);
    }

    //! Plugin setup
    T150_WEAPON_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
    {
        std::cout << "[T150_WEAPON] WsfPluginSetup() called!" << std::endl;
        if (!aApplication.ExtensionIsRegistered("t150_weapon_registration"))
        {
            std::cout << "[T150_WEAPON] Registering extension" << std::endl;
            aApplication.RegisterFeature("t150_weapon");
            aApplication.RegisterExtension(
                "t150_weapon_registration",
                std::make_unique<WsfDefaultApplicationExtension<t150::WeaponRegistration>>());
        }
        else
        {
            std::cout << "[T150_WEAPON] Extension already registered" << std::endl;
        }
    }
}
