// ****************************************************************************
// Rotorcraft 6DOF Mover
//
// A GENERIC physics-based 6DOF mover for multirotor aircraft. Implements
// Newton-Euler equations of motion with momentum theory rotor aerodynamics.
//
// This mover is designed to work with ANY multirotor UAS - configuration
// parameters (mass, inertia, rotor specs) are set via the scenario file.
//
// Features:
// - Quaternion-based attitude representation (no gimbal lock)
// - Multiple rotor configurations (quad, hex, octo, coaxial)
// - Motor dynamics with first-order lag
// - Ground effect modeling
// - Aerodynamic drag
// - Route/waypoint navigation via WsfRouteMover
// - Telemetry via AuxData
// - Integration with AFSIM platform state
//
// Supported Configurations:
//   QUAD_X, QUAD_PLUS, HEX_X, OCTO_X, COAXIAL_QUAD, COAXIAL_OCTO, CUSTOM
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef ROTORCRAFT_6DOF_MOVER_HPP
#define ROTORCRAFT_6DOF_MOVER_HPP

#include "RigidBody6DOF.hpp"
#include "RotorModel.hpp"
#include "rotorcraft_6dof_export.h"

#include <WsfRouteMover.hpp>
#include <WsfPlatform.hpp>
#include <WsfScenario.hpp>
#include <WsfRoute.hpp>
#include <WsfWaypoint.hpp>
#include <WsfPath.hpp>
#include <UtInput.hpp>
#include <UtVec3.hpp>

#include <vector>
#include <string>
#include <map>

namespace rotorcraft_6dof
{

// ============================================================================
// Spline navigation enums
// ============================================================================

//! Behavior mode for waypoint navigation
enum class WaypointBehavior
{
    FLY_THROUGH,      //!< Smooth arc through waypoint, maintain speed
    SLOW_PASS,        //!< Decelerate to pass_speed, maintain through waypoint
    STOP_AND_HOVER    //!< Full stop, hold for duration, optionally rotate
};

//! State machine states for spline navigation
enum class SplineNavState
{
    APPROACHING,      //!< Following spline toward waypoint
    AT_WAYPOINT,      //!< Hovering at waypoint (STOP_AND_HOVER)
    ROTATING,         //!< Rotating to departure heading
    DEPARTING         //!< Accelerating away (transitional)
};

// ============================================================================
// Spline navigation configuration structures
// ============================================================================

//! Per-waypoint behavior configuration
struct WaypointConfig
{
    WaypointBehavior behavior = WaypointBehavior::FLY_THROUGH;
    double passSpeed = 2.0;           //!< [m/s] for SLOW_PASS
    double hoverDuration = 0.0;       //!< [sec] for STOP_AND_HOVER
    double departureHeading = -999.0; //!< [deg] -999 = auto (face next waypoint)
};

//! Cached waypoint for spline interpolation (need 4 points: P0, P1, P2, P3)
struct CachedWaypoint
{
    double lat = 0.0;                 //!< Latitude [deg]
    double lon = 0.0;                 //!< Longitude [deg]
    double alt = 0.0;                 //!< Altitude MSL [m]
    double speed = 0.0;               //!< Target speed [m/s]
    int routeIndex = -1;              //!< Original route waypoint index
    WaypointConfig config;            //!< Behavior config for this waypoint
    bool isPhantom = false;           //!< True if extrapolated phantom waypoint
};

// ============================================================================
// Rotor configuration presets
// ============================================================================

enum class RotorConfigType
{
    CUSTOM,         // User-defined positions
    QUAD_X,         // Quadcopter X configuration (4 motors)
    QUAD_PLUS,      // Quadcopter + configuration (4 motors)
    HEX_X,          // Hexacopter (6 motors)
    OCTO_X,         // Octocopter flat (8 motors on 8 arms)
    COAXIAL_QUAD,   // Coaxial quadcopter (8 motors on 4 arms) - T-150 style
    COAXIAL_OCTO    // Coaxial octocopter (16 motors on 8 arms) - future use
};

// ============================================================================
// Drag configuration
// ============================================================================

struct DragConfig
{
    double Cd_x = 1.0;          // Drag coefficient X (forward)
    double Cd_y = 1.2;          // Drag coefficient Y (lateral)
    double Cd_z = 1.5;          // Drag coefficient Z (vertical)
    double frontalArea = 0.30;  // [m^2] Reference area
    bool enabled = true;
};

// ============================================================================
// Ground effect configuration
// ============================================================================

struct GroundEffectConfig
{
    bool enabled = true;
    double heightFactor = 1.0;  // Rotor radius multiplier for GE range
    double maxBoost = 1.25;     // Maximum thrust multiplier near ground
};

// ============================================================================
// Rotorcraft6DOF_Mover - Inherits from WsfRouteMover for proper route handling
// ============================================================================

class ROTORCRAFT_6DOF_EXPORT Rotorcraft6DOF_Mover : public WsfRouteMover
{
public:
    Rotorcraft6DOF_Mover(const WsfScenario& aScenario);
    Rotorcraft6DOF_Mover(const Rotorcraft6DOF_Mover& aSrc);
    ~Rotorcraft6DOF_Mover() override;

