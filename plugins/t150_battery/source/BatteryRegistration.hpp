// ****************************************************************************
// T-150 UAS Battery Processor Plugin - Scenario Registration
//
// This file registers the battery processor type with the scenario.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_BATTERY_REGISTRATION_HPP
#define T150_BATTERY_REGISTRATION_HPP

#include "BatteryProcessor.hpp"

#include <memory>
#include <iostream>

#include "UtInput.hpp"
#include "UtPlugin.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfProcessorTypes.hpp"
#include "WsfScenario.hpp"
#include "WsfScenarioExtension.hpp"
#include "WsfSimulation.hpp"

namespace t150
{

//! Scenario extension that registers the battery processor type
class BatteryRegistration : public WsfScenarioExtension
{
public:
    BatteryRegistration() = default;

    //! Called when the extension is added to the scenario
    void AddedToScenario() override
    {
        // Register battery processor as a core type (C++-defined)
        WsfScenario& scen = GetScenario();
        std::cout << "[T150_BATTERY] Registering processor type: "
                  << BatteryProcessor::GetBatteryProcessorClassId() << std::endl;
        WsfProcessorTypes::Get(scen).AddCoreType(
            BatteryProcessor::GetBatteryProcessorClassId(),
            std::make_unique<BatteryProcessor>(scen));
        std::cout << "[T150_BATTERY] Processor type registered successfully" << std::endl;
    }

    //! Called when a simulation is created
    void SimulationCreated(WsfSimulation& /*aSim*/) override
    {
        // No simulation-level extension needed for this processor
    }
};

} // namespace t150

#endif // T150_BATTERY_REGISTRATION_HPP
