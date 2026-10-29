// ****************************************************************************
// T-150 UAS Propulsion Processor Plugin - Scenario Registration
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_PROPULSION_REGISTRATION_HPP
#define T150_PROPULSION_REGISTRATION_HPP

#include "PropulsionProcessor.hpp"

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

//! Scenario extension that registers the propulsion processor type
class PropulsionRegistration : public WsfScenarioExtension
{
public:
    PropulsionRegistration() = default;

    void AddedToScenario() override
    {
        auto propProcessorPtr = std::make_unique<PropulsionProcessor>(GetScenario());
        GetScenario().GetProcessorTypes().Add(
            PropulsionProcessor::GetPropulsionProcessorClassId(),
            std::move(propProcessorPtr));
    }

    void SimulationCreated(WsfSimulation& /*aSim*/) override
    {
    }
};

} // namespace t150

#endif // T150_PROPULSION_REGISTRATION_HPP