    // ========================================================================
    // WsfMover interface
    // ========================================================================

    WsfMover* Clone() const override;
    bool ProcessInput(UtInput& aInput) override;
    bool Initialize(double aSimTime) override;

    // ========================================================================
    // WsfRouteMover pure virtual methods - Route navigation
    // ========================================================================

    //! Adjust guidance parameters from previous waypoint
    void SetGuidanceFrom(const WsfWaypoint& aPreviousWaypoint) override;

    //! Adjust guidance parameters to next waypoint
    void SetGuidanceTo(const WsfWaypoint& aNextWaypoint) override;

    //! Begin movement toward target waypoint
    void AdvanceToNextWaypoint(double aSimTime, const WsfWaypoint& aTarget,
                               const WsfWaypoint* aNextPointPtr) override;

    //! Update position each simulation step (6DOF physics)
    void UpdatePosition(double aSimTime) override;

    //! Set mover to paused state
    void Pause(double aSimTime, double aUnpauseTime) override;

    // ========================================================================
    // Mover state queries
    // ========================================================================

    bool IsStopped() const override;

    // ========================================================================
    // Rotor configuration
    // ========================================================================

    //! Set rotor configuration type (creates default rotor positions)
    void SetRotorConfigType(RotorConfigType type);

    //! Get number of rotors
    size_t GetRotorCount() const { return mRotors.size(); }

    //! Get rotor by index
    RotorModel& GetRotor(size_t index) { return mRotors[index]; }
    const RotorModel& GetRotor(size_t index) const { return mRotors[index]; }

    // ========================================================================
    // 6DOF state access
    // ========================================================================

    //! Get rigid body state
    const RigidBodyState& GetRigidBodyState() const { return mRigidBody.GetState(); }

    //! Get total thrust [N]
    double GetTotalThrust() const;

    //! Get total power consumption [W]
    double GetTotalPower() const;

    //! Get current air density [kg/m^3]
    double GetAirDensity() const { return mCurrentAirDensity; }

    // ========================================================================
    // Command interface (for external controllers)
    // ========================================================================

    //! Set throttle command (0-1) - maps to hover thrust fraction
    void SetThrottleCommand(double throttle);

    //! Set attitude commands (normalized -1 to 1)
    void SetRollCommand(double roll);
    void SetPitchCommand(double pitch);
    void SetYawCommand(double yaw);

    //! Set target altitude for altitude hold mode [m MSL]
    void SetTargetAltitude(double altMSL);

    //! Set target position for position hold mode [deg, deg, m MSL]
    void SetTargetPosition(double lat, double lon, double altMSL);

protected:
    // ========================================================================
    // Physics calculations
    // ========================================================================

    //! Compute ground effect factor for given height AGL
    double ComputeGroundEffectFactor(double heightAGL) const;

    //! Compute aerodynamic drag force in body frame
    Vector3 ComputeDragForce(const Vector3& velocityBody) const;

    //! Sum forces from all rotors (body frame)
    Vector3 ComputeTotalThrustForce() const;

    //! Sum moments from all rotors (body frame)
    Vector3 ComputeTotalRotorMoment() const;

    //! Apply control mixer to generate rotor commands
    void ApplyControlMixer();

    //! Simple altitude hold controller
    void UpdateAltitudeHold(double dt);

    //! Simple position hold controller
    void UpdatePositionHold(double dt);

    //! Publish telemetry data via AuxData
    void PublishTelemetry();

    //! Synchronize 6DOF state to AFSIM platform state
    void SyncToAFSIM();

    //! Initialize 6DOF state from AFSIM platform state
    void InitFromAFSIM();

    // ========================================================================
    // Spline Navigation Methods
    // ========================================================================

    //! Initialize waypoint cache with first 4 waypoints (or phantoms)
    void InitializeWaypointCache();

    //! Advance waypoint cache after reaching waypoint
    void AdvanceWaypointCache();

