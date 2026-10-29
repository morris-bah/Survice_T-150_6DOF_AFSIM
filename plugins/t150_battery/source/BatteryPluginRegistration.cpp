// ****************************************************************************
// T-150 UAS Battery Processor Plugin - Registration
//
// This file registers the battery processor plugin with AFSIM.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "t150_battery_processor_export.h"
#include "BatteryRegistration.hpp"

#include <memory>
#include <iostream>
#include <string>
#include <cstdlib>

#include "WsfPlugin.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"

namespace
{
    const std::string sFeatureName = "t150_battery";
    const std::string sExtensionName = "t150_battery_registration";

    // Global constructor to verify plugin .so is actually loaded
    struct PluginLoadVerifier
    {
        PluginLoadVerifier()
        {
            std::cout << "[T150_BATTERY] ===== PLUGIN .SO FILE LOADED (global constructor) =====" << std::endl;
        }
    };
    static PluginLoadVerifier sVerifier;

    void PrintNgicPluginBanner()
    {
        // Use environment variable to ensure banner only prints once across all NGIC plugins
        if (std::getenv("NGIC_BANNER_PRINTED") != nullptr) return;
#ifdef _WIN32
        const int environmentResult = _putenv_s("NGIC_BANNER_PRINTED", "1");
#else
        const int environmentResult = setenv("NGIC_BANNER_PRINTED", "1", 1);
#endif
        if (environmentResult != 0)
        {
            std::cerr << "[T150_BATTERY] Failed to set NGIC_BANNER_PRINTED" << std::endl;
        }

        const std::string border = "==================================================================================";
        const std::string cui_header = "CUI // CONTROLLED UNCLASSIFIED INFORMATION";

        const std::string dev_name = "Daniel S. Garrett";
        const std::string org_name = "US Army National Ground Intelligence Center (NGIC)";
        const std::string version  = "v1.0.0";

        std::cout << "\n" << border << "\n";
        std::cout << "                " << cui_header << "\n";
        std::cout << border << "\n\n";

        std::cout << "  AFSIM PLUGIN LOADED: " << version << "\n";
        std::cout << "  DEVELOPER          : " << dev_name << "\n";
        std::cout << "  ORGANIZATION       : " << org_name << "\n\n";

        std::cout << "  ------------------------------------------------------------------------------\n";
        std::cout << "  [ CUI DESIGNATION INDICATOR ]\n";
        std::cout << "  Controlled by: " << org_name << "\n";
        std::cout << "  CUI Category : [INSERT CATEGORY, e.g., CTI (Controlled Technical Info)]\n";
        std::cout << "  Distribution : [INSERT DISTRIBUTION STATEMENT, e.g., Distro C, D, or F]\n";
        std::cout << "  POC          : " << dev_name << "\n";
        std::cout << "  ------------------------------------------------------------------------------\n\n";

        std::cout << "  WARNING: This software contains Controlled Unclassified Information (CUI).\n";
        std::cout << "  Access is limited to authorized personnel with a lawful government purpose.\n";
        std::cout << "  Do not release to the public or unauthorized foreign nationals.\n\n";

        std::cout << border << "\n";
        std::cout << "                " << cui_header << "\n";
        std::cout << border << "\n" << std::endl;
    }

    //! Application extension that registers the scenario extension
    class ApplicationExtension : public WsfApplicationExtension
    {
    public:
        void ScenarioCreated(WsfScenario& aScenario) override
        {
            std::cout << "[T150_BATTERY] ApplicationExtension::ScenarioCreated() called" << std::endl;
            aScenario.RegisterExtension(sExtensionName, std::make_unique<t150::BatteryRegistration>());
        }
    };
}

extern "C"
{
    //! Plugin version check
    T150_BATTERY_PROCESSOR_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
    {
        std::cout << "[T150_BATTERY] ===== WsfPluginVersion() CALLED =====" << std::endl;
        std::cout << "[T150_BATTERY] Major: " << WSF_PLUGIN_API_MAJOR_VERSION << std::endl;
        std::cout << "[T150_BATTERY] Minor: " << WSF_PLUGIN_API_MINOR_VERSION << std::endl;
        std::cout << "[T150_BATTERY] ABI: " << WSF_PLUGIN_API_SYSTEMABI_STRING << std::endl;

        aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                   WSF_PLUGIN_API_MINOR_VERSION,
                                   WSF_PLUGIN_API_SYSTEMABI_STRING);

        std::cout << "[T150_BATTERY] Version check completed" << std::endl;
    }

    //! Plugin setup - register the extension with the application
    T150_BATTERY_PROCESSOR_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
    {
        try
        {
            std::cout << "[T150_BATTERY] ===== WsfPluginSetup() ENTRY POINT =====" << std::endl;
            std::cout << "[T150_BATTERY] Extension name: " << sExtensionName << std::endl;
            std::cout << "[T150_BATTERY] Feature name: " << sFeatureName << std::endl;

            if (!aApplication.ExtensionIsRegistered(sExtensionName))
            {
                std::cout << "[T150_BATTERY] Extension not registered, proceeding with registration" << std::endl;
                PrintNgicPluginBanner();
                std::cout << "[T150_BATTERY] Banner printed, registering feature" << std::endl;

                // Register the feature so scenarios can check for it
                aApplication.RegisterFeature(sFeatureName, sExtensionName);
                std::cout << "[T150_BATTERY] Feature registered, creating application extension" << std::endl;

                // Register our custom application extension
                aApplication.RegisterExtension(sExtensionName, std::make_unique<ApplicationExtension>());
                std::cout << "[T150_BATTERY] Application extension registered successfully" << std::endl;
            }
            else
            {
                std::cout << "[T150_BATTERY] Extension already registered" << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[T150_BATTERY] EXCEPTION in WsfPluginSetup: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "[T150_BATTERY] UNKNOWN EXCEPTION in WsfPluginSetup" << std::endl;
        }
    }
}
