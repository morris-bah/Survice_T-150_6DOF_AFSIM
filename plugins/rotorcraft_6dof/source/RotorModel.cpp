// ****************************************************************************
// Rotorcraft 6DOF Mover - Rotor Model Implementation
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "RotorModel.hpp"

namespace rotorcraft_6dof
{

// ============================================================================
// Update rotor state
// ============================================================================

void RotorModel::Update(double dt, double airDensity, double groundEffectFactor)
{
    // Sanity checks
    if (dt <= 0.0) return;
    if (airDensity <= 0.0) airDensity = AIR_DENSITY_SL;
    if (groundEffectFactor < 1.0) groundEffectFactor = 1.0;

    // ========================================================================
    // Motor dynamics - first order lag
    // ========================================================================
    // RPM_dot = (RPM_cmd - RPM) / tau
    // RPM_new = RPM + RPM_dot * dt

    if (mConfig.timeConstant > 1e-6)
    {
        double alpha = dt / (mConfig.timeConstant + dt);
        mState.actualRPM = mState.actualRPM + alpha * (mState.commandedRPM - mState.actualRPM);
    }
    else
    {
        // Instant response if time constant is zero
        mState.actualRPM = mState.commandedRPM;
    }

    // Clamp RPM
    mState.actualRPM = Clamp(mState.actualRPM, mConfig.minRPM, mConfig.maxRPM);

    // ========================================================================
    // Compute thrust using momentum theory
    // ========================================================================
    // T = CT * rho * n^2 * D^4
    // where n is in rev/sec

    mState.thrust = CalculateThrust(mState.actualRPM, mConfig.diameter,
                                     mConfig.thrustCoeff, airDensity);

    // Apply ground effect (increases thrust efficiency near ground)
    mState.thrust *= groundEffectFactor;

    // ========================================================================
    // Compute torque
    // ========================================================================
    // Q = CQ * rho * n^2 * D^5

    mState.torque = CalculateTorque(mState.actualRPM, mConfig.diameter,
                                     mConfig.torqueCoeff, airDensity);

    // ========================================================================
    // Compute induced velocity (for reference)
    // ========================================================================

    mState.inducedVelocity = CalculateInducedVelocity(mState.thrust,
                                                       mConfig.diskArea(),
                                                       airDensity);

    // ========================================================================
    // Compute power
    // ========================================================================

    mState.power = CalculatePower(mState.torque, mState.actualRPM);
}

// ============================================================================
// Get moment vector about CG
// ============================================================================

Vector3 RotorModel::GetMomentVector() const
{
    // Moment has two components:
    // 1. Thrust acting at rotor position creates moment about CG
    // 2. Reaction torque about rotor axis (opposite to rotation direction)

    // Moment from thrust at position
    // M = r x F
    Vector3 thrustForce = GetThrustVector();
    Vector3 momentFromThrust = mConfig.position.Cross(thrustForce);

    // Reaction torque (about Z axis in body frame)
    // CW rotation produces CCW reaction (positive yaw)
    // CCW rotation produces CW reaction (negative yaw)
    double reactionTorqueSign = (mConfig.direction == RotorDirection::CW) ? 1.0 : -1.0;
    Vector3 reactionTorque(0, 0, reactionTorqueSign * mState.torque);

    return momentFromThrust + reactionTorque;
}

// ============================================================================
// Static calculation functions
// ============================================================================

double RotorModel::CalculateThrust(double rpm, double diameter,
                                    double thrustCoeff, double airDensity)
{
    if (rpm <= 0.0) return 0.0;

    // Convert RPM to rev/sec
    double n = rpm / 60.0;

    // T = CT * rho * n^2 * D^4
    double D4 = diameter * diameter * diameter * diameter;
    double thrust = thrustCoeff * airDensity * n * n * D4;

    return thrust;
}

double RotorModel::CalculateTorque(double rpm, double diameter,
                                    double torqueCoeff, double airDensity)
{
    if (rpm <= 0.0) return 0.0;

    // Convert RPM to rev/sec
    double n = rpm / 60.0;

    // Q = CQ * rho * n^2 * D^5
    double D5 = diameter * diameter * diameter * diameter * diameter;
    double torque = torqueCoeff * airDensity * n * n * D5;

    return torque;
}

double RotorModel::CalculateInducedVelocity(double thrust, double diskArea,
                                             double airDensity)
{
    if (thrust <= 0.0 || diskArea <= 0.0 || airDensity <= 0.0)
        return 0.0;

    // v_i = sqrt(T / (2 * rho * A))
    return std::sqrt(thrust / (2.0 * airDensity * diskArea));
}

double RotorModel::CalculatePower(double torque, double rpm)
{
    if (rpm <= 0.0) return 0.0;

    // P = Q * omega = Q * (2 * pi * n)
    double omega = rpm * 2.0 * PI / 60.0;
    return torque * omega;
}

double RotorModel::CalculateRPMForThrust(double thrust, double diameter,
                                          double thrustCoeff, double airDensity)
{
    if (thrust <= 0.0 || diameter <= 0.0 || thrustCoeff <= 0.0 || airDensity <= 0.0)
        return 0.0;

    // From T = CT * rho * n^2 * D^4
    // n = sqrt(T / (CT * rho * D^4))
    double D4 = diameter * diameter * diameter * diameter;
    double nSquared = thrust / (thrustCoeff * airDensity * D4);

    if (nSquared <= 0.0) return 0.0;

    double n = std::sqrt(nSquared);  // rev/sec
    return n * 60.0;  // Convert to RPM
}

} // namespace rotorcraft_6dof
