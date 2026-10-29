// ****************************************************************************
// T-150 UAS Weapon/Launcher Processor Plugin
//
// This plugin models various weapon systems that can be mounted on the T-150 UAS.
// It provides physics-based recoil forces that couple to the 6DOF mover via AuxData.
//
// Supported Weapon Types:
// - PROJECTILE_LAUNCHER: Discrete high-impulse recoil (grenades, rockets)
// - MACHINE_GUN: Rapid-fire sustained recoil
// - PAYLOAD_RELEASE: Mass change effects (bombs, cargo drops)
//
// Physics Model:
// - Recoil impulse: J = m_projectile * v_muzzle (momentum transfer)
// - Recoil force: F = J / dt_impulse (during impulse duration)
// - Moment: M = r x F (about CG based on mount offset)
// - Mass tracking: Updates platform mass as ammunition is expended
//
// AuxData Interface:
// - Publishes: WEAPON_FORCE_X/Y/Z, WEAPON_MOMENT_X/Y/Z, WEAPON_STATE, etc.
// - Consumes: WEAPON_FIRE_CMD (from scenario script)
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_WEAPON_PROCESSOR_HPP
#define T150_WEAPON_PROCESSOR_HPP

#include "WsfProcessor.hpp"

#include <fstream>
#include <string>

namespace t150
{

// ============================================================================
// Weapon Types
// ============================================================================

enum class WeaponType
{
    PROJECTILE_LAUNCHER,  // Discrete high-impulse recoil (grenades, rockets)
    MACHINE_GUN,          // Rapid-fire sustained recoil
    PAYLOAD_RELEASE       // Mass change, no recoil (bombs, cargo)
};

// ============================================================================
// Weapon State Machine
// ============================================================================

enum class WeaponState
{
    IDLE,           // Ready to fire
    FIRING,         // Actively firing (impulse being applied)
    CYCLING,        // Between shots (machine gun) or reloading
    EMPTY,          // No ammunition remaining
    JAMMED,         // Weapon malfunction
    RELEASING       // Payload being released
};

// ============================================================================
// Weapon Configuration Structure
// ============================================================================

struct WeaponConfig
{
    // Weapon type
    WeaponType type = WeaponType::PROJECTILE_LAUNCHER;

    // Munition properties
    double munitionMass = 0.5;        // [kg] Mass per round (bullet only)
    double muzzleVelocity = 100.0;    // [m/s] Projectile exit velocity
    double recoilImpulse = 0.0;       // [N*s] Force-time product (0 = auto-calculate from mass*velocity*gasFactor)
    double recoilDuration = 0.02;     // [s] Duration of impulse
    double propellantGasFactor = 1.3; // [-] Multiplier for propellant gas momentum (typically 1.3-1.5)

    // Magazine/capacity
    int magazineCapacity = 10;        // Rounds per magazine
    double cycleTime = 1.0;           // [s] Time between shots (or burst rate for MG)
    double reloadTime = 5.0;          // [s] Time to reload magazine

    // Mount configuration (body frame, origin at CG)
    double mountOffsetX = 0.0;        // [m] Forward of CG (+X)
    double mountOffsetY = 0.0;        // [m] Right of CG (+Y)
    double mountOffsetZ = 0.3;        // [m] Below CG (+Z in NED)

    // Aiming angles
    double tiltAngle = 30.0;          // [deg] Weapon aim angle from vertical (0=straight down)
    double azimuthAngle = 0.0;        // [deg] Azimuth from forward (0=forward)

    // Machine gun specific
    int burstLength = 3;              // Rounds per burst
    double roundsPerMinute = 600.0;   // Rate of fire for MG

    // Payload release specific
    double releaseTime = 0.5;         // [s] Time to complete release

    // Test mode options
    bool autoFireEnabled = false;     // Enable automatic firing for testing
    double autoFireStartTime = 10.0;  // [s] First auto-fire time
    double autoFireInterval = 5.0;    // [s] Interval between auto-fires
    int autoFireCount = 3;            // Number of auto-fires (0 = unlimited)
};

// ============================================================================
// WeaponProcessor Class
// ============================================================================

class WeaponProcessor : public WsfProcessor
{
public:
    //! Constructor
    explicit WeaponProcessor(const WsfScenario& aScenario);

    //! Destructor
    ~WeaponProcessor() override = default;

