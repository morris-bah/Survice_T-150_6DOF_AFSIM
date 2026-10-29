// ****************************************************************************
// Rotorcraft 6DOF Mover - Implementation
//
// Refactored to inherit from WsfRouteMover for proper AFSIM route integration.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "Rotorcraft6DOF_Mover.hpp"

#include <WsfPlatform.hpp>
#include <WsfScenario.hpp>
#include <WsfRouteMover.hpp>
#include <WsfAttributeContainer.hpp>
#include <UtInput.hpp>
#include <UtEarth.hpp>

#include <cmath>
#include <algorithm>
#include <sstream>
#include <iostream>

namespace rotorcraft_6dof
{

// ============================================================================
// Local geodetic conversion utilities (flat-earth approximation for small distances)
// ============================================================================

namespace
{
    // WGS-84 Earth radius [m]
    constexpr double EARTH_RADIUS = 6378137.0;

    //! Convert offset from one geodetic position to another into NED coordinates
    //! Uses flat-earth approximation (valid for distances < ~100km)
    //! @param fromLat Origin latitude [deg]
    //! @param fromLon Origin longitude [deg]
    //! @param toLat Target latitude [deg]
    //! @param toLon Target longitude [deg]
    //! @param outNorth Output: North offset [m]
    //! @param outEast Output: East offset [m]
    void GeodeticOffsetToNED(double fromLat, double fromLon,
                              double toLat, double toLon,
                              double& outNorth, double& outEast)
    {
        // Convert degrees to radians
        double fromLatRad = fromLat * DEG_TO_RAD;
        double fromLonRad = fromLon * DEG_TO_RAD;
        double toLatRad = toLat * DEG_TO_RAD;
        double toLonRad = toLon * DEG_TO_RAD;

        // Latitude difference -> North offset
        // One radian of latitude = Earth radius in meters
        outNorth = (toLatRad - fromLatRad) * EARTH_RADIUS;

        // Longitude difference -> East offset
        // Scale by cos(lat) because longitude lines converge at poles
        double avgLatRad = (fromLatRad + toLatRad) * 0.5;
        outEast = (toLonRad - fromLonRad) * EARTH_RADIUS * std::cos(avgLatRad);
    }

