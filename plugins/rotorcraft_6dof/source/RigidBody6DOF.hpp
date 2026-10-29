// ****************************************************************************
// Rotorcraft 6DOF Mover - Rigid Body 6DOF Integrator
//
// Implements Newton-Euler equations of motion for a rigid body with
// quaternion-based attitude representation.
//
// Reference frames:
// - World frame: NED (North-East-Down), origin at start point
// - Body frame: X-forward, Y-right, Z-down (aircraft convention)
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef ROTORCRAFT_6DOF_RIGID_BODY_HPP
#define ROTORCRAFT_6DOF_RIGID_BODY_HPP

#include "MathUtils.hpp"

namespace rotorcraft_6dof
{

// ============================================================================
// Rigid body state - complete kinematic state of the vehicle
// ============================================================================

struct RigidBodyState
{
    // Position (World frame - NED)
    Vector3 position;           // [m] North, East, Down from origin

    // Velocity (World frame - NED)
    Vector3 velocity;           // [m/s]

    // Attitude (Body to World rotation)
    Quaternion orientation;     // Unit quaternion

    // Angular velocity (Body frame)
    Vector3 angularVelocity;    // [rad/s] roll rate, pitch rate, yaw rate

    // Derived quantities (computed from state)
    double roll = 0.0;          // [rad] Euler angle
    double pitch = 0.0;         // [rad] Euler angle
    double yaw = 0.0;           // [rad] Euler angle (heading)

    // Initialize to default (on ground, level, no motion)
    void Reset()
    {
        position = Vector3::Zero();
        velocity = Vector3::Zero();
        orientation = Quaternion::Identity();
        angularVelocity = Vector3::Zero();
        roll = pitch = yaw = 0.0;
    }

    // Update derived Euler angles from quaternion
    void UpdateEulerAngles()
    {
        orientation.ToEuler(roll, pitch, yaw);
    }

    // Get velocity in body frame
    Vector3 GetBodyVelocity() const
    {
        return orientation.RotateVectorInverse(velocity);
    }

    // Get airspeed (magnitude of velocity)
    double GetAirspeed() const
    {
        return velocity.Magnitude();
    }

    // Get ground speed (horizontal velocity magnitude)
    double GetGroundSpeed() const
    {
        return std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    }

    // Get climb rate (positive = ascending)
    double GetClimbRate() const
    {
        return -velocity.z;  // NED: positive z is down
    }

    // Get altitude (positive MSL)
    double GetAltitudeMSL() const
    {
        return -position.z;  // NED: positive z is down
    }
};

// ============================================================================
// Mass properties of the rigid body
// ============================================================================

struct MassProperties
{
    double mass = 1.0;              // [kg] Total mass

    // Principal moments of inertia (body frame, diagonal tensor assumed)
    double Ixx = 1.0;               // [kg*m^2] Roll inertia
    double Iyy = 1.0;               // [kg*m^2] Pitch inertia
    double Izz = 1.0;               // [kg*m^2] Yaw inertia

    // Center of gravity offset from geometric center (body frame)
    Vector3 cgOffset;               // [m]

    // Get inertia tensor (diagonal)
    Matrix3x3 GetInertiaTensor() const
    {
        return Matrix3x3::Diagonal(Ixx, Iyy, Izz);
    }

    // Get inverse inertia tensor (diagonal)
    Matrix3x3 GetInverseInertiaTensor() const
    {
        return Matrix3x3::Diagonal(
            (Ixx > 1e-10) ? 1.0 / Ixx : 0.0,
            (Iyy > 1e-10) ? 1.0 / Iyy : 0.0,
            (Izz > 1e-10) ? 1.0 / Izz : 0.0
        );
    }
};

// ============================================================================
// RigidBody6DOF - Main integrator class
// ============================================================================

class RigidBody6DOF
{
public:
    RigidBody6DOF() = default;

    // ========================================================================
    // Configuration
    // ========================================================================

    //! Set mass properties
    void SetMassProperties(const MassProperties& props) { mMassProps = props; }

    //! Get mass properties
    const MassProperties& GetMassProperties() const { return mMassProps; }
    MassProperties& GetMassProperties() { return mMassProps; }

    //! Set integration timestep
    void SetTimestep(double dt) { mTimestep = dt; }

    // ========================================================================
    // State access
    // ========================================================================

    //! Get current state
    const RigidBodyState& GetState() const { return mState; }
    RigidBodyState& GetState() { return mState; }

    //! Set state directly (for initialization)
    void SetState(const RigidBodyState& state) { mState = state; }

    //! Reset to initial conditions
    void Reset()
    {
        mState.Reset();
        mLastForce = Vector3::Zero();
        mLastMoment = Vector3::Zero();
    }

    // ========================================================================
    // Integration
    // ========================================================================

    //! Integrate one timestep given total forces and moments
    //! @param totalForce Total force in body frame [N]
    //! @param totalMoment Total moment about CG in body frame [Nm]
    //! @param dt Timestep [s]
    void Integrate(const Vector3& totalForce, const Vector3& totalMoment, double dt);

    //! Get last applied force (for debugging)
    const Vector3& GetLastForce() const { return mLastForce; }

    //! Get last applied moment (for debugging)
    const Vector3& GetLastMoment() const { return mLastMoment; }

    //! Get last computed acceleration (body frame)
    const Vector3& GetLastAcceleration() const { return mLastAccel; }

    //! Get last computed angular acceleration (body frame)
    const Vector3& GetLastAngularAcceleration() const { return mLastAngularAccel; }

private:
    //! Compute linear acceleration from forces (body frame)
    Vector3 ComputeLinearAcceleration(const Vector3& forceBody) const;

    //! Compute angular acceleration from moments (body frame)
    //! Implements: I * omega_dot + omega x (I * omega) = M
    Vector3 ComputeAngularAcceleration(const Vector3& momentBody) const;

    // State
    RigidBodyState mState;
    MassProperties mMassProps;

    // Integration parameters
    double mTimestep = 0.01;  // 100 Hz default

    // Debug/output values
    Vector3 mLastForce;
    Vector3 mLastMoment;
    Vector3 mLastAccel;
    Vector3 mLastAngularAccel;
};

} // namespace rotorcraft_6dof

#endif // ROTORCRAFT_6DOF_RIGID_BODY_HPP
