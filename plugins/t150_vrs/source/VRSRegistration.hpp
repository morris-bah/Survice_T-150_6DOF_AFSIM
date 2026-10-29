// ****************************************************************************
// T-150 UAS VRS Processor Plugin - Scenario Registration
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_VRS_REGISTRATION_HPP
#define T150_VRS_REGISTRATION_HPP

#include "VRSProcessor.hpp"

#include <memory>

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

//! Scenario extension that registers the VRS processor type
class VRSRegistration : public WsfScenarioExtension
{
public:
    VRSRegistration() = default;

    //! Called when the extension is added to the scenario
    void AddedToScenario() override
    {
        // Create a prototype processor
        auto vrsProcessorPtr = std::make_unique<VRSProcessor>(GetScenario());

        // Register it as an available processor type
        GetScenario().GetProcessorTypes().Add(
            VRSProcessor::GetVRSProcessorClassId(),
            std::move(vrsProcessorPtr));
    }

    //! Called when a simulation is created
    void SimulationCreated(WsfSimulation& /*aSim*/) override
    {
        // No simulation-level extension needed
    }
};

} // namespace t150

#endif // T150_VRS_REGISTRATION_HPP
