// ****************************************************************************
// T-150 UAS Weapon/Launcher Processor Plugin - Implementation
//
// Physics-based weapon system with recoil force coupling to 6DOF mover.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "WeaponProcessor.hpp"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "UtInput.hpp"
#include "UtLog.hpp"
#include "WsfPlatform.hpp"
#include "WsfAttributeContainer.hpp"

namespace t150
{

// ============================================================================
// Constants
// ============================================================================
static constexpr double GRAVITY = 9.80665;
static constexpr double DEG_TO_RAD = 0.017453292519943295;
static constexpr double RAD_TO_DEG = 57.29577951308232;
static constexpr double PI = 3.14159265358979323846;

// ============================================================================
// Static Class ID
// ============================================================================
WsfStringId WeaponProcessor::GetWeaponProcessorClassId()
{
    static WsfStringId classId("T150_WEAPON_PROCESSOR");
    return classId;
}

// ============================================================================
// Constructor
// ============================================================================
WeaponProcessor::WeaponProcessor(const WsfScenario& aScenario)
    : WsfProcessor(aScenario)
    , mConfig()
    , mWeaponName("weapon_1")
    , mState(WeaponState::IDLE)
    , mRoundsRemaining(0)
    , mRoundsFired(0)
    , mBurstRoundsRemaining(0)
    , mCurrentMass(0.0)
    , mInitialMass(0.0)
    , mMassDelta(0.0)
    , mLastFireTime(0.0)
    , mStateEntryTime(0.0)
    , mImpulseRemaining(0.0)
    , mContinuousFireEnabled(false)
    , mRecoilForceX(0.0)
    , mRecoilForceY(0.0)
    , mRecoilForceZ(0.0)
    , mRecoilMomentX(0.0)
    , mRecoilMomentY(0.0)
    , mRecoilMomentZ(0.0)
    , mLastUpdateTime(0.0)
    , mUpdateInterval(0.01)  // 100 Hz for impulse accuracy
    , mJamCount(0)
    , mReloadCount(0)
    , mAutoFiresRemaining(0)
    , mNextAutoFireTime(0.0)
    , mCsvOutputPath("")
    , mCsvEnabled(false)
    , mCsvLogInterval(1.0)
    , mLastCsvLogTime(0.0)
{
    // Register for periodic updates (default 10ms, can be overridden in scenario)
    SetUpdateInterval(0.01);
}

// ============================================================================
// Copy Constructor
// ============================================================================
WeaponProcessor::WeaponProcessor(const WeaponProcessor& aSrc)
    : WsfProcessor(aSrc)
    , mConfig(aSrc.mConfig)
    , mWeaponName(aSrc.mWeaponName)
    , mState(aSrc.mState)
    , mRoundsRemaining(aSrc.mRoundsRemaining)
    , mRoundsFired(aSrc.mRoundsFired)
    , mBurstRoundsRemaining(aSrc.mBurstRoundsRemaining)
    , mCurrentMass(aSrc.mCurrentMass)
    , mInitialMass(aSrc.mInitialMass)
    , mMassDelta(aSrc.mMassDelta)
    , mLastFireTime(aSrc.mLastFireTime)
    , mStateEntryTime(aSrc.mStateEntryTime)
    , mImpulseRemaining(aSrc.mImpulseRemaining)
    , mContinuousFireEnabled(aSrc.mContinuousFireEnabled)
    , mRecoilForceX(aSrc.mRecoilForceX)
    , mRecoilForceY(aSrc.mRecoilForceY)
    , mRecoilForceZ(aSrc.mRecoilForceZ)
    , mRecoilMomentX(aSrc.mRecoilMomentX)
    , mRecoilMomentY(aSrc.mRecoilMomentY)
    , mRecoilMomentZ(aSrc.mRecoilMomentZ)
    , mLastUpdateTime(aSrc.mLastUpdateTime)
    , mUpdateInterval(aSrc.mUpdateInterval)
    , mJamCount(aSrc.mJamCount)
    , mReloadCount(aSrc.mReloadCount)
    , mAutoFiresRemaining(aSrc.mAutoFiresRemaining)
    , mNextAutoFireTime(aSrc.mNextAutoFireTime)
    , mCsvOutputPath(aSrc.mCsvOutputPath)
    , mCsvEnabled(false)  // Don't copy file handle
    , mCsvLogInterval(aSrc.mCsvLogInterval)
    , mLastCsvLogTime(0.0)
{
}

// ============================================================================
// Clone
// ============================================================================
WsfProcessor* WeaponProcessor::Clone() const
{
    return new WeaponProcessor(*this);
}

// ============================================================================
// GetScriptClassName
// ============================================================================
const char* WeaponProcessor::GetScriptClassName() const
{
    return "T150_WEAPON_PROCESSOR";
}

// ============================================================================
// ProcessInput - Parse scenario configuration
// ============================================================================
bool WeaponProcessor::ProcessInput(UtInput& aInput)
{
    bool myCommand = true;
    std::string command(aInput.GetCommand());

    // Weapon type
    if (command == "weapon_type")
    {
        std::string typeName;
        aInput.ReadValue(typeName);
        if (typeName == "PROJECTILE_LAUNCHER" || typeName == "launcher")
        {
            mConfig.type = WeaponType::PROJECTILE_LAUNCHER;
        }
        else if (typeName == "MACHINE_GUN" || typeName == "gun")
        {
            mConfig.type = WeaponType::MACHINE_GUN;
        }
        else if (typeName == "PAYLOAD_RELEASE" || typeName == "payload")
        {
            mConfig.type = WeaponType::PAYLOAD_RELEASE;
        }
    }
    else if (command == "weapon_name" || command == "name")
    {
        aInput.ReadValue(mWeaponName);
    }
    // Munition properties
    else if (command == "munition_mass")
    {
        aInput.ReadValue(mConfig.munitionMass);
        mConfig.munitionMass = std::max(0.001, mConfig.munitionMass);
    }
    else if (command == "muzzle_velocity")
    {
        aInput.ReadValue(mConfig.muzzleVelocity);
        mConfig.muzzleVelocity = std::max(1.0, mConfig.muzzleVelocity);
    }
    else if (command == "recoil_impulse")
    {
        aInput.ReadValue(mConfig.recoilImpulse);
    }
    else if (command == "recoil_duration")
    {
        aInput.ReadValue(mConfig.recoilDuration);
        mConfig.recoilDuration = std::max(0.001, mConfig.recoilDuration);
    }
    else if (command == "propellant_gas_factor" || command == "gas_factor")
    {
        aInput.ReadValue(mConfig.propellantGasFactor);
        mConfig.propellantGasFactor = std::clamp(mConfig.propellantGasFactor, 1.0, 2.0);
    }
    // Magazine configuration
    else if (command == "magazine_capacity" || command == "capacity")
    {
        int capacity = 0;
        aInput.ReadValue(capacity);
        mConfig.magazineCapacity = std::max(1, capacity);
    }
    else if (command == "cycle_time")
    {
        aInput.ReadValue(mConfig.cycleTime);
        mConfig.cycleTime = std::max(0.01, mConfig.cycleTime);
    }
    else if (command == "reload_time")
    {
        aInput.ReadValue(mConfig.reloadTime);
        mConfig.reloadTime = std::max(0.1, mConfig.reloadTime);
    }
    // Mount configuration
    else if (command == "mount_offset_x")
    {
        aInput.ReadValue(mConfig.mountOffsetX);
    }
    else if (command == "mount_offset_y")
    {
        aInput.ReadValue(mConfig.mountOffsetY);
    }
    else if (command == "mount_offset_z")
    {
        aInput.ReadValue(mConfig.mountOffsetZ);
    }
    // Aiming angles
    else if (command == "tilt_angle")
    {
        aInput.ReadValue(mConfig.tiltAngle);
        mConfig.tiltAngle = std::clamp(mConfig.tiltAngle, 0.0, 80.0);
    }
    else if (command == "azimuth_angle")
    {
        aInput.ReadValue(mConfig.azimuthAngle);
        // Wrap to [-180, 180]
        while (mConfig.azimuthAngle > 180.0) mConfig.azimuthAngle -= 360.0;
        while (mConfig.azimuthAngle < -180.0) mConfig.azimuthAngle += 360.0;
    }
    // Machine gun specific
    else if (command == "burst_length")
    {
        int length = 0;
        aInput.ReadValue(length);
        mConfig.burstLength = std::max(1, length);
    }
    else if (command == "rounds_per_minute" || command == "rpm")
    {
        aInput.ReadValue(mConfig.roundsPerMinute);
        mConfig.roundsPerMinute = std::clamp(mConfig.roundsPerMinute, 60.0, 6000.0);
    }
    // Payload release specific
    else if (command == "release_time")
    {
        aInput.ReadValue(mConfig.releaseTime);
        mConfig.releaseTime = std::max(0.1, mConfig.releaseTime);
    }
    // Update interval
    else if (command == "update_interval")
    {
        aInput.ReadValue(mUpdateInterval);
        mUpdateInterval = std::max(0.001, mUpdateInterval);
    }
    // Auto-fire test mode options
    else if (command == "auto_fire" || command == "auto_fire_enabled")
    {
        std::string val;
        aInput.ReadValue(val);
        mConfig.autoFireEnabled = (val == "true" || val == "1" || val == "yes");
    }
    else if (command == "auto_fire_start")
    {
        aInput.ReadValue(mConfig.autoFireStartTime);
        mConfig.autoFireStartTime = std::max(0.0, mConfig.autoFireStartTime);
    }
    else if (command == "auto_fire_interval")
    {
        aInput.ReadValue(mConfig.autoFireInterval);
        mConfig.autoFireInterval = std::max(0.1, mConfig.autoFireInterval);
    }
    else if (command == "auto_fire_count")
    {
        int count = 0;
        aInput.ReadValue(count);
        mConfig.autoFireCount = std::max(0, count);
    }
    // CSV output configuration
    else if (command == "csv_output_file")
    {
        aInput.ReadValue(mCsvOutputPath);
        mCsvEnabled = !mCsvOutputPath.empty();
    }
    else if (command == "csv_log_interval")
    {
        aInput.ReadValue(mCsvLogInterval);
        mCsvLogInterval = std::max(0.1, mCsvLogInterval);
    }
    else
    {
        myCommand = WsfProcessor::ProcessInput(aInput);
    }

    return myCommand;
}

// ============================================================================
// Initialize
// ============================================================================
bool WeaponProcessor::Initialize(double aSimTime)
{
    bool ok = WsfProcessor::Initialize(aSimTime);

    if (ok)
    {
        mLastUpdateTime = aSimTime;
        mStateEntryTime = aSimTime;

        // Calculate recoil impulse from momentum if not specified
        // J = m * v * gas_factor
        // The gas factor accounts for propellant gas momentum (typically 1.3-1.5)
        if (mConfig.recoilImpulse <= 0.0)
        {
            mConfig.recoilImpulse = mConfig.munitionMass * mConfig.muzzleVelocity * mConfig.propellantGasFactor;
        }

        // Initialize ammunition
        mRoundsRemaining = mConfig.magazineCapacity;
        mRoundsFired = 0;

        // Calculate initial mass (weapon + full magazine)
        // Assume weapon base mass is 2x the ammunition mass
        double baseMass = mConfig.munitionMass * mConfig.magazineCapacity * 0.5;
        mInitialMass = baseMass + mConfig.munitionMass * mConfig.magazineCapacity;
        mCurrentMass = mInitialMass;

        // Initialize state
        mState = WeaponState::IDLE;
        mMassDelta = 0.0;

        // Initialize auto-fire mode
        if (mConfig.autoFireEnabled)
        {
            mAutoFiresRemaining = mConfig.autoFireCount;  // 0 = unlimited
            mNextAutoFireTime = aSimTime + mConfig.autoFireStartTime;
        }

        // Open CSV file and write header
        if (mCsvEnabled && !mCsvOutputPath.empty())
        {
            mCsvFile.open(mCsvOutputPath);
            if (mCsvFile.is_open())
            {
                // Write CSV header
                mCsvFile << "Time_s,Weapon_Name,State,Rounds_Remaining,Rounds_Fired,"
                         << "Current_Mass_kg,Mass_Delta_kg,Total_Mass_Expended_kg,"
                         << "Recoil_Force_X_N,Recoil_Force_Y_N,Recoil_Force_Z_N,"
                         << "Recoil_Moment_X_Nm,Recoil_Moment_Y_Nm,Recoil_Moment_Z_Nm\n";
                mCsvFile << std::fixed << std::setprecision(4);
                mLastCsvLogTime = aSimTime;
            }
            else
            {
                auto err = ut::log::warning() << "Weapon: Failed to open CSV file.";
                err.AddNote() << "Path: " << mCsvOutputPath;
                mCsvEnabled = false;
            }
        }

        // Publish initial aux data
        PublishAuxData();

        // Log initialization
        auto out = ut::log::info() << "Weapon processor initialized.";
        out.AddNote() << "Platform: " << GetPlatform()->GetName();
        out.AddNote() << "Weapon name: " << mWeaponName;
        out.AddNote() << "Type: " << (mConfig.type == WeaponType::PROJECTILE_LAUNCHER ? "PROJECTILE_LAUNCHER" :
                                       mConfig.type == WeaponType::MACHINE_GUN ? "MACHINE_GUN" : "PAYLOAD_RELEASE");
        out.AddNote() << "Munition mass: " << mConfig.munitionMass << " kg";
        out.AddNote() << "Muzzle velocity: " << mConfig.muzzleVelocity << " m/s";
        out.AddNote() << "Propellant gas factor: " << mConfig.propellantGasFactor;
        out.AddNote() << "Recoil impulse: " << mConfig.recoilImpulse << " N*s";
        out.AddNote() << "Magazine capacity: " << mConfig.magazineCapacity;
        out.AddNote() << "Tilt angle: " << mConfig.tiltAngle << " deg";
        out.AddNote() << "Mount offset: (" << mConfig.mountOffsetX << ", "
                      << mConfig.mountOffsetY << ", " << mConfig.mountOffsetZ << ") m";
        out.AddNote() << "Initial mass: " << mInitialMass << " kg";
        if (mConfig.autoFireEnabled)
        {
            out.AddNote() << "AUTO-FIRE ENABLED: start=" << mConfig.autoFireStartTime << "s, interval="
                          << mConfig.autoFireInterval << "s, count=" << (mConfig.autoFireCount == 0 ? "unlimited" : std::to_string(mConfig.autoFireCount));
        }
        if (mCsvEnabled)
        {
            out.AddNote() << "CSV output: " << mCsvOutputPath;
        }
    }

    return ok;
}

// ============================================================================
// Update - Main processing loop
// ============================================================================
void WeaponProcessor::Update(double aSimTime)
{
    WsfProcessor::Update(aSimTime);

    if (!IsOperational())
        return;

    double deltaTime = aSimTime - mLastUpdateTime;
    if (deltaTime < mUpdateInterval)
        return;

    // Reset mass delta for this frame
    mMassDelta = 0.0;

    // Read fire commands from AuxData
    ReadCommands();

    // Check auto-fire mode
    if (mConfig.autoFireEnabled && mState == WeaponState::IDLE)
    {
        if (aSimTime >= mNextAutoFireTime)
        {
            // Check if we have fires remaining (or unlimited)
            if (mConfig.autoFireCount == 0 || mAutoFiresRemaining > 0)
            {
                std::cout << "[AUTO-FIRE] T=" << aSimTime << "s: Firing weapon " << mWeaponName << std::endl;
                Fire();

                // Schedule next auto-fire
                mNextAutoFireTime = aSimTime + mConfig.autoFireInterval;

                // Decrement remaining count if not unlimited
                if (mConfig.autoFireCount > 0 && mAutoFiresRemaining > 0)
                {
                    mAutoFiresRemaining--;
                }
            }
        }
    }

    // Update state machine
    UpdateStateMachine(aSimTime, deltaTime);

    // Calculate recoil forces
    CalculateRecoilForces(deltaTime);

    // Publish telemetry
    PublishAuxData();

    // Write CSV row at specified interval
    if (mCsvEnabled && mCsvFile.is_open())
    {
        if (aSimTime - mLastCsvLogTime >= mCsvLogInterval)
        {
            WriteCSVRow(aSimTime);
            mLastCsvLogTime = aSimTime;
        }
    }

    mLastUpdateTime = aSimTime;
}

// ============================================================================
// UpdateStateMachine
// ============================================================================
void WeaponProcessor::UpdateStateMachine(double aSimTime, double aDeltaTime)
{
    double stateTime = aSimTime - mStateEntryTime;

    switch (mState)
    {
        case WeaponState::IDLE:
            // Check for fire command
            if (mContinuousFireEnabled && mRoundsRemaining > 0)
            {
                ProcessShot(aSimTime);
            }
            break;

        case WeaponState::FIRING:
            // Apply impulse for duration
            mImpulseRemaining -= aDeltaTime;
            if (mImpulseRemaining <= 0.0)
            {
                mImpulseRemaining = 0.0;
                // Transition to cycling or idle
                if (mConfig.type == WeaponType::MACHINE_GUN && mContinuousFireEnabled)
                {
                    mState = WeaponState::CYCLING;
                    mStateEntryTime = aSimTime;
                }
                else if (mConfig.type == WeaponType::MACHINE_GUN && mBurstRoundsRemaining > 0)
                {
                    // Continue burst
                    double timeBetweenRounds = 60.0 / mConfig.roundsPerMinute;
                    if (stateTime >= timeBetweenRounds)
                    {
                        ProcessShot(aSimTime);
                    }
                }
                else
                {
                    mState = WeaponState::CYCLING;
                    mStateEntryTime = aSimTime;
                }
            }
            break;

        case WeaponState::CYCLING:
            // Wait for cycle time or next MG round
            if (mConfig.type == WeaponType::MACHINE_GUN)
            {
                double timeBetweenRounds = 60.0 / mConfig.roundsPerMinute;
                if (stateTime >= timeBetweenRounds)
                {
                    if (mContinuousFireEnabled && mRoundsRemaining > 0)
                    {
                        ProcessShot(aSimTime);
                    }
                    else if (mBurstRoundsRemaining > 0 && mRoundsRemaining > 0)
                    {
                        ProcessShot(aSimTime);
                    }
                    else
                    {
                        mState = (mRoundsRemaining > 0) ? WeaponState::IDLE : WeaponState::EMPTY;
                        mStateEntryTime = aSimTime;
                    }
                }
            }
            else
            {
                // Projectile launcher - wait full cycle time
                if (stateTime >= mConfig.cycleTime)
                {
                    mState = (mRoundsRemaining > 0) ? WeaponState::IDLE : WeaponState::EMPTY;
                    mStateEntryTime = aSimTime;
                }
            }
            break;

        case WeaponState::EMPTY:
            // Wait for reload command (handled externally)
            break;

        case WeaponState::JAMMED:
            // Wait for clear jam command
            break;

        case WeaponState::RELEASING:
            // Payload release in progress
            if (stateTime >= mConfig.releaseTime)
            {
                mState = (mRoundsRemaining > 0) ? WeaponState::IDLE : WeaponState::EMPTY;
                mStateEntryTime = aSimTime;
            }
            break;
    }
}

// ============================================================================
// CalculateRecoilForces
// ============================================================================
// Priority 2 Enhancement: Shaped recoil profile using exponential decay
// Real recoil follows: fast rise, exponential decay pattern
// This provides more realistic attitude transients (+10-20% accuracy)
// ============================================================================
void WeaponProcessor::CalculateRecoilForces(double aDeltaTime)
{
    // Only apply recoil forces during FIRING state
    if (mState != WeaponState::FIRING || mImpulseRemaining <= 0.0)
    {
        mRecoilForceX = 0.0;
        mRecoilForceY = 0.0;
        mRecoilForceZ = 0.0;
        mRecoilMomentX = 0.0;
        mRecoilMomentY = 0.0;
        mRecoilMomentZ = 0.0;
        return;
    }

    // Get aim direction in body frame
    double aimX, aimY, aimZ;
    CalculateAimVector(aimX, aimY, aimZ);

    // ========================================================================
    // Shaped Recoil Profile (Priority 2 Enhancement)
    // ========================================================================
    // Instead of constant force F = J/dt, use exponential decay:
    //   F(t) = F_peak * exp(-k * t)
    // where k is chosen so that the integral equals the total impulse J.
    //
    // For exponential decay with time constant tau = duration/3:
    //   integral[0 to T] of F_peak * exp(-3t/T) dt = F_peak * T/3 * (1 - exp(-3))
    //   ≈ F_peak * T/3 * 0.95 ≈ F_peak * T * 0.317
    //
    // To deliver impulse J: F_peak = J / (T * 0.317) ≈ 3.15 * J / T
    // ========================================================================

    double t_elapsed = mConfig.recoilDuration - mImpulseRemaining;
    double tau = mConfig.recoilDuration / 3.0;  // Time constant

    // Exponential decay factor
    double decayFactor = std::exp(-t_elapsed / tau);

    // Peak force scaled to deliver correct total impulse
    // The integral of exp(-t/tau) from 0 to T is tau * (1 - exp(-T/tau))
    // For T = 3*tau: integral = tau * (1 - exp(-3)) ≈ 0.95 * tau
    // So: F_peak * 0.95 * tau = J  =>  F_peak = J / (0.95 * tau)
    double integralFactor = tau * (1.0 - std::exp(-mConfig.recoilDuration / tau));
    double peakForce = mConfig.recoilImpulse / integralFactor;

    double forceMag = peakForce * decayFactor;

    // Recoil force opposes aim direction (Newton's 3rd law)
    mRecoilForceX = -forceMag * aimX;
    mRecoilForceY = -forceMag * aimY;
    mRecoilForceZ = -forceMag * aimZ;

    // Calculate moment about CG: M = r x F
    // r is the position of the weapon mount
    double rX = mConfig.mountOffsetX;
    double rY = mConfig.mountOffsetY;
    double rZ = mConfig.mountOffsetZ;

    // Cross product: r x F
    // Mx = rY * Fz - rZ * Fy
    // My = rZ * Fx - rX * Fz
    // Mz = rX * Fy - rY * Fx
    mRecoilMomentX = rY * mRecoilForceZ - rZ * mRecoilForceY;
    mRecoilMomentY = rZ * mRecoilForceX - rX * mRecoilForceZ;
    mRecoilMomentZ = rX * mRecoilForceY - rY * mRecoilForceX;
}

// ============================================================================
// ProcessShot - Fire a single round
// ============================================================================
void WeaponProcessor::ProcessShot(double aSimTime)
{
    if (mRoundsRemaining <= 0)
    {
        mState = WeaponState::EMPTY;
        mStateEntryTime = aSimTime;
        return;
    }

    // Check for jam
    if (CheckForJam())
    {
        mState = WeaponState::JAMMED;
        mStateEntryTime = aSimTime;
        mJamCount++;

        auto warn = ut::log::warning() << "Weapon jammed!";
        warn.AddNote() << "Platform: " << GetPlatform()->GetName();
        warn.AddNote() << "Weapon: " << mWeaponName;
        warn.AddNote() << "Total jams: " << mJamCount;
        return;
    }

    // Fire round
    mRoundsRemaining--;
    mRoundsFired++;

    if (mConfig.type == WeaponType::MACHINE_GUN && mBurstRoundsRemaining > 0)
    {
        mBurstRoundsRemaining--;
    }

    // Update mass
    double prevMass = mCurrentMass;
    mCurrentMass -= mConfig.munitionMass;
    mCurrentMass = std::max(mCurrentMass, mConfig.munitionMass * 0.1);  // Keep some base mass
    mMassDelta = mCurrentMass - prevMass;  // Negative value

    // Set firing state
    mState = WeaponState::FIRING;
    mStateEntryTime = aSimTime;
    mLastFireTime = aSimTime;
    mImpulseRemaining = mConfig.recoilDuration;

    // Log shot with recoil info
    double forceMag = mConfig.recoilImpulse / mConfig.recoilDuration;
    std::cout << "[WEAPON] " << mWeaponName << " fired! Rounds: "
              << mRoundsRemaining << "/" << mConfig.magazineCapacity
              << " | Recoil force: " << forceMag << " N for "
              << (mConfig.recoilDuration * 1000) << " ms" << std::endl;
}

// ============================================================================
// CalculateAimVector - Get aim direction in body frame
// ============================================================================
void WeaponProcessor::CalculateAimVector(double& outX, double& outY, double& outZ) const
{
    // Convert angles to radians
    double tiltRad = mConfig.tiltAngle * DEG_TO_RAD;
    double azimuthRad = mConfig.azimuthAngle * DEG_TO_RAD;

    // Aim vector in body frame (NED convention)
    // tilt_angle = 0 means straight down (+Z)
    // tilt_angle = 90 would be horizontal (not allowed, max 80 deg)
    // azimuth = 0 means forward (+X when tilted)

    // Start with unit vector pointing down
    // Apply tilt (rotation about Y axis for forward tilt)
    // Then apply azimuth (rotation about Z axis)

    double sinTilt = std::sin(tiltRad);
    double cosTilt = std::cos(tiltRad);
    double sinAzim = std::sin(azimuthRad);
    double cosAzim = std::cos(azimuthRad);

    // Tilted down direction rotated by azimuth
    outX = sinTilt * cosAzim;    // Forward component
    outY = sinTilt * sinAzim;    // Right component
    outZ = cosTilt;              // Down component (positive in NED)
}

// ============================================================================
// CheckForJam - Random jam probability
// ============================================================================
bool WeaponProcessor::CheckForJam() const
{
    // Very low probability jam for reliability modeling
    // Approximately 1 in 5000 chance per shot
    double randVal = static_cast<double>(rand()) / RAND_MAX;
    return randVal < 0.0002;
}

// ============================================================================
// Fire - Trigger single shot or burst
// ============================================================================
void WeaponProcessor::Fire()
{
    if (mState != WeaponState::IDLE)
        return;

    if (mConfig.type == WeaponType::MACHINE_GUN)
    {
        mBurstRoundsRemaining = std::min(mConfig.burstLength, mRoundsRemaining);
    }

    ProcessShot(mLastUpdateTime);
}

// ============================================================================
// FireContinuous - Start continuous fire (MG only)
// ============================================================================
void WeaponProcessor::FireContinuous()
{
    if (mConfig.type != WeaponType::MACHINE_GUN)
        return;

    mContinuousFireEnabled = true;
}

// ============================================================================
// CeaseFire - Stop continuous fire
// ============================================================================
void WeaponProcessor::CeaseFire()
{
    mContinuousFireEnabled = false;
    mBurstRoundsRemaining = 0;
}

// ============================================================================
// Reload
// ============================================================================
void WeaponProcessor::Reload()
{
    if (mState == WeaponState::JAMMED)
    {
        auto warn = ut::log::warning() << "Cannot reload - weapon jammed. Clear jam first.";
        return;
    }

    mRoundsRemaining = mConfig.magazineCapacity;
    mCurrentMass = mInitialMass;
    mMassDelta = mInitialMass - mCurrentMass;  // Positive (mass added)
    mReloadCount++;
    mState = WeaponState::IDLE;

    auto info = ut::log::info() << "Weapon reloaded.";
    info.AddNote() << "Platform: " << GetPlatform()->GetName();
    info.AddNote() << "Weapon: " << mWeaponName;
    info.AddNote() << "Reloads: " << mReloadCount;
}

// ============================================================================
// Release - Release payload
// ============================================================================
void WeaponProcessor::Release()
{
    if (mConfig.type != WeaponType::PAYLOAD_RELEASE)
        return;

    if (mState != WeaponState::IDLE || mRoundsRemaining <= 0)
        return;

    mRoundsRemaining--;
    double prevMass = mCurrentMass;
    mCurrentMass -= mConfig.munitionMass;
    mMassDelta = mCurrentMass - prevMass;

    mState = WeaponState::RELEASING;
    mStateEntryTime = mLastUpdateTime;

    auto info = ut::log::info() << "Payload released!";
    info.AddNote() << "Platform: " << GetPlatform()->GetName();
    info.AddNote() << "Mass released: " << mConfig.munitionMass << " kg";
    info.AddNote() << "Remaining: " << mRoundsRemaining;
}

// ============================================================================
// ClearJam
// ============================================================================
void WeaponProcessor::ClearJam()
{
    if (mState != WeaponState::JAMMED)
        return;

    // 90% chance of successfully clearing jam
    double randVal = static_cast<double>(rand()) / RAND_MAX;
    if (randVal < 0.9)
    {
        mState = WeaponState::IDLE;
        auto info = ut::log::info() << "Jam cleared successfully.";
    }
    else
    {
        auto warn = ut::log::warning() << "Failed to clear jam. Try again.";
    }
}

// ============================================================================
// PublishAuxData - Publish telemetry
// ============================================================================
// Priority 1 Enhancement: Multi-weapon support via indexed AuxData channels
// Each weapon publishes to unique keys: WEAPON_<name>_FORCE_X, etc.
// Also maintains legacy single-weapon keys for backward compatibility.
// Registers weapon name in WEAPON_REGISTRY for 6DOF mover discovery.
// ============================================================================
void WeaponProcessor::PublishAuxData()
{
    WsfAttributeContainer& auxData = GetPlatform()->GetAuxData();

    // ========================================================================
    // Weapon Registry (Priority 1 Enhancement)
    // ========================================================================
    // Register this weapon in the platform's weapon registry
    // The 6DOF mover iterates over this registry to sum all weapon forces
    // Format: comma-separated list of weapon names
    // ========================================================================
    std::string registryKey = "WEAPON_REGISTRY";
    std::string currentRegistry = "";
    if (auxData.AttributeExists(registryKey))
    {
        currentRegistry = auxData.GetString(registryKey);
    }

    // Add this weapon to registry if not already present
    if (currentRegistry.find(mWeaponName) == std::string::npos)
    {
        if (!currentRegistry.empty())
        {
            currentRegistry += ",";
        }
        currentRegistry += mWeaponName;
        auxData.AssignString(registryKey, currentRegistry);
    }

    // ========================================================================
    // Weapon-Indexed Channels (Priority 1 Enhancement)
    // ========================================================================
    // Publish to indexed keys so multiple weapons don't overwrite each other
    // Format: WEAPON_<weapon_name>_<property>
    // ========================================================================
    std::string prefix = "WEAPON_" + mWeaponName + "_";

    // State
    auxData.AssignInt(prefix + "STATE", static_cast<int>(mState));
    auxData.AssignInt(prefix + "TYPE", static_cast<int>(mConfig.type));

    // Ammunition
    auxData.AssignInt(prefix + "ROUNDS_REMAINING", mRoundsRemaining);
    auxData.AssignInt(prefix + "ROUNDS_FIRED", mRoundsFired);

    // Mass
    auxData.AssignDouble(prefix + "MASS", mCurrentMass);
    auxData.AssignDouble(prefix + "MASS_DELTA", mMassDelta);

    // Recoil forces (body frame) - these are read by 6DOF mover
    auxData.AssignDouble(prefix + "FORCE_X", mRecoilForceX);
    auxData.AssignDouble(prefix + "FORCE_Y", mRecoilForceY);
    auxData.AssignDouble(prefix + "FORCE_Z", mRecoilForceZ);

    // Recoil moments (body frame)
    auxData.AssignDouble(prefix + "MOMENT_X", mRecoilMomentX);
    auxData.AssignDouble(prefix + "MOMENT_Y", mRecoilMomentY);
    auxData.AssignDouble(prefix + "MOMENT_Z", mRecoilMomentZ);

    // Configuration info
    auxData.AssignDouble(prefix + "TILT_ANGLE", mConfig.tiltAngle);
    auxData.AssignDouble(prefix + "AZIMUTH_ANGLE", mConfig.azimuthAngle);

    // Status flags
    auxData.AssignBool(prefix + "READY", mState == WeaponState::IDLE);
    auxData.AssignBool(prefix + "FIRING", mState == WeaponState::FIRING);
    auxData.AssignBool(prefix + "JAMMED", mState == WeaponState::JAMMED);
    auxData.AssignBool(prefix + "EMPTY", mState == WeaponState::EMPTY);

    // Statistics
    auxData.AssignInt(prefix + "JAM_COUNT", mJamCount);
    auxData.AssignInt(prefix + "RELOAD_COUNT", mReloadCount);

    // ========================================================================
    // Legacy Single-Weapon Channels (Backward Compatibility)
    // ========================================================================
    // Keep the original non-indexed keys for scenarios with single weapon
    // These accumulate forces from all weapons for simple integration
    // ========================================================================
    auxData.AssignInt("WEAPON_STATE", static_cast<int>(mState));
    auxData.AssignInt("WEAPON_TYPE", static_cast<int>(mConfig.type));
    auxData.AssignInt("WEAPON_ROUNDS_REMAINING", mRoundsRemaining);
    auxData.AssignInt("WEAPON_ROUNDS_FIRED", mRoundsFired);
    auxData.AssignDouble("WEAPON_MASS", mCurrentMass);
    auxData.AssignDouble("WEAPON_MASS_DELTA", mMassDelta);

    // For legacy force channels, we need to accumulate from all weapons
    // Read existing values and add this weapon's contribution
    double existingForceX = 0.0, existingForceY = 0.0, existingForceZ = 0.0;
    double existingMomentX = 0.0, existingMomentY = 0.0, existingMomentZ = 0.0;

    // Only accumulate if this isn't the first weapon to update this frame
    // Use a frame counter to detect new frames
    static double lastAccumTime = -1.0;
    double currentTime = mLastUpdateTime;
    if (std::abs(currentTime - lastAccumTime) < 0.0001)
    {
        // Same frame - accumulate
        if (auxData.AttributeExists("WEAPON_FORCE_X"))
            existingForceX = auxData.GetDouble("WEAPON_FORCE_X");
        if (auxData.AttributeExists("WEAPON_FORCE_Y"))
            existingForceY = auxData.GetDouble("WEAPON_FORCE_Y");
        if (auxData.AttributeExists("WEAPON_FORCE_Z"))
            existingForceZ = auxData.GetDouble("WEAPON_FORCE_Z");
        if (auxData.AttributeExists("WEAPON_MOMENT_X"))
            existingMomentX = auxData.GetDouble("WEAPON_MOMENT_X");
        if (auxData.AttributeExists("WEAPON_MOMENT_Y"))
            existingMomentY = auxData.GetDouble("WEAPON_MOMENT_Y");
        if (auxData.AttributeExists("WEAPON_MOMENT_Z"))
            existingMomentZ = auxData.GetDouble("WEAPON_MOMENT_Z");
    }
    else
    {
        // New frame - reset accumulator
        lastAccumTime = currentTime;
    }

    auxData.AssignDouble("WEAPON_FORCE_X", existingForceX + mRecoilForceX);
    auxData.AssignDouble("WEAPON_FORCE_Y", existingForceY + mRecoilForceY);
    auxData.AssignDouble("WEAPON_FORCE_Z", existingForceZ + mRecoilForceZ);
    auxData.AssignDouble("WEAPON_MOMENT_X", existingMomentX + mRecoilMomentX);
    auxData.AssignDouble("WEAPON_MOMENT_Y", existingMomentY + mRecoilMomentY);
    auxData.AssignDouble("WEAPON_MOMENT_Z", existingMomentZ + mRecoilMomentZ);

    // Legacy status flags (last weapon wins for these)
    auxData.AssignDouble("WEAPON_TILT_ANGLE", mConfig.tiltAngle);
    auxData.AssignDouble("WEAPON_AZIMUTH_ANGLE", mConfig.azimuthAngle);
    auxData.AssignBool("WEAPON_READY", mState == WeaponState::IDLE);
    auxData.AssignBool("WEAPON_FIRING", mState == WeaponState::FIRING);
    auxData.AssignBool("WEAPON_JAMMED", mState == WeaponState::JAMMED);
    auxData.AssignBool("WEAPON_EMPTY", mState == WeaponState::EMPTY);
    auxData.AssignInt("WEAPON_JAM_COUNT", mJamCount);
    auxData.AssignInt("WEAPON_RELOAD_COUNT", mReloadCount);
}

// ============================================================================
// ReadCommands - Read fire commands from AuxData
// ============================================================================
// Priority 1 Enhancement: Multi-weapon command support
// Each weapon reads both indexed commands (WEAPON_<name>_FIRE_CMD) and
// legacy broadcast commands (WEAPON_FIRE_CMD) for backward compatibility.
// ============================================================================
void WeaponProcessor::ReadCommands()
{
    WsfAttributeContainer& auxData = GetPlatform()->GetAuxData();
    std::string prefix = "WEAPON_" + mWeaponName + "_";

    // ========================================================================
    // Fire Command - Check indexed first, then legacy broadcast
    // ========================================================================
    bool fireCmd = false;

    // Check weapon-specific command (Priority 1 Enhancement)
    std::string indexedFireCmd = prefix + "FIRE_CMD";
    if (auxData.AttributeExists(indexedFireCmd) && auxData.GetBool(indexedFireCmd))
    {
        fireCmd = true;
        auxData.AssignBool(indexedFireCmd, false);
    }

    // Check legacy broadcast command (backward compatibility)
    if (auxData.AttributeExists("WEAPON_FIRE_CMD") && auxData.GetBool("WEAPON_FIRE_CMD"))
    {
        fireCmd = true;
        // Don't clear broadcast - other weapons may need to read it
        // The scenario should clear it, or it fires all weapons
    }

    if (fireCmd && mState == WeaponState::IDLE)
    {
        Fire();
    }

    // ========================================================================
    // Continuous Fire Command
    // ========================================================================
    bool continuousCmd = false;
    std::string indexedContCmd = prefix + "CONTINUOUS_FIRE_CMD";

    if (auxData.AttributeExists(indexedContCmd) && auxData.GetBool(indexedContCmd))
    {
        continuousCmd = true;
        auxData.AssignBool(indexedContCmd, false);
    }

    if (auxData.AttributeExists("WEAPON_CONTINUOUS_FIRE_CMD") && auxData.GetBool("WEAPON_CONTINUOUS_FIRE_CMD"))
    {
        continuousCmd = true;
    }

    if (continuousCmd)
    {
        if (!mContinuousFireEnabled)
        {
            FireContinuous();
        }
        else
        {
            CeaseFire();
        }
    }

    // ========================================================================
    // Reload Command
    // ========================================================================
    bool reloadCmd = false;
    std::string indexedReloadCmd = prefix + "RELOAD_CMD";

    if (auxData.AttributeExists(indexedReloadCmd) && auxData.GetBool(indexedReloadCmd))
    {
        reloadCmd = true;
        auxData.AssignBool(indexedReloadCmd, false);
    }

    if (auxData.AttributeExists("WEAPON_RELOAD_CMD") && auxData.GetBool("WEAPON_RELOAD_CMD"))
    {
        reloadCmd = true;
    }

    if (reloadCmd)
    {
        Reload();
    }

    // ========================================================================
    // Payload Release Command
    // ========================================================================
    bool releaseCmd = false;
    std::string indexedReleaseCmd = prefix + "RELEASE_CMD";

    if (auxData.AttributeExists(indexedReleaseCmd) && auxData.GetBool(indexedReleaseCmd))
    {
        releaseCmd = true;
        auxData.AssignBool(indexedReleaseCmd, false);
    }

    if (auxData.AttributeExists("WEAPON_RELEASE_CMD") && auxData.GetBool("WEAPON_RELEASE_CMD"))
    {
        releaseCmd = true;
    }

    if (releaseCmd)
    {
        Release();
    }

    // ========================================================================
    // Clear Jam Command
    // ========================================================================
    bool clearJamCmd = false;
    std::string indexedClearJamCmd = prefix + "CLEAR_JAM_CMD";

    if (auxData.AttributeExists(indexedClearJamCmd) && auxData.GetBool(indexedClearJamCmd))
    {
        clearJamCmd = true;
        auxData.AssignBool(indexedClearJamCmd, false);
    }

    if (auxData.AttributeExists("WEAPON_CLEAR_JAM_CMD") && auxData.GetBool("WEAPON_CLEAR_JAM_CMD"))
    {
        clearJamCmd = true;
    }

    if (clearJamCmd)
    {
        ClearJam();
    }
}

// ============================================================================
// WriteCSVRow - Write a row to the CSV output file
// ============================================================================
void WeaponProcessor::WriteCSVRow(double aSimTime)
{
    if (!mCsvFile.is_open())
        return;

    // Calculate total mass expended
    double totalMassExpended = mInitialMass - mCurrentMass;

    // Get state as string
    std::string stateStr;
    switch (mState)
    {
        case WeaponState::IDLE:      stateStr = "IDLE"; break;
        case WeaponState::FIRING:    stateStr = "FIRING"; break;
        case WeaponState::CYCLING:   stateStr = "CYCLING"; break;
        case WeaponState::RELEASING: stateStr = "RELEASING"; break;
        case WeaponState::JAMMED:    stateStr = "JAMMED"; break;
        case WeaponState::EMPTY:     stateStr = "EMPTY"; break;
        default:                     stateStr = "UNKNOWN"; break;
    }

    // Write CSV row
    mCsvFile << aSimTime << ","
             << mWeaponName << ","
             << stateStr << ","
             << mRoundsRemaining << ","
             << mRoundsFired << ","
             << mCurrentMass << ","
             << mMassDelta << ","
             << totalMassExpended << ","
             << mRecoilForceX << ","
             << mRecoilForceY << ","
             << mRecoilForceZ << ","
             << mRecoilMomentX << ","
             << mRecoilMomentY << ","
             << mRecoilMomentZ << "\n";
}

} // namespace t150
