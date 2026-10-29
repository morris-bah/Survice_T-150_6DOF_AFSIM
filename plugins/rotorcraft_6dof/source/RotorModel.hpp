// ****************************************************************************
// Rotorcraft 6DOF Mover - Rotor Model
//
// Models a single rotor using momentum theory. Computes thrust, torque,
// and power as functions of RPM and air density.
//
// Supports:
// - Fixed-pitch propellers (multirotors)
// - CW and CCW rotation directions
// - Motor dynamics (first-order lag)
// - Ground effect enhancement
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef ROTORCRAFT_6DOF_ROTOR_MODEL_HPP
#define ROTORCRAFT_6DOF_ROTOR_MODEL_HPP

#include "MathUtils.hpp"

namespace rotorcraft_6dof
{

// ============================================================================
// Rotor direction enumeration
// ============================================================================

enum class RotorDirection
{
    CW,     // Clockwise (looking from above) - produces CCW yaw moment
    CCW     // Counter-clockwise - produces CW yaw moment
};

// ============================================================================
// Rotor configuration parameters
// ============================================================================

struct RotorConfig
{
    // Geometric properties
    double diameter = 0.254;        // [m] Propeller diameter (default 10")
    double radius() const { return diameter * 0.5; }
    double diskArea() const { return PI * radius() * radius(); }

    // Aerodynamic coefficients
    double thrustCoeff = 0.10;      // CT - thrust coefficient
    double torqueCoeff = 0.008;     // CQ - torque coefficient
    double powerCoeff = 0.05;       // CP - power coefficient (alternative)

    // Operating limits
    double maxRPM = 10000.0;        // Maximum motor RPM
    double minRPM = 0.0;            // Minimum motor RPM (usually 0 or idle)

    // Motor dynamics
    double timeConstant = 0.05;     // [s] First-order motor response time

    // Position in body frame (from CG)
    Vector3 position;               // [m] (x, y, z) in body frame

    // Rotation direction
    RotorDirection direction = RotorDirection::CCW;

    // Thrust direction (usually -Z in body frame for upward thrust)
    Vector3 thrustAxis = Vector3(0, 0, -1);

    // Control mixing factors (how this rotor responds to roll/pitch/yaw commands)
    double rollMix = 0.0;           // Roll mixing factor (-1 to +1)
    double pitchMix = 0.0;          // Pitch mixing factor (-1 to +1)
    double yawMix = 0.0;            // Yaw mixing factor (-1 to +1)
};

// ============================================================================
// Rotor state
// ============================================================================

struct RotorState
{
    double commandedRPM = 0.0;      // Commanded RPM from controller
    double actualRPM = 0.0;         // Actual RPM (with motor dynamics)
    double thrust = 0.0;            // [N] Current thrust output
    double torque = 0.0;            // [Nm] Reaction torque
    double power = 0.0;             // [W] Mechanical power
    double inducedVelocity = 0.0;   // [m/s] Induced velocity at disk

    void Reset()
    {
        commandedRPM = actualRPM = 0.0;
        thrust = torque = power = 0.0;
        inducedVelocity = 0.0;
    }
};

// ============================================================================
// RotorModel - Single rotor physics
// ============================================================================

class RotorModel
{
public:
    RotorModel() = default;
    explicit RotorModel(const RotorConfig& config) : mConfig(config) {}

    // ========================================================================
    // Configuration
    // ========================================================================

    void SetConfig(const RotorConfig& config) { mConfig = config; }
    const RotorConfig& GetConfig() const { return mConfig; }
    RotorConfig& GetConfig() { return mConfig; }

    // ========================================================================
    // State access
    // ========================================================================

    const RotorState& GetState() const { return mState; }
    RotorState& GetState() { return mState; }

    void Reset() { mState.Reset(); }

    // ========================================================================
    // Update
    // ========================================================================

    //! Set commanded RPM (from flight controller)
    void SetCommandedRPM(double rpm)
    {
        mState.commandedRPM = Clamp(rpm, mConfig.minRPM, mConfig.maxRPM);
    }

    //! Update rotor state for one timestep
    //! @param dt Timestep [s]
    //! @param airDensity Air density [kg/m^3]
    //! @param groundEffectFactor Thrust multiplier for ground effect (1.0 = none)
    void Update(double dt, double airDensity, double groundEffectFactor = 1.0);

    //! Get thrust vector in body frame
    Vector3 GetThrustVector() const
    {
        return mConfig.thrustAxis * mState.thrust;
    }

    //! Get moment vector about CG (body frame)
    //! Includes both thrust moment arm and reaction torque
    Vector3 GetMomentVector() const;

    // ========================================================================
    // Static calculations (for analysis)
    // ========================================================================

    //! Calculate thrust from RPM (no motor dynamics)
    //! T = CT * rho * n^2 * D^4
    static double CalculateThrust(double rpm, double diameter,
                                   double thrustCoeff, double airDensity);

    //! Calculate torque from RPM
    //! Q = CQ * rho * n^2 * D^5
    static double CalculateTorque(double rpm, double diameter,
                                   double torqueCoeff, double airDensity);

    //! Calculate induced velocity (hover)
    //! v_i = sqrt(T / (2 * rho * A))
    static double CalculateInducedVelocity(double thrust, double diskArea,
                                            double airDensity);

    //! Calculate power
    //! P = Q * omega = k * T * v_i (for hover)
    static double CalculatePower(double torque, double rpm);

    //! Calculate RPM required for given thrust
    static double CalculateRPMForThrust(double thrust, double diameter,
                                         double thrustCoeff, double airDensity);

private:
    RotorConfig mConfig;
    RotorState mState;
};

} // namespace rotorcraft_6dof

#endif // ROTORCRAFT_6DOF_ROTOR_MODEL_HPP