    //! Get waypoint from route by index (handles bounds)
    bool GetRouteWaypoint(int index, CachedWaypoint& outWaypoint);

    //! Create phantom waypoint by extrapolation
    CachedWaypoint CreatePhantomWaypoint(const CachedWaypoint& p1,
                                          const CachedWaypoint& p2,
                                          bool before);

    //! Get configuration for a waypoint (by route index)
    WaypointConfig GetWaypointConfig(int routeIndex) const;

    //! Update spline navigation state machine
    void UpdateSplineNavigation(double dt, double simTime);

    //! Compute speed profile for current segment
    void ComputeSpeedProfile();

    //! Calculate current spline position and tangent
    //! @param t Spline parameter [0,1]
    //! @param outLat Output latitude [deg]
    //! @param outLon Output longitude [deg]
    //! @param outAlt Output altitude MSL [m]
    void ComputeSplinePosition(double t, double& outLat, double& outLon, double& outAlt) const;

    //! Calculate spline tangent direction at parameter t
    //! @return Tangent vector in local NED frame
    Vector3 ComputeSplineTangent(double t) const;

    //! Calculate arc length of current spline segment
    double CalculateSegmentArcLength() const;

    //! Convert arc distance to spline parameter
    double ArcDistanceToParameter(double arcDist) const;

    //! Convert spline parameter to arc distance
    double ParameterToArcDistance(double t) const;

    //! Get profiled speed at current position
    double GetProfiledSpeed() const;

    //! Transition to new navigation state
    void TransitionToNavState(SplineNavState newState, double simTime);

    //! Parse waypoint behavior string
    static WaypointBehavior ParseBehaviorMode(const std::string& mode);

    // ========================================================================
    // External Force Integration
    // ========================================================================

    //! Read external forces from AuxData (slung load, weapons, etc.)
    //! Returns force vector in body frame [N]
    Vector3 ReadExternalForces();

    //! Read external moments from AuxData (weapon recoil, etc.)
    //! Returns moment vector in body frame [N*m]
    Vector3 ReadExternalMoments();

    //! Update dynamic mass from external processors (weapon ammo, etc.)
    void UpdateDynamicMass();

    // ========================================================================
    // Configuration setup
    // ========================================================================

    //! Create rotors for quadcopter X configuration
    void SetupQuadX(double armLength, double rotorDiameter);

    //! Create rotors for hexacopter configuration
    void SetupHexX(double armLength, double rotorDiameter);

    //! Create rotors for octocopter configuration
    void SetupOctoX(double armLength, double rotorDiameter);

    //! Create rotors for coaxial quad (4 arms, 8 motors) - T-150 style
    void SetupCoaxialQuad(double armLength, double rotorDiameter, double coaxialSpacing);

    //! Create rotors for coaxial octo (8 arms, 16 motors) - future use
    void SetupCoaxialOcto(double armLength, double rotorDiameter, double coaxialSpacing);

private:
    // Rigid body dynamics
    RigidBody6DOF mRigidBody;

    // Rotor array
    std::vector<RotorModel> mRotors;
    RotorConfigType mConfigType = RotorConfigType::QUAD_X;

    // Default rotor parameters
    double mDefaultRotorDiameter = 0.254;   // [m] Default 10" props
    double mDefaultArmLength = 0.25;        // [m] Motor arm length
    double mDefaultCoaxialSpacing = 0.05;   // [m] Spacing between coaxial rotors
    double mDefaultThrustCoeff = 0.10;      // CT
    double mDefaultTorqueCoeff = 0.008;     // CQ
    double mDefaultMaxRPM = 10000.0;
    double mDefaultMotorTimeConstant = 0.05;

    // Drag configuration
    DragConfig mDragConfig;

    // Ground effect configuration
    GroundEffectConfig mGroundEffectConfig;

    // Command inputs
    double mThrottleCmd = 0.0;      // 0-1
    double mRollCmd = 0.0;          // -1 to 1
    double mPitchCmd = 0.0;         // -1 to 1
    double mYawCmd = 0.0;           // -1 to 1

    // Control gains
    double mRollMixGain = 0.3;      // Roll command mixing gain
    double mPitchMixGain = 0.3;     // Pitch command mixing gain
    double mYawMixGain = 0.2;       // Yaw command mixing gain

