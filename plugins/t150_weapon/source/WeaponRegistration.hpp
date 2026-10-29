// ****************************************************************************
// T-150 UAS Weapon Processor Plugin - Scenario Registration
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_WEAPON_REGISTRATION_HPP
#define T150_WEAPON_REGISTRATION_HPP

#include "WeaponProcessor.hpp"

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

//! Scenario extension that registers the weapon processor type
class WeaponRegistration : public WsfScenarioExtension
{
public:
    WeaponRegistration() = default;

    void AddedToScenario() override
    {
        auto weaponProcessorPtr = std::make_unique<WeaponProcessor>(GetScenario());
        GetScenario().GetProcessorTypes().Add(
            WeaponProcessor::GetWeaponProcessorClassId(),
            std::move(weaponProcessorPtr));
    }

    void SimulationCreated(WsfSimulation& /*aSim*/) override
    {
    }
};

} // namespace t150

#endif // T150_WEAPON_REGISTRATION_HPP
