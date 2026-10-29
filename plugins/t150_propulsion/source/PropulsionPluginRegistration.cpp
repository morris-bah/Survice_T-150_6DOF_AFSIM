// ****************************************************************************
// T-150 UAS Propulsion Processor Plugin - Registration
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "t150_propulsion_export.h"
#include "PropulsionRegistration.hpp"

#include <memory>

#include "WsfPlugin.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"

extern "C"
{
    T150_PROPULSION_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
    {
        aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                   WSF_PLUGIN_API_MINOR_VERSION,
                                   WSF_PLUGIN_API_SYSTEMABI_STRING);
    }

    T150_PROPULSION_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
    {
        if (!aApplication.ExtensionIsRegistered("t150_propulsion_registration"))
        {
            aApplication.RegisterFeature("t150_propulsion");
            aApplication.RegisterExtension(
                "t150_propulsion_registration",
                std::make_unique<WsfDefaultApplicationExtension<t150::PropulsionRegistration>>());
        }
    }
}
