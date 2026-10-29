// ****************************************************************************
// T-150 UAS Slung Load Processor Plugin - Scenario Registration
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_SLUNG_LOAD_REGISTRATION_HPP
#define T150_SLUNG_LOAD_REGISTRATION_HPP

#include "SlungLoadProcessor.hpp"

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

//! Scenario extension that registers the slung load processor type
class SlungLoadRegistration : public WsfScenarioExtension
{
public:
    SlungLoadRegistration() = default;

    void AddedToScenario() override
    {
        auto slungLoadProcessorPtr = std::make_unique<SlungLoadProcessor>(GetScenario());
        GetScenario().GetProcessorTypes().Add(
            SlungLoadProcessor::GetSlungLoadProcessorClassId(),
            std::move(slungLoadProcessorPtr));
    }

    void SimulationCreated(WsfSimulation& /*aSim*/) override
    {
    }
};

} // namespace t150

#endif // T150_SLUNG_LOAD_REGISTRATION_HPP