    // Altitude hold (PID controller)
    bool mAltitudeHoldEnabled = false;
    double mTargetAltMSL = 0.0;
    double mAltitudeKp = 0.3;       // Proportional gain
    double mAltitudeKi = 0.05;      // Integral gain (small to avoid oscillation)
    double mAltitudeKd = 1.2;       // Derivative gain for damping
    double mAltitudeIntegral = 0.0; // Integral accumulator
    double mAltitudeIntegralMax = 3.0; // Anti-windup limit (allows I to accumulate enough)

    // Position hold
    bool mPositionHoldEnabled = false;
    double mTargetLat = 0.0;
    double mTargetLon = 0.0;
    double mPositionKp = 0.3;
    double mPositionKd = 0.8;       // Position derivative gain - velocity damping
    double mVelocityKp = 0.5;
    double mMaxTiltAngle = 30.0 * DEG_TO_RAD;  // Max tilt for position control
    double mHeadingKp = 1.0;        // Heading control gain [1/s]
    bool mHeadingHoldEnabled = true; // Face direction of travel

    // Rate-limited desired velocity (prevents sudden reversals)
    double mDesVelN = 0.0;          // Current rate-limited desired velocity North
    double mDesVelE = 0.0;          // Current rate-limited desired velocity East
    double mMaxVelAccel = 1.0;      // Max desired velocity change rate [m/s²]

    // Environmental state
    double mCurrentAirDensity = AIR_DENSITY_SL;
    double mHeightAGL = 0.0;

    // Origin for local NED frame (for position control)
    double mOriginLat = 0.0;
    double mOriginLon = 0.0;
    double mOriginAlt = 0.0;
    bool mOriginSet = false;

    // Simulation timing
    double mLastSimTime = 0.0;

    // Cached hover RPM for the current configuration
    double mHoverRPM = 0.0;

    // ========================================================================
    // Route navigation state (managed by WsfRouteMover, we track current target)
    // ========================================================================
    double mTargetSpeed = 5.0;              // [m/s] Current target speed from waypoint
    double mWaypointArrivalRadius = 10.0;   // [m] Distance threshold to consider waypoint reached
    bool mIsPaused = false;                 // Pause state
    double mPauseEndTime = 0.0;             // When to unpause

    // ========================================================================
    // Spline Navigation Configuration
    // ========================================================================
    bool mSplineEnabled = false;            // Enable spline interpolation
    double mSplineTension = 0.5;            // Catmull-Rom tension [0-1]

    // Default waypoint behavior settings
    WaypointBehavior mDefaultBehavior = WaypointBehavior::FLY_THROUGH;
    double mDefaultPassSpeed = 2.0;         // [m/s] Default slow-pass speed
    double mDefaultHoverDuration = 3.0;     // [sec] Default hover duration

    // Per-waypoint configuration (keyed by route index)
    std::map<int, WaypointConfig> mWaypointConfigs;

    // ========================================================================
    // Spline Navigation State
    // ========================================================================
    // Waypoint cache (need 4 points for Catmull-Rom: P0, P1, P2, P3)
    // Current segment is P1 -> P2
    std::vector<CachedWaypoint> mWaypointCache;

    // Current waypoint index in original route
    int mCurrentWaypointIndex = 0;
    int mTotalWaypoints = 0;

    // Spline interpolation state
    double mSplineParameter = 0.0;          // t in [0,1] along current segment
    double mSegmentArcLength = 0.0;         // Total segment length [m]
    double mDistanceAlongSegment = 0.0;     // Current position along segment [m]

    // Navigation state machine
    SplineNavState mNavState = SplineNavState::APPROACHING;
    double mHoverStartTime = 0.0;           // When hover started
    double mHoverEndTime = 0.0;             // When hover should end
    double mDepartureHeading = 0.0;         // Target heading after hover [rad]
    bool mDepartureHeadingSet = false;      // Whether departure heading was specified

    // Speed profiling
    double mProfiledSpeed = 0.0;            // Current profiled speed [m/s]
    double mSegmentEntrySpeed = 0.0;        // Speed at start of segment [m/s]
    double mSegmentExitSpeed = 0.0;         // Target speed at end of segment [m/s]
    double mDecelStartDistance = 0.0;       // Distance from end to start decel [m]
    double mMaxDeceleration = 2.0;          // Maximum deceleration [m/s²]

    // ========================================================================
    // Telemetry configuration
    // ========================================================================
    bool mTelemetryEnabled = true;          // Enable AuxData telemetry output
};

} // namespace rotorcraft_6dof

using Rotorcraft6DOF_Mover = rotorcraft_6dof::Rotorcraft6DOF_Mover;

#endif // ROTORCRAFT_6DOF_MOVER_HPP
