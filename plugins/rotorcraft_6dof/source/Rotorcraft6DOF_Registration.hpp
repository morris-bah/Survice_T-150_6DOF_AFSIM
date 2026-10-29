// ****************************************************************************
// Rotorcraft 6DOF Mover - Scenario Extension Registration
//
// Registers the WSF_ROTORCRAFT_6DOF_MOVER type with AFSIM.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef ROTORCRAFT_6DOF_REGISTRATION_HPP
#define ROTORCRAFT_6DOF_REGISTRATION_HPP

#include "Rotorcraft6DOF_Mover.hpp"

#include <memory>
#include <iostream>

#include "UtInput.hpp"
#include "UtPlugin.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfScenario.hpp"
#include "WsfScenarioExtension.hpp"
#include "WsfSimulation.hpp"
#include "WsfMoverTypes.hpp"

namespace rotorcraft_6dof
{

//! Scenario extension that registers the 6DOF mover type
class Rotorcraft6DOF_Registration : public WsfScenarioExtension
{
public:
    Rotorcraft6DOF_Registration() = default;

    void AddedToScenario() override
    {
        std::cout << "[ROTORCRAFT_6DOF] AddedToScenario() called!" << std::endl;
        // Register our mover type with AFSIM using AddCoreType
        // This marks it as a core type defined by C++ code
        WsfScenario& scen = GetScenario();
        std::cout << "[ROTORCRAFT_6DOF] Registering mover type WSF_ROTORCRAFT_6DOF_MOVER" << std::endl;
        WsfMoverTypes::Get(scen).AddCoreType(
            "WSF_ROTORCRAFT_6DOF_MOVER",
            std::make_unique<Rotorcraft6DOF_Mover>(scen));
        std::cout << "[ROTORCRAFT_6DOF] Mover type registered successfully" << std::endl;
    }

    void SimulationCreated(WsfSimulation& /*aSim*/) override
    {
    }
};

} // namespace rotorcraft_6dof

#endif // ROTORCRAFT_6DOF_REGISTRATION_HPP