    //! Convert NED offset back to geodetic coordinates
    //! @param originLat Origin latitude [deg]
    //! @param originLon Origin longitude [deg]
    //! @param north North offset [m]
    //! @param east East offset [m]
    //! @param outLat Output: Target latitude [deg]
    //! @param outLon Output: Target longitude [deg]
    void NEDOffsetToGeodetic(double originLat, double originLon,
                              double north, double east,
                              double& outLat, double& outLon)
    {
        // Convert origin to radians for calculation
        double originLatRad = originLat * DEG_TO_RAD;

        // North offset -> latitude change (in radians, then convert to degrees)
        double deltaLatRad = north / EARTH_RADIUS;
        outLat = originLat + (deltaLatRad * RAD_TO_DEG);

        // East offset -> longitude change (accounting for latitude)
        double cosLat = std::cos(originLatRad);
        if (cosLat > 0.001)  // Avoid division by zero near poles
        {
            double deltaLonRad = east / (EARTH_RADIUS * cosLat);
            outLon = originLon + (deltaLonRad * RAD_TO_DEG);
        }
        else
        {
            outLon = originLon;  // Near pole, longitude is undefined
        }
    }
} // anonymous namespace

// ============================================================================
// Parse behavior mode string
// ============================================================================

WaypointBehavior Rotorcraft6DOF_Mover::ParseBehaviorMode(const std::string& mode)
{
    if (mode == "STOP_AND_HOVER" || mode == "stop_and_hover" || mode == "STOP")
        return WaypointBehavior::STOP_AND_HOVER;
    else if (mode == "SLOW_PASS" || mode == "slow_pass" || mode == "SLOW")
        return WaypointBehavior::SLOW_PASS;
    else
        return WaypointBehavior::FLY_THROUGH;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

Rotorcraft6DOF_Mover::Rotorcraft6DOF_Mover(const WsfScenario& aScenario)
    : WsfRouteMover(aScenario)
{
    SetUpdateInterval(0.01);  // 100 Hz default for physics stability
}

Rotorcraft6DOF_Mover::Rotorcraft6DOF_Mover(const Rotorcraft6DOF_Mover& aSrc)
    : WsfRouteMover(aSrc)
    , mRigidBody(aSrc.mRigidBody)
    , mRotors(aSrc.mRotors)
    , mConfigType(aSrc.mConfigType)
    , mDefaultRotorDiameter(aSrc.mDefaultRotorDiameter)
    , mDefaultArmLength(aSrc.mDefaultArmLength)
    , mDefaultCoaxialSpacing(aSrc.mDefaultCoaxialSpacing)
    , mDefaultThrustCoeff(aSrc.mDefaultThrustCoeff)
    , mDefaultTorqueCoeff(aSrc.mDefaultTorqueCoeff)
    , mDefaultMaxRPM(aSrc.mDefaultMaxRPM)
    , mDefaultMotorTimeConstant(aSrc.mDefaultMotorTimeConstant)
    , mDragConfig(aSrc.mDragConfig)
    , mGroundEffectConfig(aSrc.mGroundEffectConfig)
    , mThrottleCmd(aSrc.mThrottleCmd)
    , mRollCmd(aSrc.mRollCmd)
    , mPitchCmd(aSrc.mPitchCmd)
    , mYawCmd(aSrc.mYawCmd)
    , mRollMixGain(aSrc.mRollMixGain)
    , mPitchMixGain(aSrc.mPitchMixGain)
    , mYawMixGain(aSrc.mYawMixGain)
    , mAltitudeHoldEnabled(aSrc.mAltitudeHoldEnabled)
    , mTargetAltMSL(aSrc.mTargetAltMSL)
    , mAltitudeKp(aSrc.mAltitudeKp)
    , mAltitudeKd(aSrc.mAltitudeKd)
    , mPositionHoldEnabled(aSrc.mPositionHoldEnabled)
    , mTargetLat(aSrc.mTargetLat)
    , mTargetLon(aSrc.mTargetLon)
    , mPositionKp(aSrc.mPositionKp)
    , mPositionKd(aSrc.mPositionKd)
    , mVelocityKp(aSrc.mVelocityKp)
    , mMaxTiltAngle(aSrc.mMaxTiltAngle)
    , mHeadingKp(aSrc.mHeadingKp)
    , mHeadingHoldEnabled(aSrc.mHeadingHoldEnabled)
    , mDesVelN(aSrc.mDesVelN)
    , mDesVelE(aSrc.mDesVelE)
    , mMaxVelAccel(aSrc.mMaxVelAccel)
    , mCurrentAirDensity(aSrc.mCurrentAirDensity)
    , mHeightAGL(aSrc.mHeightAGL)
    , mOriginLat(aSrc.mOriginLat)
    , mOriginLon(aSrc.mOriginLon)
    , mOriginAlt(aSrc.mOriginAlt)
    , mOriginSet(aSrc.mOriginSet)
    , mLastSimTime(aSrc.mLastSimTime)
    , mHoverRPM(aSrc.mHoverRPM)
    , mTargetSpeed(aSrc.mTargetSpeed)
    , mWaypointArrivalRadius(aSrc.mWaypointArrivalRadius)
    , mIsPaused(aSrc.mIsPaused)
    , mPauseEndTime(aSrc.mPauseEndTime)
    // Spline navigation configuration
    , mSplineEnabled(aSrc.mSplineEnabled)
    , mSplineTension(aSrc.mSplineTension)
    , mDefaultBehavior(aSrc.mDefaultBehavior)
    , mDefaultPassSpeed(aSrc.mDefaultPassSpeed)
    , mDefaultHoverDuration(aSrc.mDefaultHoverDuration)
    , mWaypointConfigs(aSrc.mWaypointConfigs)
    // Spline navigation state
    , mWaypointCache(aSrc.mWaypointCache)
    , mCurrentWaypointIndex(aSrc.mCurrentWaypointIndex)
    , mTotalWaypoints(aSrc.mTotalWaypoints)
    , mSplineParameter(aSrc.mSplineParameter)
    , mSegmentArcLength(aSrc.mSegmentArcLength)
    , mDistanceAlongSegment(aSrc.mDistanceAlongSegment)
    , mNavState(aSrc.mNavState)
    , mHoverStartTime(aSrc.mHoverStartTime)
    , mHoverEndTime(aSrc.mHoverEndTime)
    , mDepartureHeading(aSrc.mDepartureHeading)
    , mDepartureHeadingSet(aSrc.mDepartureHeadingSet)
    // Speed profiling
    , mProfiledSpeed(aSrc.mProfiledSpeed)
    , mSegmentEntrySpeed(aSrc.mSegmentEntrySpeed)
    , mSegmentExitSpeed(aSrc.mSegmentExitSpeed)
    , mDecelStartDistance(aSrc.mDecelStartDistance)
    , mMaxDeceleration(aSrc.mMaxDeceleration)
    // Telemetry
    , mTelemetryEnabled(aSrc.mTelemetryEnabled)
{
}

Rotorcraft6DOF_Mover::~Rotorcraft6DOF_Mover() = default;

// ============================================================================
// Clone
// ============================================================================

WsfMover* Rotorcraft6DOF_Mover::Clone() const
{
    return new Rotorcraft6DOF_Mover(*this);
}

// ============================================================================
// ProcessInput - Parse configuration (route handling delegated to parent)
// ============================================================================

bool Rotorcraft6DOF_Mover::ProcessInput(UtInput& aInput)
{
    bool myCommand = true;
    std::string command(aInput.GetCommand());

    // Mass properties
    if (command == "mass")
    {
        aInput.ReadValue(mRigidBody.GetMassProperties().mass);
    }
    else if (command == "inertia_xx")
    {
        aInput.ReadValue(mRigidBody.GetMassProperties().Ixx);
    }
    else if (command == "inertia_yy")
    {
        aInput.ReadValue(mRigidBody.GetMassProperties().Iyy);
    }
    else if (command == "inertia_zz")
    {
        aInput.ReadValue(mRigidBody.GetMassProperties().Izz);
    }
    // Rotor configuration type
    else if (command == "rotor_configuration")
    {
        std::string configName;
        aInput.ReadValue(configName);
        if (configName == "QUAD_X")
            mConfigType = RotorConfigType::QUAD_X;
        else if (configName == "QUAD_PLUS")
            mConfigType = RotorConfigType::QUAD_PLUS;
        else if (configName == "HEX_X")
            mConfigType = RotorConfigType::HEX_X;
        else if (configName == "OCTO_X")
            mConfigType = RotorConfigType::OCTO_X;
        else if (configName == "COAXIAL_QUAD")
            mConfigType = RotorConfigType::COAXIAL_QUAD;
        else if (configName == "COAXIAL_OCTO")
            mConfigType = RotorConfigType::COAXIAL_OCTO;
        else if (configName == "CUSTOM")
            mConfigType = RotorConfigType::CUSTOM;
    }
    // Rotor parameters
    else if (command == "rotor_diameter")
    {
        aInput.ReadValue(mDefaultRotorDiameter);
    }
    else if (command == "arm_length")
    {
        aInput.ReadValue(mDefaultArmLength);
    }
    else if (command == "coaxial_spacing")
    {
        aInput.ReadValue(mDefaultCoaxialSpacing);
    }
    else if (command == "rotor_ct" || command == "thrust_coefficient")
    {
        aInput.ReadValue(mDefaultThrustCoeff);
    }
    else if (command == "rotor_cq" || command == "torque_coefficient")
    {
        aInput.ReadValue(mDefaultTorqueCoeff);
    }
    else if (command == "rotor_max_rpm" || command == "max_rpm")
    {
        aInput.ReadValue(mDefaultMaxRPM);
    }
    else if (command == "motor_time_constant")
    {
        aInput.ReadValue(mDefaultMotorTimeConstant);
    }
    // Drag
    else if (command == "drag_enabled")
    {
        mDragConfig.enabled = aInput.ReadBool();
    }
    else if (command == "drag_coefficient_x")
    {
        aInput.ReadValue(mDragConfig.Cd_x);
    }
    else if (command == "drag_coefficient_y")
    {
        aInput.ReadValue(mDragConfig.Cd_y);
    }
    else if (command == "drag_coefficient_z")
    {
        aInput.ReadValue(mDragConfig.Cd_z);
    }
    else if (command == "frontal_area")
    {
        aInput.ReadValue(mDragConfig.frontalArea);
    }
    // Ground effect
    else if (command == "ground_effect_enabled")
    {
        mGroundEffectConfig.enabled = aInput.ReadBool();
    }
    else if (command == "ground_effect_height_factor")
    {
        aInput.ReadValue(mGroundEffectConfig.heightFactor);
    }
    else if (command == "ground_effect_max_boost")
    {
        aInput.ReadValue(mGroundEffectConfig.maxBoost);
    }
    // Control mixing
    else if (command == "roll_mix_gain")
    {
        aInput.ReadValue(mRollMixGain);
    }
    else if (command == "pitch_mix_gain")
    {
        aInput.ReadValue(mPitchMixGain);
    }
    else if (command == "yaw_mix_gain")
    {
        aInput.ReadValue(mYawMixGain);
    }
    // Altitude hold
    else if (command == "altitude_hold")
    {
        mAltitudeHoldEnabled = aInput.ReadBool();
    }
    else if (command == "altitude_kp")
    {
        aInput.ReadValue(mAltitudeKp);
    }
    else if (command == "altitude_kd")
    {
        aInput.ReadValue(mAltitudeKd);
    }
    // Position hold
    else if (command == "position_hold")
    {
        mPositionHoldEnabled = aInput.ReadBool();
    }
    else if (command == "position_kp")
    {
        aInput.ReadValue(mPositionKp);
    }
    else if (command == "position_kd")
    {
        aInput.ReadValue(mPositionKd);
    }
    else if (command == "velocity_kp")
    {
        aInput.ReadValue(mVelocityKp);
    }
    else if (command == "max_vel_accel")
    {
        aInput.ReadValue(mMaxVelAccel);
    }
    else if (command == "max_tilt_angle")
    {
        aInput.ReadValue(mMaxTiltAngle);
        mMaxTiltAngle *= DEG_TO_RAD;
    }
    else if (command == "heading_kp")
    {
        aInput.ReadValue(mHeadingKp);
    }
    else if (command == "heading_hold")
    {
        mHeadingHoldEnabled = aInput.ReadBool();
    }
    // Waypoint arrival radius
    else if (command == "waypoint_arrival_radius")
    {
        aInput.ReadValue(mWaypointArrivalRadius);
    }
    // Telemetry enable/disable
    else if (command == "telemetry_enabled" || command == "enable_telemetry")
    {
        mTelemetryEnabled = aInput.ReadBool();
    }
    // ========================================================================
    // Spline Navigation Configuration
    // ========================================================================
    else if (command == "spline_interpolation")
    {
        mSplineEnabled = aInput.ReadBool();
        if (mSplineEnabled)
        {
            std::cout << "[ROTORCRAFT_6DOF] Spline interpolation ENABLED" << std::endl;
        }
    }
    else if (command == "spline_tension")
    {
        aInput.ReadValue(mSplineTension);
        mSplineTension = Clamp(mSplineTension, 0.0, 1.0);
    }
    else if (command == "max_deceleration")
    {
        aInput.ReadValue(mMaxDeceleration);
    }
    else if (command == "default_waypoint_behavior")
    {
        std::string mode;
        aInput.ReadValue(mode);
        mDefaultBehavior = ParseBehaviorMode(mode);
    }
    else if (command == "default_pass_speed")
    {
        aInput.ReadValue(mDefaultPassSpeed);
    }
    else if (command == "default_hover_duration")
    {
        aInput.ReadValue(mDefaultHoverDuration);
    }
    else if (command == "waypoint_behavior")
    {
        // Format: waypoint_behavior <index> <mode>
        // For STOP_AND_HOVER and SLOW_PASS, use default settings
        // Use waypoint_behavior_stop, waypoint_behavior_slow for custom params
        int index;
        std::string mode;
        aInput.ReadValue(index);
        aInput.ReadValue(mode);

        WaypointConfig config;
        config.behavior = ParseBehaviorMode(mode);
        config.hoverDuration = mDefaultHoverDuration;
        config.passSpeed = mDefaultPassSpeed;
        config.departureHeading = -999.0;  // Auto

        mWaypointConfigs[index] = config;

        std::cout << "[ROTORCRAFT_6DOF] Waypoint " << index << " behavior: " << mode << std::endl;
    }
    else if (command == "waypoint_stop")
    {
        // Format: waypoint_stop <index> <hover_duration> [departure_heading]
        // Example: waypoint_stop 2 5.0 90.0   // hover 5s, face 90 deg
        int index;
        double hoverDuration;
        aInput.ReadValue(index);
        aInput.ReadValue(hoverDuration);

        WaypointConfig config;
        config.behavior = WaypointBehavior::STOP_AND_HOVER;
        config.hoverDuration = hoverDuration;
        config.departureHeading = -999.0;  // Auto by default

        mWaypointConfigs[index] = config;

        std::cout << "[ROTORCRAFT_6DOF] Waypoint " << index << " STOP_AND_HOVER: "
                  << hoverDuration << "s" << std::endl;
    }
    else if (command == "waypoint_stop_heading")
    {
        // Format: waypoint_stop_heading <index> <hover_duration> <departure_heading>
        // Example: waypoint_stop_heading 2 5.0 90.0   // hover 5s, face 90 deg
        int index;
        double hoverDuration, departureHeading;
        aInput.ReadValue(index);
        aInput.ReadValue(hoverDuration);
        aInput.ReadValue(departureHeading);

        WaypointConfig config;
        config.behavior = WaypointBehavior::STOP_AND_HOVER;
        config.hoverDuration = hoverDuration;
        config.departureHeading = departureHeading;

        mWaypointConfigs[index] = config;

        std::cout << "[ROTORCRAFT_6DOF] Waypoint " << index << " STOP_AND_HOVER: "
                  << hoverDuration << "s, heading=" << departureHeading << " deg" << std::endl;
    }
    else if (command == "waypoint_slow")
    {
        // Format: waypoint_slow <index> <pass_speed>
        // Example: waypoint_slow 4 2.0   // pass at 2 m/s
        int index;
        double passSpeed;
        aInput.ReadValue(index);
        aInput.ReadValue(passSpeed);

        WaypointConfig config;
        config.behavior = WaypointBehavior::SLOW_PASS;
        config.passSpeed = passSpeed;

        mWaypointConfigs[index] = config;

        std::cout << "[ROTORCRAFT_6DOF] Waypoint " << index << " SLOW_PASS: "
                  << passSpeed << " m/s" << std::endl;
    }
    // Unknown command - let parent class handle (including route blocks)
    else
    {
        myCommand = WsfRouteMover::ProcessInput(aInput);
    }

    return myCommand;
}

// ============================================================================
// Initialize
// ============================================================================

bool Rotorcraft6DOF_Mover::Initialize(double aSimTime)
{
    // Let parent class initialize (handles route setup)
    if (!WsfRouteMover::Initialize(aSimTime))
        return false;

    // Set up rotor configuration based on type
    SetRotorConfigType(mConfigType);

    // Initialize 6DOF state from AFSIM platform
    InitFromAFSIM();

    // Calculate hover RPM for the current configuration
    double totalMass = mRigidBody.GetMassProperties().mass;
    double hoverThrustPerRotor = (totalMass * GRAVITY) / mRotors.size();

    if (!mRotors.empty())
    {
        const RotorConfig& cfg = mRotors[0].GetConfig();
        mHoverRPM = RotorModel::CalculateRPMForThrust(
            hoverThrustPerRotor, cfg.diameter, cfg.thrustCoeff, AIR_DENSITY_SL);

        std::cout << "[ROTORCRAFT_6DOF] ===== MASS AND HOVER CONFIGURATION =====" << std::endl;
        std::cout << "[ROTORCRAFT_6DOF] Total Mass: " << totalMass << " kg" << std::endl;
        std::cout << "[ROTORCRAFT_6DOF] Total Weight: " << (totalMass * GRAVITY) << " N" << std::endl;
        std::cout << "[ROTORCRAFT_6DOF] Thrust per rotor: " << hoverThrustPerRotor << " N" << std::endl;
        std::cout << "[ROTORCRAFT_6DOF] Hover RPM: " << mHoverRPM << " RPM" << std::endl;
        std::cout << "[ROTORCRAFT_6DOF] Number of rotors: " << mRotors.size() << std::endl;

        // Pre-spin rotors to hover RPM so drone doesn't fall on first frame
        for (auto& rotor : mRotors)
        {
            rotor.GetState().actualRPM = mHoverRPM;
            rotor.GetState().commandedRPM = mHoverRPM;
            // Also compute initial thrust so we have lift on first frame
            rotor.GetState().thrust = RotorModel::CalculateThrust(
                mHoverRPM, cfg.diameter, cfg.thrustCoeff, AIR_DENSITY_SL);
        }

        // Calculate and display initial hover power
        double hoverPowerPerRotor = RotorModel::CalculatePower(
            RotorModel::CalculateTorque(mHoverRPM, cfg.diameter, cfg.torqueCoeff, AIR_DENSITY_SL),
            mHoverRPM);
        double totalHoverPower = hoverPowerPerRotor * mRotors.size();
        std::cout << "[ROTORCRAFT_6DOF] Hover power per rotor: " << (hoverPowerPerRotor/1000.0) << " kW" << std::endl;
        std::cout << "[ROTORCRAFT_6DOF] Total hover power: " << (totalHoverPower/1000.0) << " kW" << std::endl;
        std::cout << "[ROTORCRAFT_6DOF] ===========================================" << std::endl;
    }

    // Set initial throttle to hover
    mThrottleCmd = 0.5;

    // Enable altitude and position hold by default
    mAltitudeHoldEnabled = true;
    mPositionHoldEnabled = true;

    // ========================================================================
    // Initialize spline navigation if enabled
    // ========================================================================
    if (mSplineEnabled)
    {
        InitializeWaypointCache();
        mNavState = SplineNavState::APPROACHING;
    }

    mLastSimTime = aSimTime;
    return true;
}

// ============================================================================
// WsfRouteMover Virtual Methods - Route Navigation
// ============================================================================

void Rotorcraft6DOF_Mover::SetGuidanceFrom(const WsfWaypoint& aPreviousWaypoint)
{
    // Extract speed from previous waypoint if specified
    double speed = aPreviousWaypoint.GetSpeed();
    if (speed > 0.0 && speed < 1000.0)
    {
        mTargetSpeed = speed;
    }
}

void Rotorcraft6DOF_Mover::SetGuidanceTo(const WsfWaypoint& aNextWaypoint)
{
    // Set target position from waypoint
    double lat = aNextWaypoint.GetLat();
    double lon = aNextWaypoint.GetLon();
    double alt = aNextWaypoint.GetAlt();
    double speed = aNextWaypoint.GetSpeed();

    // Handle "use previous" speed
    if (speed > 0.0 && speed < 1000.0)
    {
        mTargetSpeed = speed;
    }

    // Set our target position for the position hold controller
    SetTargetPosition(lat, lon, alt);
}

void Rotorcraft6DOF_Mover::AdvanceToNextWaypoint(double aSimTime,
                                                  const WsfWaypoint& aTarget,
                                                  const WsfWaypoint* aNextPointPtr)
{
    // Set guidance to target waypoint
    SetGuidanceTo(aTarget);

    // Reset rate-limited velocities for new waypoint
    mDesVelN = 0.0;
    mDesVelE = 0.0;
}

void Rotorcraft6DOF_Mover::Pause(double aSimTime, double aUnpauseTime)
{
    mIsPaused = true;
    mPauseEndTime = aUnpauseTime;
}

// ============================================================================
// UpdatePosition - Main physics update (called by WsfRouteMover::Update)
// ============================================================================

void Rotorcraft6DOF_Mover::UpdatePosition(double aSimTime)
{
    // Check pause state - unpause if time has passed
    if (mIsPaused && aSimTime >= mPauseEndTime)
    {
        mIsPaused = false;
    }

    // Calculate time step
    double dt = aSimTime - mLastSimTime;
    if (dt <= 0.0 || dt > 1.0)
    {
        dt = 0.01;  // Default to 100 Hz if invalid
    }

    // ========================================================================
    // Spline navigation or standard waypoint navigation
    // ========================================================================
    WsfPlatform* platform = GetPlatform();
    if (mSplineEnabled && mWaypointCache.size() >= 4 && !mIsPaused)
    {
        // Use spline navigation state machine
        UpdateSplineNavigation(dt, aSimTime);
    }
    else if (platform && mPositionHoldEnabled && !mIsPaused)
    {
        // Standard waypoint navigation - check if we've reached the current waypoint
        double currentLat, currentLon, currentAlt;
        platform->GetLocationLLA(currentLat, currentLon, currentAlt);

        // Calculate distance to target
        double errN, errE;
        GeodeticOffsetToNED(currentLat, currentLon,
                            mTargetLat, mTargetLon,
                            errN, errE);
        double horizDist = std::sqrt(errN * errN + errE * errE);
        double vertDist = std::abs(mTargetAltMSL - currentAlt);
        double totalDist = std::sqrt(horizDist * horizDist + vertDist * vertDist);

        // Check if we've arrived at waypoint
        if (totalDist < mWaypointArrivalRadius)
        {
            // Notify parent class that we reached the waypoint
            WaypointReached(aSimTime);
        }
    }

    // ========================================================================
    // Update controllers
    // ========================================================================

    if (mAltitudeHoldEnabled)
    {
        UpdateAltitudeHold(dt);
    }

    if (mPositionHoldEnabled)
    {
        UpdatePositionHold(dt);
    }

    // ========================================================================
    // Update rotor states (motor dynamics)
    // ========================================================================

    ApplyControlMixer();

    for (auto& rotor : mRotors)
    {
        rotor.Update(dt, mCurrentAirDensity);
    }

    // ========================================================================
    // Update dynamic mass (from weapon ammo, payload changes, etc.)
    // ========================================================================
    UpdateDynamicMass();

    // ========================================================================
    // Compute forces and moments
    // ========================================================================

    // Rotor thrust (body frame, -Z is up)
    Vector3 thrustForce = ComputeTotalThrustForce();

    // Rotor moments (body frame)
    Vector3 rotorMoment = ComputeTotalRotorMoment();

    // Aerodynamic drag (body frame)
    Vector3 velocityBody = mRigidBody.GetState().GetBodyVelocity();
    Vector3 dragForce = ComputeDragForce(velocityBody);

    // External forces from other processors (slung load, weapons, etc.)
    Vector3 externalForce = ReadExternalForces();

    // External moments from other processors (weapon recoil, etc.)
    Vector3 externalMoment = ReadExternalMoments();

    // Total force (body frame) - includes thrust, drag, and external forces
    Vector3 totalForce = thrustForce + dragForce + externalForce;

    // Total moment (body frame) - includes rotor torque and external moments
    Vector3 totalMoment = rotorMoment + externalMoment;

    // ========================================================================
    // Integrate 6DOF equations of motion
    // ========================================================================

    mRigidBody.Integrate(totalForce, totalMoment, dt);

    // ========================================================================
    // Apply ground constraint (prevent going underground)
    // ========================================================================

    RigidBodyState& state = mRigidBody.GetState();
    double altMSL = -state.position.z + mOriginAlt;

    if (altMSL < 0.0)
    {
        // Clamp position to ground (sea level = 0 MSL)
        state.position.z = mOriginAlt;

        // Zero out downward velocity
        if (state.velocity.z > 0.0)  // NED: positive z is down
        {
            state.velocity.z = 0.0;
        }

        // Strongly damp horizontal velocity on ground
        state.velocity.x *= 0.8;
        state.velocity.y *= 0.8;

        // Strongly damp angular rates on ground
        state.angularVelocity *= 0.5;

        // Push attitude toward level (prevent tipping over on ground)
        Quaternion levelingCorrection = Quaternion::FromEuler(
            -state.roll * 0.1,   // Counter-roll
            -state.pitch * 0.1,  // Counter-pitch
            0.0                  // Preserve yaw
        );
        state.orientation = (state.orientation * levelingCorrection).Normalized();
        state.UpdateEulerAngles();
    }

    // ========================================================================
    // Sync state back to AFSIM platform
    // ========================================================================

    SyncToAFSIM();

    // ========================================================================
    // Publish telemetry via AuxData
    // ========================================================================

    if (mTelemetryEnabled)
    {
        PublishTelemetry();
    }

    // Update timing
    mLastSimTime = aSimTime;
}

// ============================================================================
// IsStopped
// ============================================================================

bool Rotorcraft6DOF_Mover::IsStopped() const
{
    const RigidBodyState& state = mRigidBody.GetState();
    return state.velocity.MagnitudeSquared() < 0.01;  // < 0.1 m/s
}

// ============================================================================
// Get total thrust
// ============================================================================

double Rotorcraft6DOF_Mover::GetTotalThrust() const
{
    double total = 0.0;
    for (const auto& rotor : mRotors)
    {
        total += rotor.GetState().thrust;
    }
    return total;
}

// ============================================================================
// Get total power
// ============================================================================

double Rotorcraft6DOF_Mover::GetTotalPower() const
{
    double total = 0.0;
    for (const auto& rotor : mRotors)
    {
        total += rotor.GetState().power;
    }
    return total;
}

// ============================================================================
// Set throttle command
// ============================================================================

void Rotorcraft6DOF_Mover::SetThrottleCommand(double throttle)
{
    mThrottleCmd = Clamp(throttle, 0.0, 1.0);
}

void Rotorcraft6DOF_Mover::SetRollCommand(double roll)
{
    mRollCmd = Clamp(roll, -1.0, 1.0);
}

void Rotorcraft6DOF_Mover::SetPitchCommand(double pitch)
{
    mPitchCmd = Clamp(pitch, -1.0, 1.0);
}

void Rotorcraft6DOF_Mover::SetYawCommand(double yaw)
{
    mYawCmd = Clamp(yaw, -1.0, 1.0);
}

void Rotorcraft6DOF_Mover::SetTargetAltitude(double altMSL)
{
    // Reset integral if target changes significantly
    if (std::abs(altMSL - mTargetAltMSL) > 5.0)
    {
        mAltitudeIntegral = 0.0;
    }
    mTargetAltMSL = altMSL;
    mAltitudeHoldEnabled = true;
}

void Rotorcraft6DOF_Mover::SetTargetPosition(double lat, double lon, double altMSL)
{
    // Check if this is primarily a vertical move (same lat/lon)
    constexpr double COORD_TOLERANCE = 0.00001;  // ~1m tolerance
    bool sameHorizontalPos = (std::abs(lat - mTargetLat) < COORD_TOLERANCE &&
                               std::abs(lon - mTargetLon) < COORD_TOLERANCE);

    // If target is changing significantly, reset rate-limited velocities
    if (!sameHorizontalPos)
    {
        mDesVelN = 0.0;
        mDesVelE = 0.0;
    }

    // Reset altitude integral if altitude target changes significantly
    if (std::abs(altMSL - mTargetAltMSL) > 5.0)
    {
        mAltitudeIntegral = 0.0;
    }

    mTargetLat = lat;
    mTargetLon = lon;
    mTargetAltMSL = altMSL;
    mPositionHoldEnabled = true;
    mAltitudeHoldEnabled = true;
}

// ============================================================================
// Set rotor configuration type
// ============================================================================

void Rotorcraft6DOF_Mover::SetRotorConfigType(RotorConfigType type)
{
    mConfigType = type;
    mRotors.clear();

    switch (type)
    {
        case RotorConfigType::QUAD_X:
            SetupQuadX(mDefaultArmLength, mDefaultRotorDiameter);
            break;
        case RotorConfigType::HEX_X:
            SetupHexX(mDefaultArmLength, mDefaultRotorDiameter);
            break;
        case RotorConfigType::OCTO_X:
            SetupOctoX(mDefaultArmLength, mDefaultRotorDiameter);
            break;
        case RotorConfigType::COAXIAL_QUAD:
            SetupCoaxialQuad(mDefaultArmLength, mDefaultRotorDiameter, mDefaultCoaxialSpacing);
            break;
        case RotorConfigType::COAXIAL_OCTO:
            SetupCoaxialOcto(mDefaultArmLength, mDefaultRotorDiameter, mDefaultCoaxialSpacing);
            break;
        default:
            // Default to quad-X
            SetupQuadX(mDefaultArmLength, mDefaultRotorDiameter);
            break;
    }
}

// ============================================================================
// Compute ground effect factor
// ============================================================================

double Rotorcraft6DOF_Mover::ComputeGroundEffectFactor(double heightAGL) const
{
    if (!mGroundEffectConfig.enabled)
        return 1.0;

    // Ground effect increases thrust when close to ground
    // Based on empirical models: GE factor = 1 / (1 - (R/4h)^2)
    // where R is rotor radius, h is height above ground

    double rotorRadius = mDefaultRotorDiameter / 2.0;
    double geHeight = mGroundEffectConfig.heightFactor * rotorRadius;

    if (heightAGL >= geHeight)
        return 1.0;

    if (heightAGL < 0.1)
        heightAGL = 0.1;  // Prevent division by zero

    double ratio = rotorRadius / (4.0 * heightAGL);
    double factor = 1.0 / (1.0 - ratio * ratio);

    // Clamp to maximum boost
    return std::min(factor, mGroundEffectConfig.maxBoost);
}

// ============================================================================
// Compute aerodynamic drag force
// ============================================================================

Vector3 Rotorcraft6DOF_Mover::ComputeDragForce(const Vector3& velocityBody) const
{
    if (!mDragConfig.enabled)
        return Vector3::Zero();

    // Drag = 0.5 * rho * V^2 * Cd * A
    // Applied separately in each body axis

    double rho = mCurrentAirDensity;
    double area = mDragConfig.frontalArea;

    double vx = velocityBody.x;
    double vy = velocityBody.y;
    double vz = velocityBody.z;

    // Drag opposes velocity in each axis
    double dragX = -0.5 * rho * vx * std::abs(vx) * mDragConfig.Cd_x * area;
    double dragY = -0.5 * rho * vy * std::abs(vy) * mDragConfig.Cd_y * area;
    double dragZ = -0.5 * rho * vz * std::abs(vz) * mDragConfig.Cd_z * area;

    return Vector3(dragX, dragY, dragZ);
}

// ============================================================================
// Compute total thrust force from all rotors
// ============================================================================

Vector3 Rotorcraft6DOF_Mover::ComputeTotalThrustForce() const
{
    // Thrust is purely in -Z direction (up) in body frame
    // Individual rotor positions don't affect total force, only moments
    double totalThrust = GetTotalThrust();

    // Apply ground effect
    double geFactor = ComputeGroundEffectFactor(mHeightAGL);
    totalThrust *= geFactor;

    // In NED body frame, thrust points in -Z direction (up)
    return Vector3(0.0, 0.0, -totalThrust);
}

// ============================================================================
// Compute total moment from all rotors
// ============================================================================

Vector3 Rotorcraft6DOF_Mover::ComputeTotalRotorMoment() const
{
    Vector3 totalMoment = Vector3::Zero();

    for (const auto& rotor : mRotors)
    {
        const RotorConfig& cfg = rotor.GetConfig();
        const RotorState& state = rotor.GetState();

        // Thrust creates moment around position
        // M = r × F, where r is position and F is thrust in -Z
        double thrust = state.thrust;
        double x = cfg.position.x;
        double y = cfg.position.y;

        // Moment from thrust: r × (0, 0, -thrust)
        // = (y * (-thrust) - 0, 0 - x * (-thrust), 0)
        // = (-y * thrust, x * thrust, 0)
        totalMoment.x += -y * thrust;  // Roll moment
        totalMoment.y += x * thrust;   // Pitch moment

        // Torque reaction from rotor (yaw moment)
        // CW rotors create CCW reaction (positive yaw moment in NED)
        // CCW rotors create CW reaction (negative yaw moment)
        double torqueReaction = state.torque;
        if (cfg.direction == RotorDirection::CCW)  // CCW rotor
        {
            totalMoment.z -= torqueReaction;
        }
        else  // CW rotor
        {
            totalMoment.z += torqueReaction;
        }
    }

    return totalMoment;
}

// ============================================================================
// Apply control mixer - Generate rotor commands from control inputs
// ============================================================================

void Rotorcraft6DOF_Mover::ApplyControlMixer()
{
    // Calculate base RPM from throttle command
    // Throttle 0.5 = hover, scales linearly
    double throttleFraction = mThrottleCmd * 2.0;  // 0-2 range, 1.0 = hover
    double baseRPM = mHoverRPM * std::sqrt(throttleFraction);

    // Clamp base RPM
    baseRPM = Clamp(baseRPM, 0.0, mDefaultMaxRPM);

    // ============================================================
    // INNER ATTITUDE STABILIZATION LOOP (PD controller)
    // ============================================================
    const RigidBodyState& state = mRigidBody.GetState();

    // Convert normalized commands to desired attitude angles
    double desiredRoll = mRollCmd * mMaxTiltAngle;    // radians
    double desiredPitch = mPitchCmd * mMaxTiltAngle;  // radians
    double desiredYawRate = mYawCmd * 0.5;            // rad/s

    // Attitude errors (desired - actual)
    double rollError = desiredRoll - state.roll;
    double pitchError = desiredPitch - state.pitch;
    double yawRateError = desiredYawRate - state.angularVelocity.z;

    // PD gains for attitude stabilization
    double attitudeKp = 8.0;   // Proportional gain on attitude error
    double attitudeKd = 2.0;   // Derivative gain (rate damping)
    double yawKp = 3.0;        // Yaw rate proportional gain

    // PD control outputs
    double stabilizedRollCmd = attitudeKp * rollError - attitudeKd * state.angularVelocity.x;
    double stabilizedPitchCmd = attitudeKp * pitchError - attitudeKd * state.angularVelocity.y;
    double stabilizedYawCmd = yawKp * yawRateError;

    // Clamp stabilized commands
    stabilizedRollCmd = Clamp(stabilizedRollCmd, -4.0, 4.0);
    stabilizedPitchCmd = Clamp(stabilizedPitchCmd, -4.0, 4.0);
    stabilizedYawCmd = Clamp(stabilizedYawCmd, -2.0, 2.0);

    // Apply mixer to each rotor
    for (auto& rotor : mRotors)
    {
        const RotorConfig& cfg = rotor.GetConfig();

        // Differential thrust for roll/pitch/yaw
        double rollDelta = stabilizedRollCmd * mRollMixGain * cfg.rollMix;
        double pitchDelta = stabilizedPitchCmd * mPitchMixGain * cfg.pitchMix;
        double yawDelta = stabilizedYawCmd * mYawMixGain * cfg.yawMix;

        // Total RPM for this rotor
        double rotorRPM = baseRPM + (rollDelta + pitchDelta + yawDelta) * mHoverRPM;

        // Clamp to valid range
        rotorRPM = Clamp(rotorRPM, 0.0, mDefaultMaxRPM);

        // Command rotor
        rotor.SetCommandedRPM(rotorRPM);
    }
}

// ============================================================================
// Altitude Hold Controller
// ============================================================================

void Rotorcraft6DOF_Mover::UpdateAltitudeHold(double dt)
{
    // Get current altitude from 6DOF state
    const RigidBodyState& state = mRigidBody.GetState();

    // Current altitude MSL = origin altitude - NED position Z
    double currentAlt = mOriginAlt - state.position.z;

    // Altitude error
    double altError = mTargetAltMSL - currentAlt;

    // Climb rate (NED: positive Z is down, so negate)
    double climbRate = -state.velocity.z;

    // Update integral with anti-windup
    // Only integrate when error is small (prevents windup during large maneuvers)
    if (std::abs(altError) < 10.0)
    {
        mAltitudeIntegral += altError * dt;
        // Clamp integral to prevent windup
        mAltitudeIntegral = Clamp(mAltitudeIntegral, -mAltitudeIntegralMax, mAltitudeIntegralMax);
    }
    else
    {
        // Decay integral when far from target to prevent windup
        mAltitudeIntegral *= 0.99;
    }

    // PID controller for altitude
    double pTerm = mAltitudeKp * altError;
    double iTerm = mAltitudeKi * mAltitudeIntegral;
    double dTerm = -mAltitudeKd * climbRate;
    double throttleAdj = pTerm + iTerm + dTerm;

    // Clamp throttle adjustment
    throttleAdj = Clamp(throttleAdj, -0.3, 0.3);

    // ========================================================================
    // CRITICAL FIX: Calculate hover throttle based on current mass
    // ========================================================================
    // The hover RPM (mHoverRPM) is set at initialization for the initial mass.
    // For the altitude controller to work correctly with different masses,
    // we need to calculate what throttle value produces hover for THIS mass.
    //
    // From ApplyControlMixer: baseRPM = mHoverRPM * sqrt(throttleFraction)
    // where throttleFraction = throttle * 2.0
    //
    // At initialization, throttle=0.5 produces hover for the initial mass.
    // If mass changes, we need to find the new hover throttle.
    //
    // Thrust needed = Weight = mass * g
    // The current mHoverRPM is calculated for the INITIAL mass, so if mass
    // has changed since initialization, we need to recalculate the hover
    // throttle that produces the required thrust.
    //
    // However, mass doesn't change during flight for this simulation (no fuel
    // consumption modeled in 6DOF), so we can use 0.5 as the reference.
    // If you add dynamic mass changes (fuel, payload drops), recalculate
    // mHoverRPM when mass changes.

    double hoverThrottle = 0.5;  // Assume mass is constant (set at init)
    mThrottleCmd = hoverThrottle + throttleAdj;
    mThrottleCmd = Clamp(mThrottleCmd, 0.0, 1.0);
}

// ============================================================================
// Position Hold Controller
// ============================================================================

void Rotorcraft6DOF_Mover::UpdatePositionHold(double dt)
{
    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return;

    // Get current position
    double currentLat, currentLon, currentAlt;
    platform->GetLocationLLA(currentLat, currentLon, currentAlt);

    // Calculate position error in NED frame
    double errN, errE;
    GeodeticOffsetToNED(currentLat, currentLon,
                        mTargetLat, mTargetLon,
                        errN, errE);

    // Deadband: ignore very small position errors (< 0.5m) to prevent drift
    const double deadband = 0.5;  // [m]
    if (std::abs(errN) < deadband) errN = 0.0;
    if (std::abs(errE) < deadband) errE = 0.0;

    // Get current velocity in NED
    const RigidBodyState& state = mRigidBody.GetState();
    double velN = state.velocity.x;  // NED velocity
    double velE = state.velocity.y;

    // ========================================================================
    // Cascaded Position Controller
    // ========================================================================
    // Outer loop: position error -> desired velocity
    double rawDesVelN = mPositionKp * errN;
    double rawDesVelE = mPositionKp * errE;

    // Limit desired velocity magnitude to target speed
    double desVelMag = std::sqrt(rawDesVelN * rawDesVelN + rawDesVelE * rawDesVelE);
    if (desVelMag > mTargetSpeed)
    {
        double scale = mTargetSpeed / desVelMag;
        rawDesVelN *= scale;
        rawDesVelE *= scale;
    }

    // Rate-limit desired velocity changes to prevent sudden reversals
    double maxDeltaVel = mMaxVelAccel * dt;
    double deltaVelN = rawDesVelN - mDesVelN;
    double deltaVelE = rawDesVelE - mDesVelE;

    deltaVelN = Clamp(deltaVelN, -maxDeltaVel, maxDeltaVel);
    deltaVelE = Clamp(deltaVelE, -maxDeltaVel, maxDeltaVel);

    mDesVelN += deltaVelN;
    mDesVelE += deltaVelE;

    // Velocity error
    double velErrN = mDesVelN - velN;
    double velErrE = mDesVelE - velE;

    // Velocity damping (derivative term on position)
    double dampN = -mPositionKd * velN;
    double dampE = -mPositionKd * velE;

    // Inner loop: velocity error -> tilt command (NED frame)
    double tiltCmdN = mVelocityKp * velErrN + dampN;
    double tiltCmdE = mVelocityKp * velErrE + dampE;

    // ========================================================================
    // Transform NED tilt commands to body frame based on current heading
    // ========================================================================
    // This is critical: pitch/roll are body-frame commands, but our position
    // controller computes desired accelerations in NED frame. We must rotate
    // the commands to account for the current yaw angle.
    //
    // Body forward (pitch) aligns with yaw direction
    // Body right (roll) is 90° clockwise from yaw
    double yaw = state.yaw;
    double cosYaw = std::cos(yaw);
    double sinYaw = std::sin(yaw);

    // Rotate NED tilt commands to body frame
    double tiltCmdForward = tiltCmdN * cosYaw + tiltCmdE * sinYaw;
    double tiltCmdRight = -tiltCmdN * sinYaw + tiltCmdE * cosYaw;

    // Convert to pitch/roll commands
    // Pitch down (negative) to accelerate forward
    // Roll right (positive) to accelerate right
    double pitchCmd = -tiltCmdForward;
    double rollCmd = tiltCmdRight;

    // Clamp tilt commands
    double maxTiltNorm = 1.0;  // Normalized -1 to 1
    pitchCmd = Clamp(pitchCmd, -maxTiltNorm, maxTiltNorm);
    rollCmd = Clamp(rollCmd, -maxTiltNorm, maxTiltNorm);

    // Set attitude commands
    mPitchCmd = pitchCmd;
    mRollCmd = rollCmd;

    // ========================================================================
    // Heading Control: Face direction of travel
    // ========================================================================
    if (mHeadingHoldEnabled)
    {
        // Only control heading when moving above a minimum speed
        double speed = std::sqrt(mDesVelN * mDesVelN + mDesVelE * mDesVelE);
        const double minSpeedForHeading = 0.5;  // [m/s]

        if (speed > minSpeedForHeading)
        {
            // Desired heading from velocity direction
            // atan2(E, N) gives heading from North, positive clockwise
            double desiredYaw = std::atan2(mDesVelE, mDesVelN);

            // Current heading
            double currentYaw = state.yaw;

            // Heading error with proper angle wrapping
            double yawError = desiredYaw - currentYaw;

            // Wrap to [-PI, PI]
            while (yawError > PI) yawError -= 2.0 * PI;
            while (yawError < -PI) yawError += 2.0 * PI;

            // Proportional yaw command
            mYawCmd = mHeadingKp * yawError;

            // Clamp yaw command
            mYawCmd = Clamp(mYawCmd, -1.0, 1.0);
        }
        else
        {
            // When nearly stopped, hold current heading
            mYawCmd = 0.0;
        }
    }
    else
    {
        mYawCmd = 0.0;
    }
}

// ============================================================================
// Publish telemetry via AuxData
// ============================================================================

void Rotorcraft6DOF_Mover::PublishTelemetry()
{
    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return;

    const RigidBodyState& state = mRigidBody.GetState();
    WsfAttributeContainer& auxData = platform->GetAuxData();

    // Position
    auxData.Assign("6dof_pos_n", state.position.x);
    auxData.Assign("6dof_pos_e", state.position.y);
    auxData.Assign("6dof_pos_d", state.position.z);

    // Velocity
    auxData.Assign("6dof_vel_n", state.velocity.x);
    auxData.Assign("6dof_vel_e", state.velocity.y);
    auxData.Assign("6dof_vel_d", state.velocity.z);

    // Attitude (degrees)
    auxData.Assign("6dof_roll_deg", state.roll * RAD_TO_DEG);
    auxData.Assign("6dof_pitch_deg", state.pitch * RAD_TO_DEG);
    auxData.Assign("6dof_yaw_deg", state.yaw * RAD_TO_DEG);

    // Angular rates (deg/s)
    auxData.Assign("6dof_p_dps", state.angularVelocity.x * RAD_TO_DEG);
    auxData.Assign("6dof_q_dps", state.angularVelocity.y * RAD_TO_DEG);
    auxData.Assign("6dof_r_dps", state.angularVelocity.z * RAD_TO_DEG);

    // Rotor data
    double totalThrust = GetTotalThrust();
    double totalPower = GetTotalPower();
    auxData.Assign("6dof_total_thrust", totalThrust);
    auxData.Assign("6dof_total_power", totalPower);
    auxData.Assign("6dof_throttle_cmd", mThrottleCmd);

    // Average RPM
    double avgRPM = 0.0;
    if (!mRotors.empty())
    {
        for (const auto& rotor : mRotors)
        {
            avgRPM += rotor.GetState().actualRPM;
        }
        avgRPM /= mRotors.size();
    }
    auxData.Assign("6dof_avg_rpm", avgRPM);

    // Mass and weight
    double mass = mRigidBody.GetMassProperties().mass;
    double weight = mass * GRAVITY;
    auxData.Assign("6dof_mass", mass);
    auxData.Assign("6dof_weight", weight);

    // Target position
    auxData.Assign("6dof_target_alt", mTargetAltMSL);

    // Periodic diagnostic output during cruise (every 30 seconds)
    static double lastDiagTime = 0.0;
    static int diagCounter = 0;
    diagCounter++;
    if (diagCounter % 3000 == 0)  // Every 3000 updates (~30 seconds at 100Hz)
    {
        double simTime = diagCounter * 0.01;  // Approximate time
        lastDiagTime = simTime;
        double altMSL = mOriginAlt - state.position.z;
        double velHoriz = std::sqrt(state.velocity.x * state.velocity.x + state.velocity.y * state.velocity.y);

        std::cout << "[6DOF DIAG] T=" << simTime << "s | "
                  << "Mass=" << mass << "kg | "
                  << "Alt=" << altMSL << "m | "
                  << "Vel=" << velHoriz << "m/s | "
                  << "Tilt=" << (state.pitch * RAD_TO_DEG) << "deg | "
                  << "Throttle=" << mThrottleCmd << " | "
                  << "RPM=" << avgRPM << " | "
                  << "Thrust=" << (totalThrust/1000.0) << "kN | "
                  << "Power=" << (totalPower/1000.0) << "kW" << std::endl;
    }
}

// ============================================================================
// Sync 6DOF state to AFSIM platform
// ============================================================================

void Rotorcraft6DOF_Mover::SyncToAFSIM()
{
    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return;

    const RigidBodyState& state = mRigidBody.GetState();

    // Convert local NED position back to geodetic
    // Local position is relative to origin
    double lat, lon;
    NEDOffsetToGeodetic(mOriginLat, mOriginLon,
                        state.position.x, state.position.y,
                        lat, lon);

    // Altitude MSL = origin altitude - NED down position
    double alt = mOriginAlt - state.position.z;

    // Set platform position
    platform->SetLocationLLA(lat, lon, alt);

    // Set platform velocity (NED)
    double velNED[3] = {state.velocity.x, state.velocity.y, state.velocity.z};
    platform->SetVelocityNED(velNED);

    // Set platform orientation
    // Note: AFSIM uses heading (yaw), pitch, roll order
    platform->SetOrientationNED(state.yaw, state.pitch, state.roll);
}

// ============================================================================
// Initialize 6DOF state from AFSIM platform
// ============================================================================

void Rotorcraft6DOF_Mover::InitFromAFSIM()
{
    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return;

    RigidBodyState& state = mRigidBody.GetState();

    // Get current LLA
    double lat, lon, alt;
    platform->GetLocationLLA(lat, lon, alt);

    // Set origin to current position
    mOriginLat = lat;
    mOriginLon = lon;
    mOriginAlt = alt;
    mOriginSet = true;

    // Local NED position is (0, 0, 0) at origin
    state.position = Vector3::Zero();

    // Get velocity using array-based API
    double velocityNED[3] = {0.0, 0.0, 0.0};
    platform->GetVelocityNED(velocityNED);
    state.velocity = Vector3(velocityNED[0], velocityNED[1], velocityNED[2]);

    // Get orientation
    double heading, pitch, roll;
    platform->GetOrientationNED(heading, pitch, roll);
    state.yaw = heading;
    state.pitch = pitch;
    state.roll = roll;
    state.orientation = Quaternion::FromEuler(state.roll, state.pitch, state.yaw);

    // Zero angular velocity initially
    state.angularVelocity = Vector3::Zero();

    // NOTE: Don't reset target position here - it's already set by route navigation
    // via SetGuidanceTo() which is called before InitFromAFSIM()
}

// ============================================================================
// Setup quad-X configuration
// ============================================================================

void Rotorcraft6DOF_Mover::SetupQuadX(double armLength, double rotorDiameter)
{
    mRotors.clear();

    double diagOffset = armLength * std::cos(45.0 * DEG_TO_RAD);

    // Motor 1: Front-Right (+X, +Y) - CCW
    RotorConfig cfg1;
    cfg1.position = Vector3(diagOffset, diagOffset, 0.0);
    cfg1.direction = RotorDirection::CCW;
    cfg1.diameter = rotorDiameter;
    cfg1.thrustCoeff = mDefaultThrustCoeff;
    cfg1.torqueCoeff = mDefaultTorqueCoeff;
    cfg1.maxRPM = mDefaultMaxRPM;
    cfg1.timeConstant = mDefaultMotorTimeConstant;
    cfg1.rollMix = -1.0;   // Right side: negative roll
    cfg1.pitchMix = 1.0;   // Front: positive pitch
    cfg1.yawMix = 1.0;     // CCW: positive yaw
    mRotors.push_back(RotorModel(cfg1));

    // Motor 2: Back-Right (-X, +Y) - CW
    RotorConfig cfg2;
    cfg2.position = Vector3(-diagOffset, diagOffset, 0.0);
    cfg2.direction = RotorDirection::CW;
    cfg2.diameter = rotorDiameter;
    cfg2.thrustCoeff = mDefaultThrustCoeff;
    cfg2.torqueCoeff = mDefaultTorqueCoeff;
    cfg2.maxRPM = mDefaultMaxRPM;
    cfg2.timeConstant = mDefaultMotorTimeConstant;
    cfg2.rollMix = -1.0;
    cfg2.pitchMix = -1.0;  // Back: negative pitch
    cfg2.yawMix = -1.0;    // CW: negative yaw
    mRotors.push_back(RotorModel(cfg2));

    // Motor 3: Back-Left (-X, -Y) - CCW
    RotorConfig cfg3;
    cfg3.position = Vector3(-diagOffset, -diagOffset, 0.0);
    cfg3.direction = RotorDirection::CCW;
    cfg3.diameter = rotorDiameter;
    cfg3.thrustCoeff = mDefaultThrustCoeff;
    cfg3.torqueCoeff = mDefaultTorqueCoeff;
    cfg3.maxRPM = mDefaultMaxRPM;
    cfg3.timeConstant = mDefaultMotorTimeConstant;
    cfg3.rollMix = 1.0;    // Left side: positive roll
    cfg3.pitchMix = -1.0;
    cfg3.yawMix = 1.0;
    mRotors.push_back(RotorModel(cfg3));

    // Motor 4: Front-Left (+X, -Y) - CW
    RotorConfig cfg4;
    cfg4.position = Vector3(diagOffset, -diagOffset, 0.0);
    cfg4.direction = RotorDirection::CW;
    cfg4.diameter = rotorDiameter;
    cfg4.thrustCoeff = mDefaultThrustCoeff;
    cfg4.torqueCoeff = mDefaultTorqueCoeff;
    cfg4.maxRPM = mDefaultMaxRPM;
    cfg4.timeConstant = mDefaultMotorTimeConstant;
    cfg4.rollMix = 1.0;
    cfg4.pitchMix = 1.0;
    cfg4.yawMix = -1.0;
    mRotors.push_back(RotorModel(cfg4));
}

// ============================================================================
// Setup hex-X configuration
// ============================================================================

void Rotorcraft6DOF_Mover::SetupHexX(double armLength, double rotorDiameter)
{
    mRotors.clear();

    // 6 motors at 60 degree intervals
    for (int i = 0; i < 6; ++i)
    {
        double angle = i * 60.0 * DEG_TO_RAD;
        double posX = armLength * std::cos(angle);
        double posY = armLength * std::sin(angle);

        RotorConfig cfg;
        cfg.position = Vector3(posX, posY, 0.0);
        cfg.direction = (i % 2 == 0) ? RotorDirection::CCW : RotorDirection::CW;
        cfg.diameter = rotorDiameter;
        cfg.thrustCoeff = mDefaultThrustCoeff;
        cfg.torqueCoeff = mDefaultTorqueCoeff;
        cfg.maxRPM = mDefaultMaxRPM;
        cfg.timeConstant = mDefaultMotorTimeConstant;

        // Mixing based on position
        cfg.rollMix = -posY / armLength;
        cfg.pitchMix = posX / armLength;
        cfg.yawMix = (cfg.direction == RotorDirection::CCW) ? 1.0 : -1.0;

        mRotors.push_back(RotorModel(cfg));
    }
}

// ============================================================================
// Setup octo-X configuration
// ============================================================================

void Rotorcraft6DOF_Mover::SetupOctoX(double armLength, double rotorDiameter)
{
    mRotors.clear();

    // 8 motors at 45 degree intervals
    for (int i = 0; i < 8; ++i)
    {
        double angle = i * 45.0 * DEG_TO_RAD;
        double posX = armLength * std::cos(angle);
        double posY = armLength * std::sin(angle);

        RotorConfig cfg;
        cfg.position = Vector3(posX, posY, 0.0);
        cfg.direction = (i % 2 == 0) ? RotorDirection::CCW : RotorDirection::CW;
        cfg.diameter = rotorDiameter;
        cfg.thrustCoeff = mDefaultThrustCoeff;
        cfg.torqueCoeff = mDefaultTorqueCoeff;
        cfg.maxRPM = mDefaultMaxRPM;
        cfg.timeConstant = mDefaultMotorTimeConstant;

        cfg.rollMix = -posY / armLength;
        cfg.pitchMix = posX / armLength;
        cfg.yawMix = (cfg.direction == RotorDirection::CCW) ? 1.0 : -1.0;

        mRotors.push_back(RotorModel(cfg));
    }
}

// ============================================================================
// Setup coaxial quad configuration (T-150 style: 4 arms, 2 rotors per arm = 8 motors)
// ============================================================================

void Rotorcraft6DOF_Mover::SetupCoaxialQuad(double armLength, double rotorDiameter, double coaxialSpacing)
{
    mRotors.clear();

    // 4 arms at 45, 135, 225, 315 degrees
    // Each arm has 2 counter-rotating rotors (upper and lower)
    double armAngles[4] = {45.0, 135.0, 225.0, 315.0};

    for (int arm = 0; arm < 4; ++arm)
    {
        double angle = armAngles[arm] * DEG_TO_RAD;
        double x = armLength * std::cos(angle);
        double y = armLength * std::sin(angle);

        // Upper rotor (CCW)
        RotorConfig cfgUpper;
        cfgUpper.position = Vector3(x, y, -coaxialSpacing / 2.0);  // Above CG (negative Z in NED)
        cfgUpper.direction = RotorDirection::CCW;
        cfgUpper.diameter = rotorDiameter;
        cfgUpper.thrustCoeff = mDefaultThrustCoeff;
        cfgUpper.torqueCoeff = mDefaultTorqueCoeff;
        cfgUpper.maxRPM = mDefaultMaxRPM;
        cfgUpper.timeConstant = mDefaultMotorTimeConstant;
        cfgUpper.rollMix = -y / armLength;
        cfgUpper.pitchMix = x / armLength;
        cfgUpper.yawMix = 1.0;  // CCW
        mRotors.push_back(RotorModel(cfgUpper));

        // Lower rotor (CW)
        RotorConfig cfgLower;
        cfgLower.position = Vector3(x, y, coaxialSpacing / 2.0);  // Below CG (positive Z in NED)
        cfgLower.direction = RotorDirection::CW;
        cfgLower.diameter = rotorDiameter;
        cfgLower.thrustCoeff = mDefaultThrustCoeff;
        cfgLower.torqueCoeff = mDefaultTorqueCoeff;
        cfgLower.maxRPM = mDefaultMaxRPM;
        cfgLower.timeConstant = mDefaultMotorTimeConstant;
        cfgLower.rollMix = -y / armLength;
        cfgLower.pitchMix = x / armLength;
        cfgLower.yawMix = -1.0;  // CW
        mRotors.push_back(RotorModel(cfgLower));
    }
}

// ============================================================================
// Read External Forces from AuxData
// ============================================================================
// Priority 1 Enhancement: Multi-weapon support via weapon registry
// Iterates over all registered weapons and sums their forces.
// Falls back to legacy single-channel if registry not present.
// ============================================================================

Vector3 Rotorcraft6DOF_Mover::ReadExternalForces()
{
    Vector3 totalExternal = Vector3::Zero();

    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return totalExternal;

    WsfAttributeContainer& auxData = platform->GetAuxData();

    // ========================================================================
    // Slung Load Forces
    // ========================================================================
    // The slung load processor publishes forces in body frame
    // These are the reaction forces on the aircraft from the cable tension
    if (auxData.AttributeExists("SLUNG_FORCE_X"))
        totalExternal.x += auxData.GetDouble("SLUNG_FORCE_X");
    if (auxData.AttributeExists("SLUNG_FORCE_Y"))
        totalExternal.y += auxData.GetDouble("SLUNG_FORCE_Y");
    if (auxData.AttributeExists("SLUNG_FORCE_Z"))
        totalExternal.z += auxData.GetDouble("SLUNG_FORCE_Z");

    // ========================================================================
    // Weapon Recoil Forces (Priority 1 Enhancement: Multi-Weapon Support)
    // ========================================================================
    // Check for weapon registry - if present, iterate over registered weapons
    // Otherwise fall back to legacy single-channel WEAPON_FORCE_X/Y/Z
    // ========================================================================
    if (auxData.AttributeExists("WEAPON_REGISTRY"))
    {
        // Parse comma-separated list of weapon names
        std::string registry = auxData.GetString("WEAPON_REGISTRY");
        std::istringstream ss(registry);
        std::string weaponName;

        while (std::getline(ss, weaponName, ','))
        {
            // Trim whitespace
            size_t start = weaponName.find_first_not_of(" \t");
            size_t end = weaponName.find_last_not_of(" \t");
            if (start == std::string::npos)
                continue;
            weaponName = weaponName.substr(start, end - start + 1);

            // Build indexed attribute names
            std::string prefix = "WEAPON_" + weaponName + "_";

            // Read forces from this weapon
            if (auxData.AttributeExists(prefix + "FORCE_X"))
                totalExternal.x += auxData.GetDouble(prefix + "FORCE_X");
            if (auxData.AttributeExists(prefix + "FORCE_Y"))
                totalExternal.y += auxData.GetDouble(prefix + "FORCE_Y");
            if (auxData.AttributeExists(prefix + "FORCE_Z"))
                totalExternal.z += auxData.GetDouble(prefix + "FORCE_Z");
        }
    }
    else
    {
        // Legacy single-channel fallback
        if (auxData.AttributeExists("WEAPON_FORCE_X"))
            totalExternal.x += auxData.GetDouble("WEAPON_FORCE_X");
        if (auxData.AttributeExists("WEAPON_FORCE_Y"))
            totalExternal.y += auxData.GetDouble("WEAPON_FORCE_Y");
        if (auxData.AttributeExists("WEAPON_FORCE_Z"))
            totalExternal.z += auxData.GetDouble("WEAPON_FORCE_Z");
    }

    return totalExternal;
}

// ============================================================================
// Read External Moments from AuxData
// ============================================================================
// Priority 1 Enhancement: Multi-weapon support via weapon registry
// Iterates over all registered weapons and sums their moments.
// Falls back to legacy single-channel if registry not present.
// ============================================================================

Vector3 Rotorcraft6DOF_Mover::ReadExternalMoments()
{
    Vector3 totalExternal = Vector3::Zero();

    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return totalExternal;

    WsfAttributeContainer& auxData = platform->GetAuxData();

    // ========================================================================
    // Weapon Recoil Moments (Priority 1 Enhancement: Multi-Weapon Support)
    // ========================================================================
    // The weapon processor publishes recoil moments about the CG
    // These are caused by off-center weapon mounts: M = r x F
    // ========================================================================
    if (auxData.AttributeExists("WEAPON_REGISTRY"))
    {
        // Parse comma-separated list of weapon names
        std::string registry = auxData.GetString("WEAPON_REGISTRY");
        std::istringstream ss(registry);
        std::string weaponName;

        while (std::getline(ss, weaponName, ','))
        {
            // Trim whitespace
            size_t start = weaponName.find_first_not_of(" \t");
            size_t end = weaponName.find_last_not_of(" \t");
            if (start == std::string::npos)
                continue;
            weaponName = weaponName.substr(start, end - start + 1);

            // Build indexed attribute names
            std::string prefix = "WEAPON_" + weaponName + "_";

            // Read moments from this weapon
            if (auxData.AttributeExists(prefix + "MOMENT_X"))
                totalExternal.x += auxData.GetDouble(prefix + "MOMENT_X");
            if (auxData.AttributeExists(prefix + "MOMENT_Y"))
                totalExternal.y += auxData.GetDouble(prefix + "MOMENT_Y");
            if (auxData.AttributeExists(prefix + "MOMENT_Z"))
                totalExternal.z += auxData.GetDouble(prefix + "MOMENT_Z");
        }
    }
    else
    {
        // Legacy single-channel fallback
        if (auxData.AttributeExists("WEAPON_MOMENT_X"))
            totalExternal.x += auxData.GetDouble("WEAPON_MOMENT_X");
        if (auxData.AttributeExists("WEAPON_MOMENT_Y"))
            totalExternal.y += auxData.GetDouble("WEAPON_MOMENT_Y");
        if (auxData.AttributeExists("WEAPON_MOMENT_Z"))
            totalExternal.z += auxData.GetDouble("WEAPON_MOMENT_Z");
    }

    // ========================================================================
    // Slung Load Moments (if mount is off-center)
    // ========================================================================
    // Note: Currently slung load attach point is typically at CG (z-offset only)
    // so it doesn't create roll/pitch moments from cable forces.
    // If needed, we could add SLUNG_MOMENT_* in the future.

    return totalExternal;
}

// ============================================================================
// Update Dynamic Mass from External Processors
// ============================================================================
// Priority 1 Enhancement: Multi-weapon support via weapon registry
// Iterates over all registered weapons and sums their mass deltas.
// Falls back to legacy single-channel if registry not present.
// ============================================================================

void Rotorcraft6DOF_Mover::UpdateDynamicMass()
{
    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return;

    WsfAttributeContainer& auxData = platform->GetAuxData();

    // ========================================================================
    // Weapon Mass Changes (Priority 1 Enhancement: Multi-Weapon Support)
    // ========================================================================
    // The weapon processor tracks ammunition mass changes
    // Sum WEAPON_<name>_MASS_DELTA from all registered weapons
    // ========================================================================
    double weaponMassDelta = 0.0;

    if (auxData.AttributeExists("WEAPON_REGISTRY"))
    {
        // Parse comma-separated list of weapon names
        std::string registry = auxData.GetString("WEAPON_REGISTRY");
        std::istringstream ss(registry);
        std::string weaponName;

        while (std::getline(ss, weaponName, ','))
        {
            // Trim whitespace
            size_t start = weaponName.find_first_not_of(" \t");
            size_t end = weaponName.find_last_not_of(" \t");
            if (start == std::string::npos)
                continue;
            weaponName = weaponName.substr(start, end - start + 1);

            // Build indexed attribute name
            std::string massKey = "WEAPON_" + weaponName + "_MASS_DELTA";

            // Read mass delta from this weapon
            if (auxData.AttributeExists(massKey))
            {
                weaponMassDelta += auxData.GetDouble(massKey);
            }
        }
    }
    else if (auxData.AttributeExists("WEAPON_MASS_DELTA"))
    {
        // Legacy single-channel fallback
        weaponMassDelta = auxData.GetDouble("WEAPON_MASS_DELTA");
    }

    if (std::abs(weaponMassDelta) > 0.0001)  // Only update if significant change
    {
        MassProperties& mass = mRigidBody.GetMassProperties();
        double newMass = mass.mass + weaponMassDelta;

        // Clamp to reasonable minimum mass (don't let aircraft disappear!)
        const double minMass = 50.0;  // [kg] Minimum structural mass
        newMass = std::max(minMass, newMass);

        if (newMass != mass.mass)
        {
            double oldMass = mass.mass;
            mass.mass = newMass;

            // Recalculate hover RPM for new mass
            double hoverThrustPerRotor = (newMass * GRAVITY) / mRotors.size();
            if (!mRotors.empty())
            {
                const RotorConfig& cfg = mRotors[0].GetConfig();
                mHoverRPM = RotorModel::CalculateRPMForThrust(
                    hoverThrustPerRotor, cfg.diameter, cfg.thrustCoeff, AIR_DENSITY_SL);
            }

            // Log mass change
            std::cout << "[6DOF] Mass updated: " << oldMass << " -> " << newMass
                      << " kg (delta=" << weaponMassDelta << " kg)" << std::endl;
        }
    }

    // ========================================================================
    // Slung Load Mass (already included in initial mass if configured)
    // ========================================================================
    // The slung load mass is typically configured at scenario start and
    // doesn't change dynamically. If we implement load release, we would
    // need to handle SLUNG_LOAD_STATE == RELEASED and reduce mass.

    int slungLoadState = 0;
    if (auxData.AttributeExists("SLUNG_LOAD_STATE"))
    {
        slungLoadState = static_cast<int>(auxData.GetInt("SLUNG_LOAD_STATE"));
    }
    static int lastSlungLoadState = slungLoadState;

    // Check if slung load was just released (state 4 = RELEASED)
    if (slungLoadState == 4 && lastSlungLoadState != 4)
    {
        double slungLoadMass = 0.0;
        if (auxData.AttributeExists("SLUNG_LOAD_MASS"))
        {
            slungLoadMass = auxData.GetDouble("SLUNG_LOAD_MASS");
        }

        if (slungLoadMass > 0.0)
        {
            MassProperties& mass = mRigidBody.GetMassProperties();
            double oldMass = mass.mass;
            mass.mass -= slungLoadMass;
            mass.mass = std::max(50.0, mass.mass);  // Clamp to minimum

            // Recalculate hover RPM
            double hoverThrustPerRotor = (mass.mass * GRAVITY) / mRotors.size();
            if (!mRotors.empty())
            {
                const RotorConfig& cfg = mRotors[0].GetConfig();
                mHoverRPM = RotorModel::CalculateRPMForThrust(
                    hoverThrustPerRotor, cfg.diameter, cfg.thrustCoeff, AIR_DENSITY_SL);
            }

            std::cout << "[6DOF] Slung load released! Mass: " << oldMass
                      << " -> " << mass.mass << " kg" << std::endl;
        }
    }
    lastSlungLoadState = slungLoadState;
}

// ============================================================================
// Get waypoint configuration for a route index
// ============================================================================

WaypointConfig Rotorcraft6DOF_Mover::GetWaypointConfig(int routeIndex) const
{
    auto it = mWaypointConfigs.find(routeIndex);
    if (it != mWaypointConfigs.end())
    {
        return it->second;
    }

    // Return default configuration
    WaypointConfig defaultConfig;
    defaultConfig.behavior = mDefaultBehavior;
    defaultConfig.passSpeed = mDefaultPassSpeed;
    defaultConfig.hoverDuration = mDefaultHoverDuration;
    defaultConfig.departureHeading = -999.0;
    return defaultConfig;
}

// ============================================================================
// Get waypoint from route by index
// ============================================================================

bool Rotorcraft6DOF_Mover::GetRouteWaypoint(int index, CachedWaypoint& outWaypoint)
{
    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return false;

    // Get current route from the platform
    const WsfRoute* route = GetRoute();
    if (!route)
        return false;

    int numWaypoints = static_cast<int>(route->GetSize());
    if (index < 0 || index >= numWaypoints)
        return false;

    const WsfWaypoint& wp = route->GetWaypointAt(index);

    outWaypoint.lat = wp.GetLat();
    outWaypoint.lon = wp.GetLon();
    outWaypoint.alt = wp.GetAlt();
    outWaypoint.speed = wp.GetSpeed();
    if (outWaypoint.speed <= 0.0 || outWaypoint.speed > 100.0)
        outWaypoint.speed = mTargetSpeed;  // Use default if invalid
    outWaypoint.routeIndex = index;
    outWaypoint.config = GetWaypointConfig(index);
    outWaypoint.isPhantom = false;

    return true;
}

// ============================================================================
// Create phantom waypoint by extrapolation
// ============================================================================

CachedWaypoint Rotorcraft6DOF_Mover::CreatePhantomWaypoint(
    const CachedWaypoint& p1, const CachedWaypoint& p2, bool before)
{
    CachedWaypoint phantom;

    // Mirror p2 across p1 (if before) or p1 across p2 (if after)
    if (before)
    {
        // Phantom before p1: P0 = 2*P1 - P2
        phantom.lat = 2.0 * p1.lat - p2.lat;
        phantom.lon = 2.0 * p1.lon - p2.lon;
        phantom.alt = 2.0 * p1.alt - p2.alt;
        phantom.speed = p1.speed;
        phantom.routeIndex = -1;
    }
    else
    {
        // Phantom after p2: P3 = 2*P2 - P1
        phantom.lat = 2.0 * p2.lat - p1.lat;
        phantom.lon = 2.0 * p2.lon - p1.lon;
        phantom.alt = 2.0 * p2.alt - p1.alt;
        phantom.speed = p2.speed;
        phantom.routeIndex = -1;
    }

    phantom.config.behavior = WaypointBehavior::FLY_THROUGH;
    phantom.isPhantom = true;

    return phantom;
}

// ============================================================================
// Initialize waypoint cache with first 4 waypoints
// ============================================================================

void Rotorcraft6DOF_Mover::InitializeWaypointCache()
{
    mWaypointCache.clear();

    const WsfRoute* route = GetRoute();
    if (!route)
    {
        std::cout << "[ROTORCRAFT_6DOF] WARNING: No route for spline navigation" << std::endl;
        mSplineEnabled = false;
        return;
    }

    mTotalWaypoints = static_cast<int>(route->GetSize());
    if (mTotalWaypoints < 2)
    {
        std::cout << "[ROTORCRAFT_6DOF] WARNING: Need at least 2 waypoints for spline" << std::endl;
        mSplineEnabled = false;
        return;
    }

    mCurrentWaypointIndex = 0;

    // Get waypoints 0 and 1
    CachedWaypoint wp0, wp1, wp2, wp3;

    if (!GetRouteWaypoint(0, wp0))
    {
        mSplineEnabled = false;
        return;
    }

    if (!GetRouteWaypoint(1, wp1))
    {
        mSplineEnabled = false;
        return;
    }

    // Create phantom P0 before first waypoint
    CachedWaypoint phantomP0 = CreatePhantomWaypoint(wp0, wp1, true);

    // Get P2 or create phantom
    if (!GetRouteWaypoint(2, wp2))
    {
        // Only 2 waypoints, create phantom after
        wp2 = CreatePhantomWaypoint(wp0, wp1, false);
    }

    // Cache is: [P0(phantom), P1(wp0), P2(wp1), P3(wp2)]
    // Current segment is P1->P2 (from wp0 to wp1)
    mWaypointCache.push_back(phantomP0);
    mWaypointCache.push_back(wp0);
    mWaypointCache.push_back(wp1);
    mWaypointCache.push_back(wp2);

    // Initialize spline state
    mSplineParameter = 0.0;
    mDistanceAlongSegment = 0.0;

    // Calculate segment arc length
    mSegmentArcLength = CalculateSegmentArcLength();

    // Initialize speed profile
    mSegmentEntrySpeed = mWaypointCache[1].speed;
    mSegmentExitSpeed = mWaypointCache[2].speed;
    ComputeSpeedProfile();

    mNavState = SplineNavState::APPROACHING;

    std::cout << "[ROTORCRAFT_6DOF] Spline cache initialized with " << mTotalWaypoints
              << " waypoints, segment length=" << mSegmentArcLength << " m" << std::endl;
}

// ============================================================================
// Advance waypoint cache after reaching waypoint
// ============================================================================

void Rotorcraft6DOF_Mover::AdvanceWaypointCache()
{
    mCurrentWaypointIndex++;

    if (mCurrentWaypointIndex >= mTotalWaypoints - 1)
    {
        // Reached final waypoint, disable spline
        std::cout << "[ROTORCRAFT_6DOF] Reached final waypoint, spline navigation complete" << std::endl;
        return;
    }

    // Shift cache: [P0, P1, P2, P3] -> [P1, P2, P3, P4_new]
    mWaypointCache[0] = mWaypointCache[1];
    mWaypointCache[1] = mWaypointCache[2];
    mWaypointCache[2] = mWaypointCache[3];

    // Get next waypoint (P3 in new segment = mCurrentWaypointIndex + 2)
    CachedWaypoint nextWp;
    if (!GetRouteWaypoint(mCurrentWaypointIndex + 2, nextWp))
    {
        // No more waypoints, create phantom
        nextWp = CreatePhantomWaypoint(mWaypointCache[1], mWaypointCache[2], false);
    }
    mWaypointCache[3] = nextWp;

    // Reset spline state
    mSplineParameter = 0.0;
    mDistanceAlongSegment = 0.0;

    // Recalculate segment arc length
    mSegmentArcLength = CalculateSegmentArcLength();

    // Update speed profile
    mSegmentEntrySpeed = mWaypointCache[1].speed;
    mSegmentExitSpeed = mWaypointCache[2].speed;
    ComputeSpeedProfile();

    mNavState = SplineNavState::APPROACHING;

    std::cout << "[ROTORCRAFT_6DOF] Advanced to segment " << mCurrentWaypointIndex
              << " -> " << (mCurrentWaypointIndex + 1)
              << ", length=" << mSegmentArcLength << " m" << std::endl;
}

// ============================================================================
// Calculate arc length of current spline segment
// ============================================================================

double Rotorcraft6DOF_Mover::CalculateSegmentArcLength() const
{
    if (mWaypointCache.size() < 4)
        return 0.0;

    // Convert waypoints to local NED vectors for spline calculation
    const CachedWaypoint& p0 = mWaypointCache[0];
    const CachedWaypoint& p1 = mWaypointCache[1];
    const CachedWaypoint& p2 = mWaypointCache[2];
    const CachedWaypoint& p3 = mWaypointCache[3];

    // Use P1 as origin for local coordinates
    double n0, e0, n1, e1, n2, e2, n3, e3;
    GeodeticOffsetToNED(p1.lat, p1.lon, p0.lat, p0.lon, n0, e0);
    n1 = 0.0; e1 = 0.0;  // P1 is origin
    GeodeticOffsetToNED(p1.lat, p1.lon, p2.lat, p2.lon, n2, e2);
    GeodeticOffsetToNED(p1.lat, p1.lon, p3.lat, p3.lon, n3, e3);

    Vector3 v0(n0, e0, -(p0.alt - p1.alt));  // NED
    Vector3 v1(0, 0, 0);
    Vector3 v2(n2, e2, -(p2.alt - p1.alt));
    Vector3 v3(n3, e3, -(p3.alt - p1.alt));

    return CatmullRomArcLength(v0, v1, v2, v3, mSplineTension, 32);
}

// ============================================================================
// Compute spline position at parameter t
// ============================================================================

void Rotorcraft6DOF_Mover::ComputeSplinePosition(double t, double& outLat,
                                                   double& outLon, double& outAlt) const
{
    if (mWaypointCache.size() < 4)
    {
        // Fallback to P1
        outLat = mWaypointCache.size() > 1 ? mWaypointCache[1].lat : 0.0;
        outLon = mWaypointCache.size() > 1 ? mWaypointCache[1].lon : 0.0;
        outAlt = mWaypointCache.size() > 1 ? mWaypointCache[1].alt : 0.0;
        return;
    }

    const CachedWaypoint& p0 = mWaypointCache[0];
    const CachedWaypoint& p1 = mWaypointCache[1];
    const CachedWaypoint& p2 = mWaypointCache[2];
    const CachedWaypoint& p3 = mWaypointCache[3];

    // Convert to local NED with P1 as origin
    double n0, e0, n2, e2, n3, e3;
    GeodeticOffsetToNED(p1.lat, p1.lon, p0.lat, p0.lon, n0, e0);
    GeodeticOffsetToNED(p1.lat, p1.lon, p2.lat, p2.lon, n2, e2);
    GeodeticOffsetToNED(p1.lat, p1.lon, p3.lat, p3.lon, n3, e3);

    Vector3 v0(n0, e0, p0.alt - p1.alt);  // Altitude positive up for interpolation
    Vector3 v1(0, 0, 0);
    Vector3 v2(n2, e2, p2.alt - p1.alt);
    Vector3 v3(n3, e3, p3.alt - p1.alt);

    // Interpolate in local NED
    Vector3 localPos = CatmullRomPoint(v0, v1, v2, v3, t, mSplineTension);

    // Convert back to geodetic
    NEDOffsetToGeodetic(p1.lat, p1.lon, localPos.x, localPos.y, outLat, outLon);
    outAlt = p1.alt + localPos.z;
}

// ============================================================================
// Compute spline tangent at parameter t
// ============================================================================

Vector3 Rotorcraft6DOF_Mover::ComputeSplineTangent(double t) const
{
    if (mWaypointCache.size() < 4)
        return Vector3(1, 0, 0);  // Default forward

    const CachedWaypoint& p0 = mWaypointCache[0];
    const CachedWaypoint& p1 = mWaypointCache[1];
    const CachedWaypoint& p2 = mWaypointCache[2];
    const CachedWaypoint& p3 = mWaypointCache[3];

    // Convert to local NED
    double n0, e0, n2, e2, n3, e3;
    GeodeticOffsetToNED(p1.lat, p1.lon, p0.lat, p0.lon, n0, e0);
    GeodeticOffsetToNED(p1.lat, p1.lon, p2.lat, p2.lon, n2, e2);
    GeodeticOffsetToNED(p1.lat, p1.lon, p3.lat, p3.lon, n3, e3);

    Vector3 v0(n0, e0, -(p0.alt - p1.alt));
    Vector3 v1(0, 0, 0);
    Vector3 v2(n2, e2, -(p2.alt - p1.alt));
    Vector3 v3(n3, e3, -(p3.alt - p1.alt));

    return CatmullRomTangent(v0, v1, v2, v3, t, mSplineTension);
}

// ============================================================================
// Convert arc distance to spline parameter
// ============================================================================

double Rotorcraft6DOF_Mover::ArcDistanceToParameter(double arcDist) const
{
    if (mSegmentArcLength <= 0.0)
        return 0.0;
    if (arcDist <= 0.0)
        return 0.0;
    if (arcDist >= mSegmentArcLength)
        return 1.0;

    if (mWaypointCache.size() < 4)
        return arcDist / mSegmentArcLength;  // Linear fallback

    const CachedWaypoint& p0 = mWaypointCache[0];
    const CachedWaypoint& p1 = mWaypointCache[1];
    const CachedWaypoint& p2 = mWaypointCache[2];
    const CachedWaypoint& p3 = mWaypointCache[3];

    // Convert to local NED
    double n0, e0, n2, e2, n3, e3;
    GeodeticOffsetToNED(p1.lat, p1.lon, p0.lat, p0.lon, n0, e0);
    GeodeticOffsetToNED(p1.lat, p1.lon, p2.lat, p2.lon, n2, e2);
    GeodeticOffsetToNED(p1.lat, p1.lon, p3.lat, p3.lon, n3, e3);

    Vector3 v0(n0, e0, -(p0.alt - p1.alt));
    Vector3 v1(0, 0, 0);
    Vector3 v2(n2, e2, -(p2.alt - p1.alt));
    Vector3 v3(n3, e3, -(p3.alt - p1.alt));

    return ArcDistanceToSplineParameter(v0, v1, v2, v3, arcDist, mSegmentArcLength, mSplineTension);
}

// ============================================================================
// Convert spline parameter to arc distance
// ============================================================================

double Rotorcraft6DOF_Mover::ParameterToArcDistance(double t) const
{
    if (mSegmentArcLength <= 0.0)
        return 0.0;
    if (t <= 0.0)
        return 0.0;
    if (t >= 1.0)
        return mSegmentArcLength;

    // Linear approximation (fast, good enough for speed profiling)
    return t * mSegmentArcLength;
}

// ============================================================================
// Compute speed profile for current segment
// ============================================================================

void Rotorcraft6DOF_Mover::ComputeSpeedProfile()
{
    if (mWaypointCache.size() < 4)
        return;

    // Get behavior of destination waypoint (P2 in cache)
    const WaypointConfig& destConfig = mWaypointCache[2].config;

    // Determine entry and exit speeds
    mSegmentEntrySpeed = mWaypointCache[1].speed;
    if (mSegmentEntrySpeed <= 0.0)
        mSegmentEntrySpeed = mTargetSpeed;

    // Exit speed depends on waypoint behavior
    switch (destConfig.behavior)
    {
        case WaypointBehavior::STOP_AND_HOVER:
            mSegmentExitSpeed = 0.0;
            break;

        case WaypointBehavior::SLOW_PASS:
            mSegmentExitSpeed = destConfig.passSpeed;
            break;

        case WaypointBehavior::FLY_THROUGH:
        default:
            // Maintain speed or use waypoint speed
            mSegmentExitSpeed = mWaypointCache[2].speed;
            if (mSegmentExitSpeed <= 0.0)
                mSegmentExitSpeed = mSegmentEntrySpeed;
            break;
    }

    // Calculate deceleration distance if needed
    if (mSegmentExitSpeed < mSegmentEntrySpeed)
    {
        // d = (v1² - v2²) / (2 * a)
        double v1 = mSegmentEntrySpeed;
        double v2 = mSegmentExitSpeed;
        mDecelStartDistance = (v1 * v1 - v2 * v2) / (2.0 * mMaxDeceleration);

        // Clamp to segment length
        if (mDecelStartDistance > mSegmentArcLength)
            mDecelStartDistance = mSegmentArcLength;
    }
    else
    {
        mDecelStartDistance = 0.0;  // No deceleration needed
    }

    // Initialize profiled speed
    mProfiledSpeed = mSegmentEntrySpeed;

    std::cout << "[ROTORCRAFT_6DOF] Speed profile: entry=" << mSegmentEntrySpeed
              << " m/s, exit=" << mSegmentExitSpeed << " m/s, decel_dist="
              << mDecelStartDistance << " m" << std::endl;
}

// ============================================================================
// Get profiled speed at current position
// ============================================================================

double Rotorcraft6DOF_Mover::GetProfiledSpeed() const
{
    double distanceToEnd = mSegmentArcLength - mDistanceAlongSegment;

    // Check if in deceleration zone
    if (distanceToEnd <= mDecelStartDistance && mDecelStartDistance > 0.0)
    {
        // v = sqrt(v2² + 2*a*d)
        double v2 = mSegmentExitSpeed;
        double speed = std::sqrt(v2 * v2 + 2.0 * mMaxDeceleration * distanceToEnd);
        return std::min(speed, mSegmentEntrySpeed);
    }

    // Cruise at entry speed
    return mSegmentEntrySpeed;
}

// ============================================================================
// Transition to new navigation state
// ============================================================================

void Rotorcraft6DOF_Mover::TransitionToNavState(SplineNavState newState, double simTime)
{
    if (newState == mNavState)
        return;

    SplineNavState oldState = mNavState;
    mNavState = newState;

    switch (newState)
    {
        case SplineNavState::AT_WAYPOINT:
        {
            // Starting hover at waypoint
            const WaypointConfig& config = mWaypointCache[2].config;
            mHoverStartTime = simTime;
            mHoverEndTime = simTime + config.hoverDuration;

            // Determine departure heading
            if (config.departureHeading > -900.0)
            {
                // User-specified heading
                mDepartureHeading = config.departureHeading * DEG_TO_RAD;
                mDepartureHeadingSet = true;
            }
            else
            {
                // Auto: face next waypoint
                if (mWaypointCache.size() > 3)
                {
                    double n, e;
                    GeodeticOffsetToNED(mWaypointCache[2].lat, mWaypointCache[2].lon,
                                        mWaypointCache[3].lat, mWaypointCache[3].lon,
                                        n, e);
                    mDepartureHeading = std::atan2(e, n);
                    mDepartureHeadingSet = true;
                }
                else
                {
                    mDepartureHeadingSet = false;
                }
            }

            std::cout << "[ROTORCRAFT_6DOF] Entering hover at waypoint "
                      << mCurrentWaypointIndex + 1 << " for "
                      << config.hoverDuration << " seconds" << std::endl;
            break;
        }

        case SplineNavState::ROTATING:
            std::cout << "[ROTORCRAFT_6DOF] Rotating to departure heading "
                      << (mDepartureHeading * RAD_TO_DEG) << " deg" << std::endl;
            break;

        case SplineNavState::DEPARTING:
            std::cout << "[ROTORCRAFT_6DOF] Departing waypoint" << std::endl;
            break;

        case SplineNavState::APPROACHING:
            // Normal navigation
            break;
    }
}

// ============================================================================
// Update spline navigation state machine
// ============================================================================

void Rotorcraft6DOF_Mover::UpdateSplineNavigation(double dt, double simTime)
{
    if (!mSplineEnabled || mWaypointCache.size() < 4)
        return;

    WsfPlatform* platform = GetPlatform();
    if (!platform)
        return;

    // Get current platform state
    double currentLat, currentLon, currentAlt;
    platform->GetLocationLLA(currentLat, currentLon, currentAlt);

    const RigidBodyState& state = mRigidBody.GetState();
    double currentSpeed = state.velocity.Magnitude();
    double currentYaw = state.yaw;

    switch (mNavState)
    {
        case SplineNavState::APPROACHING:
        {
            // Update distance along segment based on actual movement
            // Use current velocity to estimate progress
            mDistanceAlongSegment += currentSpeed * dt;

            // Clamp to segment length
            if (mDistanceAlongSegment >= mSegmentArcLength)
                mDistanceAlongSegment = mSegmentArcLength;

            // Update spline parameter from distance
            mSplineParameter = mDistanceAlongSegment / mSegmentArcLength;
            if (mSplineParameter > 1.0)
                mSplineParameter = 1.0;

            // Update profiled speed
            mProfiledSpeed = GetProfiledSpeed();

            // Compute target position on spline
            double targetLat, targetLon, targetAlt;
            ComputeSplinePosition(mSplineParameter, targetLat, targetLon, targetAlt);

            // Set target for position controller
            SetTargetPosition(targetLat, targetLon, targetAlt);
            mTargetSpeed = mProfiledSpeed;

            // Check for waypoint arrival
            if (mSplineParameter >= 0.999)
            {
                const WaypointConfig& destConfig = mWaypointCache[2].config;

                if (destConfig.behavior == WaypointBehavior::STOP_AND_HOVER)
                {
                    TransitionToNavState(SplineNavState::AT_WAYPOINT, simTime);
                }
                else
                {
                    // FLY_THROUGH or SLOW_PASS: advance to next segment
                    WaypointReached(simTime);
                    AdvanceWaypointCache();
                }
            }
            break;
        }

        case SplineNavState::AT_WAYPOINT:
        {
            // Hovering at waypoint
            // Target is the waypoint position
            SetTargetPosition(mWaypointCache[2].lat, mWaypointCache[2].lon,
                              mWaypointCache[2].alt);
            mTargetSpeed = 0.0;
            mProfiledSpeed = 0.0;

            // Check if hover duration complete
            if (simTime >= mHoverEndTime)
            {
                if (mDepartureHeadingSet)
                {
                    TransitionToNavState(SplineNavState::ROTATING, simTime);
                }
                else
                {
                    // No rotation needed, proceed
                    WaypointReached(simTime);
                    AdvanceWaypointCache();
                }
            }
            break;
        }

        case SplineNavState::ROTATING:
        {
            // Rotating to departure heading
            // Hold position but allow yaw
            SetTargetPosition(mWaypointCache[2].lat, mWaypointCache[2].lon,
                              mWaypointCache[2].alt);
            mTargetSpeed = 0.0;
            mProfiledSpeed = 0.0;

            // Check if rotation complete
            double headingError = mDepartureHeading - currentYaw;
            while (headingError > PI) headingError -= 2.0 * PI;
            while (headingError < -PI) headingError += 2.0 * PI;

            // Override yaw command to rotate to departure heading
            mYawCmd = Clamp(headingError * 1.5, -1.0, 1.0);

            if (std::abs(headingError) < 5.0 * DEG_TO_RAD)
            {
                // Rotation complete
                WaypointReached(simTime);
                AdvanceWaypointCache();
            }
            break;
        }

        case SplineNavState::DEPARTING:
            // Transitional state - should immediately go to APPROACHING
            mNavState = SplineNavState::APPROACHING;
            break;
    }
}

// ============================================================================
// Setup coaxial octo configuration (8 arms, 2 rotors per arm = 16 motors)
// Reserved for future heavy-lift platforms
// ============================================================================

void Rotorcraft6DOF_Mover::SetupCoaxialOcto(double armLength, double rotorDiameter, double coaxialSpacing)
{
    mRotors.clear();

    // 8 arms at 45 degree intervals (0, 45, 90, 135, 180, 225, 270, 315)
    // Each arm has 2 counter-rotating rotors (upper and lower)
    for (int arm = 0; arm < 8; ++arm)
    {
        double angle = arm * 45.0 * DEG_TO_RAD;
        double x = armLength * std::cos(angle);
        double y = armLength * std::sin(angle);

        // Upper rotor (alternating CCW/CW by arm)
        RotorConfig cfgUpper;
        cfgUpper.position = Vector3(x, y, -coaxialSpacing / 2.0);
        cfgUpper.direction = (arm % 2 == 0) ? RotorDirection::CCW : RotorDirection::CW;
        cfgUpper.diameter = rotorDiameter;
        cfgUpper.thrustCoeff = mDefaultThrustCoeff;
        cfgUpper.torqueCoeff = mDefaultTorqueCoeff;
        cfgUpper.maxRPM = mDefaultMaxRPM;
        cfgUpper.timeConstant = mDefaultMotorTimeConstant;
        cfgUpper.rollMix = -y / armLength;
        cfgUpper.pitchMix = x / armLength;
        cfgUpper.yawMix = (cfgUpper.direction == RotorDirection::CCW) ? 1.0 : -1.0;
        mRotors.push_back(RotorModel(cfgUpper));

        // Lower rotor (opposite direction from upper)
        RotorConfig cfgLower;
        cfgLower.position = Vector3(x, y, coaxialSpacing / 2.0);
        cfgLower.direction = (arm % 2 == 0) ? RotorDirection::CW : RotorDirection::CCW;
        cfgLower.diameter = rotorDiameter;
        cfgLower.thrustCoeff = mDefaultThrustCoeff;
        cfgLower.torqueCoeff = mDefaultTorqueCoeff;
        cfgLower.maxRPM = mDefaultMaxRPM;
        cfgLower.timeConstant = mDefaultMotorTimeConstant;
        cfgLower.rollMix = -y / armLength;
        cfgLower.pitchMix = x / armLength;
        cfgLower.yawMix = (cfgLower.direction == RotorDirection::CCW) ? 1.0 : -1.0;
        mRotors.push_back(RotorModel(cfgLower));
    }
}

} // namespace rotorcraft_6dof