    //! Get the class ID for this processor type
    static WsfStringId GetWeaponProcessorClassId();

    //! @name Framework methods
    //@{
    WsfProcessor* Clone() const override;
    bool          ProcessInput(UtInput& aInput) override;
    bool          Initialize(double aSimTime) override;
    void          Update(double aSimTime) override;
    const char*   GetScriptClassName() const override;
    //@}

    //! @name Weapon State Accessors
    //@{
    WeaponType GetWeaponType() const { return mConfig.type; }
    WeaponState GetWeaponState() const { return mState; }
    int GetRoundsRemaining() const { return mRoundsRemaining; }
    int GetRoundsFired() const { return mRoundsFired; }
    double GetCurrentMass() const { return mCurrentMass; }
    double GetMassDelta() const { return mMassDelta; }
    bool IsReady() const { return mState == WeaponState::IDLE; }
    //@}

    //! @name Recoil Force Accessors (body frame)
    //@{
    double GetRecoilForceX() const { return mRecoilForceX; }
    double GetRecoilForceY() const { return mRecoilForceY; }
    double GetRecoilForceZ() const { return mRecoilForceZ; }
    double GetRecoilMomentX() const { return mRecoilMomentX; }
    double GetRecoilMomentY() const { return mRecoilMomentY; }
    double GetRecoilMomentZ() const { return mRecoilMomentZ; }
    //@}

    //! @name Weapon Commands
    //@{
    void Fire();              // Trigger single shot or burst
    void FireContinuous();    // Start continuous fire (MG)
    void CeaseFire();         // Stop continuous fire
    void Reload();            // Reload magazine
    void Release();           // Release payload
    void ClearJam();          // Attempt to clear weapon jam
    //@}

protected:
    //! Copy constructor for Clone()
    WeaponProcessor(const WeaponProcessor& aSrc);

private:
    //! State machine update
    void UpdateStateMachine(double aSimTime, double aDeltaTime);

    //! Calculate recoil forces for current firing state
    void CalculateRecoilForces(double aDeltaTime);

    //! Process single shot
    void ProcessShot(double aSimTime);

    //! Calculate aim direction in body frame
    void CalculateAimVector(double& outX, double& outY, double& outZ) const;

    //! Check for jam condition
    bool CheckForJam() const;

    //! Publish telemetry to AuxData
    void PublishAuxData();

    //! Read commands from AuxData
    void ReadCommands();

    //! Write a row to the CSV output file
    void WriteCSVRow(double aSimTime);

    // =========================================================================
    // Configuration
    // =========================================================================
    WeaponConfig mConfig;
    std::string mWeaponName;

    // =========================================================================
    // Dynamic State
    // =========================================================================
    WeaponState mState;

    // Ammunition tracking
    int mRoundsRemaining;         // Current rounds in magazine
    int mRoundsFired;             // Total rounds fired
    int mBurstRoundsRemaining;    // Rounds remaining in current burst (MG)

    // Mass tracking
    double mCurrentMass;          // Current weapon + ammo mass [kg]
    double mInitialMass;          // Initial total mass [kg]
    double mMassDelta;            // Mass change this frame [kg]

    // Timing state
    double mLastFireTime;         // Time of last shot
    double mStateEntryTime;       // Time when current state was entered
    double mImpulseRemaining;     // Remaining impulse duration [s]

    // Continuous fire mode
    bool mContinuousFireEnabled;

    // Recoil forces (body frame)
    double mRecoilForceX;         // [N] Recoil force X
    double mRecoilForceY;         // [N] Recoil force Y
    double mRecoilForceZ;         // [N] Recoil force Z
    double mRecoilMomentX;        // [N*m] Recoil moment about X (roll)
    double mRecoilMomentY;        // [N*m] Recoil moment about Y (pitch)
    double mRecoilMomentZ;        // [N*m] Recoil moment about Z (yaw)

    // Timing
    double mLastUpdateTime;
    double mUpdateInterval;

    // Statistics
    int mJamCount;
    int mReloadCount;

    // Auto-fire tracking
    int mAutoFiresRemaining;
    double mNextAutoFireTime;

    // CSV output
    std::string mCsvOutputPath;
    std::ofstream mCsvFile;
    bool mCsvEnabled;
    double mCsvLogInterval;
    double mLastCsvLogTime;
};

} // namespace t150

#endif // T150_WEAPON_PROCESSOR_HPP
