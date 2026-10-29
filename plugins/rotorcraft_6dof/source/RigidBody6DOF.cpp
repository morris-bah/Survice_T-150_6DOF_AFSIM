// ****************************************************************************
// Rotorcraft 6DOF Mover - Rigid Body 6DOF Integrator Implementation
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "RigidBody6DOF.hpp"

namespace rotorcraft_6dof
{

// ============================================================================
// Integration
// ============================================================================

void RigidBody6DOF::Integrate(const Vector3& totalForce, const Vector3& totalMoment, double dt)
{
    // Store for debugging
    mLastForce = totalForce;
    mLastMoment = totalMoment;

    // Sanity check
    if (dt <= 0.0) return;
    if (mMassProps.mass <= 0.0) return;

    // ========================================================================
    // Step 1: Compute accelerations in body frame
    // ========================================================================

    // Linear acceleration: a = F/m (body frame)
    mLastAccel = ComputeLinearAcceleration(totalForce);

    // Angular acceleration: Euler equations
    mLastAngularAccel = ComputeAngularAcceleration(totalMoment);

    // ========================================================================
    // Step 2: Transform body acceleration to world frame and add gravity
    // ========================================================================

    // Rotate body-frame acceleration to world frame
    Vector3 accelWorld = mState.orientation.RotateVector(mLastAccel);

    // Add gravity (always acts downward in NED)
    accelWorld.z += GRAVITY;

    // ========================================================================
    // Step 3: Integrate translational state (Semi-implicit Euler)
    // ========================================================================

    // Update velocity first (semi-implicit for better stability)
    mState.velocity += accelWorld * dt;

    // Then update position
    mState.position += mState.velocity * dt;

    // ========================================================================
    // Step 4: Integrate rotational state
    // ========================================================================

    // Update angular velocity (body frame)
    mState.angularVelocity += mLastAngularAccel * dt;

    // Update quaternion using angular velocity
    // q_dot = 0.5 * q * [0, omega]
    Quaternion omegaQuat(0, mState.angularVelocity.x,
                            mState.angularVelocity.y,
                            mState.angularVelocity.z);
    Quaternion qDot = mState.orientation * omegaQuat * 0.5;

    mState.orientation.w += qDot.w * dt;
    mState.orientation.x += qDot.x * dt;
    mState.orientation.y += qDot.y * dt;
    mState.orientation.z += qDot.z * dt;

    // Normalize quaternion to prevent drift
    mState.orientation = mState.orientation.Normalized();

    // ========================================================================
    // Step 5: Update derived quantities
    // ========================================================================

    mState.UpdateEulerAngles();
}

// ============================================================================
// Compute linear acceleration from forces
// ============================================================================

Vector3 RigidBody6DOF::ComputeLinearAcceleration(const Vector3& forceBody) const
{
    // Newton's second law: F = m * a  =>  a = F / m
    return forceBody / mMassProps.mass;
}

// ============================================================================
// Compute angular acceleration from moments
// ============================================================================

Vector3 RigidBody6DOF::ComputeAngularAcceleration(const Vector3& momentBody) const
{
    // Euler's equations for a rigid body:
    // I * omega_dot + omega x (I * omega) = M
    //
    // Solving for omega_dot:
    // omega_dot = I^(-1) * (M - omega x (I * omega))
    //
    // For diagonal inertia tensor I = diag(Ixx, Iyy, Izz):
    // omega_dot_x = (M_x - (Izz - Iyy) * omega_y * omega_z) / Ixx
    // omega_dot_y = (M_y - (Ixx - Izz) * omega_z * omega_x) / Iyy
    // omega_dot_z = (M_z - (Iyy - Ixx) * omega_x * omega_y) / Izz

    const Vector3& omega = mState.angularVelocity;

    // Gyroscopic coupling terms
    double gyroX = (mMassProps.Izz - mMassProps.Iyy) * omega.y * omega.z;
    double gyroY = (mMassProps.Ixx - mMassProps.Izz) * omega.z * omega.x;
    double gyroZ = (mMassProps.Iyy - mMassProps.Ixx) * omega.x * omega.y;

    // Angular acceleration
    Vector3 omegaDot;

    if (mMassProps.Ixx > 1e-10)
        omegaDot.x = (momentBody.x - gyroX) / mMassProps.Ixx;
    else
        omegaDot.x = 0.0;

    if (mMassProps.Iyy > 1e-10)
        omegaDot.y = (momentBody.y - gyroY) / mMassProps.Iyy;
    else
        omegaDot.y = 0.0;

    if (mMassProps.Izz > 1e-10)
        omegaDot.z = (momentBody.z - gyroZ) / mMassProps.Izz;
    else
        omegaDot.z = 0.0;

    return omegaDot;
}

} // namespace rotorcraft_6dof
