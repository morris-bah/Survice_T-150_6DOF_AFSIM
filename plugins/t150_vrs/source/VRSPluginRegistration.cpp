// ****************************************************************************
// T-150 UAS VRS Processor Plugin - Registration
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "t150_vrs_export.h"
#include "VRSRegistration.hpp"

#include <memory>

#include "WsfPlugin.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"

extern "C"
{
    //! Plugin version check
    T150_VRS_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
    {
        aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                   WSF_PLUGIN_API_MINOR_VERSION,
                                   WSF_PLUGIN_API_SYSTEMABI_STRING);
    }

    //! Plugin setup
    T150_VRS_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
    {
        if (!aApplication.ExtensionIsRegistered("t150_vrs_registration"))
        {
            aApplication.RegisterFeature("t150_vrs");
            aApplication.RegisterExtension(
                "t150_vrs_registration",
                std::make_unique<WsfDefaultApplicationExtension<t150::VRSRegistration>>());
        }
    }
}
