// ****************************************************************************
// T-150 UAS Electric Propulsion System Plugin - Implementation
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "PropulsionProcessor.hpp"

#include <cmath>
#include <numbers>
#include <algorithm>
#include <numeric>
#include <iomanip>

#include "UtInput.hpp"
#include "UtLog.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

namespace t150
{

// Physical constants
constexpr double AIR_DENSITY_SEA_LEVEL = 1.225;  // kg/m^3
constexpr double GRAVITY = 9.81;                  // m/s^2

// ============================================================================
// Constructor
PropulsionProcessor::PropulsionProcessor(const WsfScenario& aScenario)
    : WsfProcessor(aScenario),
      mMotorCount(8),                    // T-150 has 8 motors
      mKvRating(100.0),                  // U15II KV rating
      mMaxPowerPerMotor(3500.0),         // 3.5 kW per motor
      mPropellerDiameter(0.76),          // 30" propeller
      mCoaxialEfficiency(0.85),          // Coaxial interference
      mMotorThermalLimit(120.0),         // Max motor temp (C)
      mESCThermalLimit(90.0),            // Max ESC temp (C)
      mMotorThermalMass(1500.0),         // Thermal mass J/K (~1.5kg motor * 1000 J/kg/K)
      mMotorCoolingRate(20.0),           // Cooling W/K (forced convection from prop wash)
      mPeakMotorEfficiency(0.92),        // Peak efficiency
      mNominalVoltage(46.8),             // 13S Li-Ion
      mThrustConstant(0.10),             // Thrust coefficient
      mTorqueConstant(0.012),            // Torque coefficient
      mMotorTimeConstant(0.15),          // 150ms time constant for large motors
      mGroundEffectEnabled(true),        // Ground effect enabled by default
      mRotorHeight(0.5),                 // Rotor height above ground when landed (m)
      mGroundEffectMaxBoost(1.3),        // Up to 30% thrust boost in ground effect
      mEmptyMass(68.0),                  // T-150 empty mass (kg)
      mPayloadMass(0.0),                 // No payload by default
      mMaxPayloadMass(55.0),             // Max 55 kg payload for T-150
      mPayloadCgOffsetX(0.0),            // Payload CG at aircraft CG (m)
      mPayloadCgOffsetY(0.0),
      mPayloadCgOffsetZ(0.2),            // Payload below aircraft CG (m)
      mGroundEffectFactor(1.0),          // Current ground effect multiplier
      mCurrentAltitudeAGL(0.0),          // Current altitude AGL
      mTotalThrust(0.0),
      mTotalPower(0.0),
      mSystemEfficiency(0.85),
      mAmbientTemperature(25.0),
      mLastUpdateTime(0.0),
      mThermalWarningIssued(false),
      mMotorFailureWarningIssued(false),
      mTotalMass(68.0),                  // Start with empty mass
      mCombinedCgX(0.0),
      mCombinedCgY(0.0),
      mCombinedCgZ(0.0),
      mThrustMargin(1.0),                // Full margin when empty
      // Phase 6: Dynamic flight state
      mLastVelNorth(0.0),
      mLastVelEast(0.0),
      mLastVelDown(0.0),
      mCurrentAccelX(0.0),
      mCurrentAccelY(0.0),
      mCurrentAccelZ(0.0),
      mForwardSpeed(0.0),
      mClimbRate(0.0),
      mBankAngle(0.0),
      // Phase 6: Aerodynamic parameters (T-150 specific)
      mDragCoefficient(1.0),             // Bluff body drag coefficient
      mFrontalArea(0.30),                // m^2, derived from cruise power data
      mInducedPowerFactor(1.15),         // Non-ideal rotor factor
      // CSV output
      mCsvOutputPath(""),
      mCsvEnabled(false),
      mCsvLogInterval(1.0),
      mLastCsvLogTime(0.0)
{
    SetUpdateInterval(0.1);  // 100ms update rate

    // Initialize motor array
    for (auto& motor : mMotors)
    {
        motor.state = MotorState::OFF;
        motor.rpm = 0.0;
        motor.current = 0.0;
        motor.temperature = 25.0;
        motor.efficiency = mPeakMotorEfficiency;
        motor.thrust = 0.0;
        motor.torque = 0.0;
        motor.throttle = 0.0;
    }
}

// ============================================================================
// Copy constructor
PropulsionProcessor::PropulsionProcessor(const PropulsionProcessor& aSrc)
    : WsfProcessor(aSrc),
      mMotorCount(aSrc.mMotorCount),
      mKvRating(aSrc.mKvRating),
      mMaxPowerPerMotor(aSrc.mMaxPowerPerMotor),
      mPropellerDiameter(aSrc.mPropellerDiameter),
      mCoaxialEfficiency(aSrc.mCoaxialEfficiency),
      mMotorThermalLimit(aSrc.mMotorThermalLimit),
      mESCThermalLimit(aSrc.mESCThermalLimit),
      mMotorThermalMass(aSrc.mMotorThermalMass),
      mMotorCoolingRate(aSrc.mMotorCoolingRate),
      mPeakMotorEfficiency(aSrc.mPeakMotorEfficiency),
      mNominalVoltage(aSrc.mNominalVoltage),
      mThrustConstant(aSrc.mThrustConstant),
      mTorqueConstant(aSrc.mTorqueConstant),
      mMotorTimeConstant(aSrc.mMotorTimeConstant),
      mGroundEffectEnabled(aSrc.mGroundEffectEnabled),
      mRotorHeight(aSrc.mRotorHeight),
      mGroundEffectMaxBoost(aSrc.mGroundEffectMaxBoost),
      mEmptyMass(aSrc.mEmptyMass),
      mPayloadMass(aSrc.mPayloadMass),
      mMaxPayloadMass(aSrc.mMaxPayloadMass),
      mPayloadCgOffsetX(aSrc.mPayloadCgOffsetX),
      mPayloadCgOffsetY(aSrc.mPayloadCgOffsetY),
      mPayloadCgOffsetZ(aSrc.mPayloadCgOffsetZ),
      mGroundEffectFactor(aSrc.mGroundEffectFactor),
      mCurrentAltitudeAGL(aSrc.mCurrentAltitudeAGL),
      mMotors(aSrc.mMotors),
      mTotalThrust(aSrc.mTotalThrust),
      mTotalPower(aSrc.mTotalPower),
      mSystemEfficiency(aSrc.mSystemEfficiency),
      mAmbientTemperature(aSrc.mAmbientTemperature),
      mLastUpdateTime(aSrc.mLastUpdateTime),
      mThermalWarningIssued(aSrc.mThermalWarningIssued),
      mMotorFailureWarningIssued(aSrc.mMotorFailureWarningIssued),
      mTotalMass(aSrc.mTotalMass),
      mCombinedCgX(aSrc.mCombinedCgX),
      mCombinedCgY(aSrc.mCombinedCgY),
      mCombinedCgZ(aSrc.mCombinedCgZ),
      mThrustMargin(aSrc.mThrustMargin),
      // Phase 6: Dynamic flight state
      mLastVelNorth(aSrc.mLastVelNorth),
      mLastVelEast(aSrc.mLastVelEast),
      mLastVelDown(aSrc.mLastVelDown),
      mCurrentAccelX(aSrc.mCurrentAccelX),
      mCurrentAccelY(aSrc.mCurrentAccelY),
      mCurrentAccelZ(aSrc.mCurrentAccelZ),
      mForwardSpeed(aSrc.mForwardSpeed),
      mClimbRate(aSrc.mClimbRate),
      mBankAngle(aSrc.mBankAngle),
      // Phase 6: Aerodynamic parameters
      mDragCoefficient(aSrc.mDragCoefficient),
      mFrontalArea(aSrc.mFrontalArea),
      mInducedPowerFactor(aSrc.mInducedPowerFactor)
{
}

// ============================================================================
// Get the class ID
WsfStringId PropulsionProcessor::GetPropulsionProcessorClassId()
{
    static WsfStringId classId("T150_PROPULSION_PROCESSOR");
    return classId;
}

// ============================================================================
// Clone
WsfProcessor* PropulsionProcessor::Clone() const
{
    return new PropulsionProcessor(*this);
}

// ============================================================================
// Process input commands
bool PropulsionProcessor::ProcessInput(UtInput& aInput)
{
    bool myCommand = true;
    std::string command(aInput.GetCommand());

    if (command == "motor_count")
    {
        aInput.ReadValue(mMotorCount);
        mMotorCount = std::clamp(mMotorCount, 1, NUM_MOTORS);
    }
    else if (command == "kv_rating")
    {
        aInput.ReadValue(mKvRating);
        mKvRating = std::abs(mKvRating);
    }
    else if (command == "max_power_per_motor")
    {
        aInput.ReadValue(mMaxPowerPerMotor);
        mMaxPowerPerMotor = std::abs(mMaxPowerPerMotor);
    }
    else if (command == "propeller_diameter")
    {
        aInput.ReadValue(mPropellerDiameter);
        mPropellerDiameter = std::abs(mPropellerDiameter);
    }
    else if (command == "coaxial_efficiency")
    {
        aInput.ReadValue(mCoaxialEfficiency);
        mCoaxialEfficiency = std::clamp(mCoaxialEfficiency, 0.5, 1.0);
    }
    else if (command == "motor_thermal_limit")
    {
        aInput.ReadValue(mMotorThermalLimit);
    }
    else if (command == "esc_thermal_limit")
    {
        aInput.ReadValue(mESCThermalLimit);
    }
    else if (command == "nominal_voltage")
    {
        aInput.ReadValue(mNominalVoltage);
        mNominalVoltage = std::abs(mNominalVoltage);
    }
    else if (command == "peak_motor_efficiency")
    {
        aInput.ReadValue(mPeakMotorEfficiency);
        mPeakMotorEfficiency = std::clamp(mPeakMotorEfficiency, 0.5, 0.98);
    }
    else if (command == "ambient_temperature")
    {
        aInput.ReadValue(mAmbientTemperature);
    }
    else if (command == "motor_time_constant")
    {
        aInput.ReadValue(mMotorTimeConstant);
        mMotorTimeConstant = std::max(0.01, mMotorTimeConstant);  // Minimum 10ms
    }
    // Phase 4: Ground effect parameters
    else if (command == "ground_effect_enabled")
    {
        mGroundEffectEnabled = aInput.ReadBool();
    }
    else if (command == "rotor_height")
    {
        aInput.ReadValue(mRotorHeight);
        mRotorHeight = std::max(0.1, mRotorHeight);  // Minimum 0.1m
    }
    else if (command == "ground_effect_max_boost")
    {
        aInput.ReadValue(mGroundEffectMaxBoost);
        mGroundEffectMaxBoost = std::clamp(mGroundEffectMaxBoost, 1.0, 1.5);  // 0-50% boost max
    }
    // Phase 4: Payload dynamics parameters
    else if (command == "empty_mass")
    {
        aInput.ReadValue(mEmptyMass);
        mEmptyMass = std::max(10.0, mEmptyMass);  // Minimum 10 kg
    }
    else if (command == "payload_mass")
    {
        aInput.ReadValue(mPayloadMass);
        mPayloadMass = std::max(0.0, mPayloadMass);  // No negative payload
    }
    else if (command == "max_payload_mass")
    {
        aInput.ReadValue(mMaxPayloadMass);
        mMaxPayloadMass = std::max(0.0, mMaxPayloadMass);
    }
    else if (command == "payload_cg_offset_x")
    {
        aInput.ReadValue(mPayloadCgOffsetX);
    }
    else if (command == "payload_cg_offset_y")
    {
        aInput.ReadValue(mPayloadCgOffsetY);
    }
    else if (command == "payload_cg_offset_z")
    {
        aInput.ReadValue(mPayloadCgOffsetZ);
    }
    // Phase 6: Aerodynamic parameters for dynamic power model
    else if (command == "drag_coefficient")
    {
        aInput.ReadValue(mDragCoefficient);
        mDragCoefficient = std::clamp(mDragCoefficient, 0.1, 2.0);
    }
    else if (command == "frontal_area")
    {
        aInput.ReadValue(mFrontalArea);
        mFrontalArea = std::max(0.1, mFrontalArea);
    }
    else if (command == "induced_power_factor")
    {
        aInput.ReadValue(mInducedPowerFactor);
        mInducedPowerFactor = std::clamp(mInducedPowerFactor, 1.0, 1.5);
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
bool PropulsionProcessor::Initialize(double aSimTime)
{
    bool ok = WsfProcessor::Initialize(aSimTime);
    if (ok)
    {
        // Initialize all motors
        for (int i = 0; i < mMotorCount; ++i)
        {
            mMotors[i].state = MotorState::RUNNING;
            mMotors[i].temperature = mAmbientTemperature;
            mMotors[i].efficiency = mPeakMotorEfficiency;
        }

        mLastUpdateTime = aSimTime;
        mThermalWarningIssued = false;
        mMotorFailureWarningIssued = false;

        // Initialize ground effect state
        mGroundEffectFactor = 1.0;
        mCurrentAltitudeAGL = 0.0;

        // Initialize payload state (Phase 4)
        mTotalMass = mEmptyMass + mPayloadMass;
        mCombinedCgX = 0.0;
        mCombinedCgY = 0.0;
        mCombinedCgZ = 0.0;

        // Calculate combined CG if payload is present
        if (mPayloadMass > 0.0 && mTotalMass > 0.0)
        {
            // CG shift = (payload_mass * payload_offset) / total_mass
            mCombinedCgX = (mPayloadMass * mPayloadCgOffsetX) / mTotalMass;
            mCombinedCgY = (mPayloadMass * mPayloadCgOffsetY) / mTotalMass;
            mCombinedCgZ = (mPayloadMass * mPayloadCgOffsetZ) / mTotalMass;
        }

        // Calculate thrust margin (max thrust vs required hover thrust)
        double maxThrust = mMotorCount * mMaxPowerPerMotor * mThrustConstant;
        double requiredHoverThrust = mTotalMass * GRAVITY;
        mThrustMargin = (maxThrust > 0.0) ? std::max(0.0, 1.0 - (requiredHoverThrust / maxThrust)) : 0.0;

        // Set aux data
        GetPlatform()->GetAuxData().AssignDouble("PROP_TOTAL_THRUST", mTotalThrust);
        GetPlatform()->GetAuxData().AssignDouble("PROP_TOTAL_POWER", mTotalPower);
        GetPlatform()->GetAuxData().AssignDouble("PROP_SYSTEM_EFF", mSystemEfficiency);
        GetPlatform()->GetAuxData().AssignInt("PROP_MOTOR_COUNT", GetOperationalMotorCount());
        GetPlatform()->GetAuxData().AssignDouble("PROP_GROUND_EFFECT", mGroundEffectFactor);
        GetPlatform()->GetAuxData().AssignDouble("PROP_ALTITUDE_AGL", mCurrentAltitudeAGL);

        // Payload aux data (Phase 4)
        GetPlatform()->GetAuxData().AssignDouble("PROP_TOTAL_MASS", mTotalMass);
        GetPlatform()->GetAuxData().AssignDouble("PROP_PAYLOAD_MASS", mPayloadMass);
        GetPlatform()->GetAuxData().AssignDouble("PROP_THRUST_MARGIN", mThrustMargin);
        GetPlatform()->GetAuxData().AssignDouble("PROP_CG_OFFSET_X", mCombinedCgX);
        GetPlatform()->GetAuxData().AssignDouble("PROP_CG_OFFSET_Y", mCombinedCgY);
        GetPlatform()->GetAuxData().AssignDouble("PROP_CG_OFFSET_Z", mCombinedCgZ);
        GetPlatform()->GetAuxData().AssignBool("PROP_OVERLOADED", IsOverloaded());

        // Open CSV file and write header
        if (mCsvEnabled && !mCsvOutputPath.empty())
        {
            mCsvFile.open(mCsvOutputPath);
            if (mCsvFile.is_open())
            {
                // Write CSV header
                mCsvFile << "Time_s,Total_Thrust_N,Total_Power_W,System_Efficiency,"
                         << "Altitude_AGL_m,Ground_Effect_Factor,Total_Mass_kg,Payload_Mass_kg,"
                         << "Thrust_Margin,Forward_Speed_ms,Climb_Rate_ms,Bank_Angle_deg,"
                         << "Motor_Avg_RPM,Motor_Avg_Temp_C,Motor_Avg_Current_A\n";
                mCsvFile << std::fixed << std::setprecision(4);
                mLastCsvLogTime = aSimTime;
            }
            else
            {
                auto err = ut::log::warning() << "Propulsion: Failed to open CSV file.";
                err.AddNote() << "Path: " << mCsvOutputPath;
                mCsvEnabled = false;
            }
        }

        auto out = ut::log::info() << "Propulsion processor initialized.";
        out.AddNote() << "Platform: " << GetPlatform()->GetName();
        out.AddNote() << "Motors: " << mMotorCount;
        out.AddNote() << "Max power per motor: " << mMaxPowerPerMotor << " W";
        out.AddNote() << "Ground effect: " << (mGroundEffectEnabled ? "ENABLED" : "DISABLED")
                      << " (max boost: " << ((mGroundEffectMaxBoost - 1.0) * 100.0) << "%)";
        out.AddNote() << "Empty mass: " << mEmptyMass << " kg, Payload: " << mPayloadMass
                      << " kg, Total: " << mTotalMass << " kg";
        out.AddNote() << "Thrust margin: " << (mThrustMargin * 100.0) << "%"
                      << (IsOverloaded() ? " (OVERLOADED!)" : "");
        if (mCsvEnabled)
        {
            out.AddNote() << "CSV output: " << mCsvOutputPath;
        }
    }
    return ok;
}

// ============================================================================
// Update
void PropulsionProcessor::Update(double aSimTime)
{
    WsfProcessor::Update(aSimTime);

    if (!IsOperational())
        return;

    double deltaTime = aSimTime - mLastUpdateTime;
    if (deltaTime <= 0.0)
        return;

    // ========================================================================
    // Phase 4: Get altitude AGL for ground effect calculation
    // ========================================================================
    mCurrentAltitudeAGL = 0.0;
    if (GetPlatform() != nullptr)
    {
        // Get altitude MSL from platform
        // Note: For accurate ground effect, we'd need terrain height lookup via WsfTerrain.
        // For now, we use MSL altitude and assume flat terrain at sea level.
        // This is a reasonable approximation for most operational scenarios.
        double altitudeMSL = GetPlatform()->GetAltitude();

        // Assume flat terrain at sea level (terrain height = 0)
        // TODO: Future enhancement - integrate with WsfTerrain for actual terrain height
        mCurrentAltitudeAGL = altitudeMSL;
    }

    // ========================================================================
    // Phase 6: Read velocity and compute acceleration for dynamic power model
    // ========================================================================
    double velocityNED[3] = {0.0, 0.0, 0.0};
    if (GetPlatform() != nullptr)
    {
        GetPlatform()->GetVelocityNED(velocityNED);
    }

    double velNorth = velocityNED[0];
    double velEast = velocityNED[1];
    double velDown = velocityNED[2];

    // Compute derived flight state values
    mForwardSpeed = std::sqrt(velNorth * velNorth + velEast * velEast);
    mClimbRate = -velDown;  // Positive = climbing (NED convention: down is positive)

    // Compute acceleration from velocity change (numerical differentiation)
    if (deltaTime > 0.001)  // Avoid division by very small numbers
    {
        mCurrentAccelX = (velNorth - mLastVelNorth) / deltaTime;
        mCurrentAccelY = (velEast - mLastVelEast) / deltaTime;
        mCurrentAccelZ = (velDown - mLastVelDown) / deltaTime;

        // Clamp to reasonable values to filter noise
        const double maxAccel = 20.0;  // m/s^2
        mCurrentAccelX = std::clamp(mCurrentAccelX, -maxAccel, maxAccel);
        mCurrentAccelY = std::clamp(mCurrentAccelY, -maxAccel, maxAccel);
        mCurrentAccelZ = std::clamp(mCurrentAccelZ, -maxAccel, maxAccel);
    }

    // Store velocity for next iteration
    mLastVelNorth = velNorth;
    mLastVelEast = velEast;
    mLastVelDown = velDown;

    // Estimate bank angle from horizontal acceleration
    // In a coordinated turn: a_centripetal = g * tan(bank)
    // So: bank = atan(a_horiz / g)
    double horizAccel = std::sqrt(mCurrentAccelX * mCurrentAccelX +
                                   mCurrentAccelY * mCurrentAccelY);
    mBankAngle = std::atan(horizAccel / GRAVITY);
    mBankAngle = std::clamp(mBankAngle, 0.0, 1.05);  // Max ~60 degrees

    // Calculate ground effect factor
    if (mGroundEffectEnabled)
    {
        mGroundEffectFactor = ComputeGroundEffectFactor(mCurrentAltitudeAGL);
    }
    else
    {
        mGroundEffectFactor = 1.0;
    }

    // Compute required thrust
    double requiredThrust = ComputeRequiredThrust(aSimTime);

    // Distribute thrust across motors
    DistributeThrust(requiredThrust);

    // Update each motor
    mTotalThrust = 0.0;
    mTotalPower = 0.0;
    double totalEfficiency = 0.0;
    int operationalCount = 0;

    for (int i = 0; i < mMotorCount; ++i)
    {
        UpdateMotor(i, deltaTime);

        if (mMotors[i].state == MotorState::RUNNING)
        {
            mTotalThrust += mMotors[i].thrust;
            mTotalPower += mMotors[i].current * mNominalVoltage;
            totalEfficiency += mMotors[i].efficiency;
            operationalCount++;
        }

        CheckMotorHealth(i, aSimTime);
    }

    // Apply ground effect to total thrust (thrust augmentation near ground)
    mTotalThrust *= mGroundEffectFactor;

    // Calculate system efficiency
    if (operationalCount > 0)
    {
        mSystemEfficiency = totalEfficiency / operationalCount;
    }

    // Apply coaxial efficiency (lower rotors get less clean air)
    mSystemEfficiency *= mCoaxialEfficiency;

    // ========================================================================
    // Phase 6: Compute dynamic power based on actual flight state
    // This replaces the motor-summed power with a physics-based calculation
    // ========================================================================
    mTotalPower = ComputeDynamicPower();

    // Update aux data
    GetPlatform()->GetAuxData().AssignDouble("PROP_TOTAL_THRUST", mTotalThrust);
    GetPlatform()->GetAuxData().AssignDouble("PROP_TOTAL_POWER", mTotalPower);
    GetPlatform()->GetAuxData().AssignDouble("PROP_SYSTEM_EFF", mSystemEfficiency);
    GetPlatform()->GetAuxData().AssignInt("PROP_MOTOR_COUNT", operationalCount);
    GetPlatform()->GetAuxData().AssignDouble("PROP_MAX_TEMP", GetMaxMotorTemperature());
    GetPlatform()->GetAuxData().AssignDouble("PROP_AVG_TEMP", GetAverageMotorTemperature());
    GetPlatform()->GetAuxData().AssignDouble("PROP_GROUND_EFFECT", mGroundEffectFactor);
    GetPlatform()->GetAuxData().AssignDouble("PROP_ALTITUDE_AGL", mCurrentAltitudeAGL);

    // Update payload aux data (Phase 4)
    GetPlatform()->GetAuxData().AssignDouble("PROP_TOTAL_MASS", mTotalMass);
    GetPlatform()->GetAuxData().AssignDouble("PROP_PAYLOAD_MASS", mPayloadMass);
    GetPlatform()->GetAuxData().AssignDouble("PROP_THRUST_MARGIN", mThrustMargin);
    GetPlatform()->GetAuxData().AssignDouble("PROP_CG_OFFSET_X", mCombinedCgX);
    GetPlatform()->GetAuxData().AssignDouble("PROP_CG_OFFSET_Y", mCombinedCgY);
    GetPlatform()->GetAuxData().AssignDouble("PROP_CG_OFFSET_Z", mCombinedCgZ);

    // Phase 6: Dynamic flight state aux data
    GetPlatform()->GetAuxData().AssignDouble("PROP_FORWARD_SPEED", mForwardSpeed);
    GetPlatform()->GetAuxData().AssignDouble("PROP_CLIMB_RATE", mClimbRate);
    GetPlatform()->GetAuxData().AssignDouble("PROP_BANK_ANGLE", mBankAngle * 57.2957795);  // Convert to degrees
    // horizAccel was already computed above for bank angle estimation
    GetPlatform()->GetAuxData().AssignDouble("PROP_ACCEL_MAG", horizAccel);

    mLastUpdateTime = aSimTime;

    // Write CSV row if enabled and interval has elapsed
    if (mCsvEnabled && mCsvFile.is_open())
    {
        if (aSimTime - mLastCsvLogTime >= mCsvLogInterval)
        {
            WriteCSVRow(aSimTime);
            mLastCsvLogTime = aSimTime;
        }
    }
}

// ============================================================================
// Compute required thrust
double PropulsionProcessor::ComputeRequiredThrust(double /*aSimTime*/)
{
    // Use dynamic total mass (empty + payload) from Phase 4
    // mTotalMass is initialized in Initialize() and updated via configuration

    // Phase 6: Adjust thrust for bank angle
    // When banking, thrust vector tilts, requiring more total thrust to maintain altitude
    // T_required = m * g / cos(bank_angle)
    double bankFactor = 1.0 / std::max(0.5, std::cos(mBankAngle));

    // Base thrust for hover = weight * bank factor
    double weightForce = mTotalMass * GRAVITY * bankFactor;

    // Phase 6: Climb thrust component
    // Positive climb rate requires additional thrust (simplified model)
    // More accurate: T_climb ~ m * climb_rate / induced_velocity
    double climbThrust = 0.0;
    if (mClimbRate > 0.0)
    {
        // Climbing requires extra thrust
        climbThrust = mTotalMass * mClimbRate * 0.5;
    }
    else if (mClimbRate < 0.0)
    {
        // Descending reduces required thrust (but not below zero)
        climbThrust = mTotalMass * mClimbRate * 0.3;  // Less reduction than increase
    }

    // Total required thrust
    double totalThrust = weightForce + climbThrust;

    // Account for any failed motors
    int opCount = GetOperationalMotorCount();
    if (opCount < mMotorCount && opCount > 0)
    {
        totalThrust *= static_cast<double>(mMotorCount) / static_cast<double>(opCount);
    }

    return std::max(0.0, totalThrust);
}

// ============================================================================
// Distribute thrust across motors
void PropulsionProcessor::DistributeThrust(double aTotalThrust)
{
    int opCount = GetOperationalMotorCount();
    if (opCount == 0)
        return;

    // Equal distribution for hover
    double thrustPerMotor = aTotalThrust / opCount;

    // Limit to max thrust per motor
    double maxThrustPerMotor = (mMaxPowerPerMotor / 746.0) * 3.0;
    thrustPerMotor = std::min(thrustPerMotor, maxThrustPerMotor);

    // Calculate throttle
    double throttle = thrustPerMotor / maxThrustPerMotor;
    throttle = std::clamp(throttle, 0.0, 1.0);

    for (int i = 0; i < mMotorCount; ++i)
    {
        if (mMotors[i].state == MotorState::RUNNING)
        {
            mMotors[i].throttle = throttle;
        }
        else
        {
            mMotors[i].throttle = 0.0;
        }
    }
}

// ============================================================================
// Update individual motor state
void PropulsionProcessor::UpdateMotor(int aIndex, double aDeltaTime)
{
    if (aIndex < 0 || aIndex >= NUM_MOTORS)
        return;

    MotorData& motor = mMotors[aIndex];

    if (motor.state != MotorState::RUNNING)
    {
        motor.rpm = 0.0;
        motor.current = 0.0;
        motor.thrust = 0.0;
        motor.torque = 0.0;
        return;
    }

    // Calculate target RPM from throttle and voltage
    double effectiveVoltage = mNominalVoltage * motor.throttle;
    double noLoadRPM = mKvRating * effectiveVoltage;

    // Target RPM accounting for load (simplified)
    double targetRPM = noLoadRPM * 0.85;

    // First-order lag for motor spin-up/spin-down (actuator dynamics)
    // dRPM/dt = (target - current) / tau
    if (mMotorTimeConstant > 0.0 && aDeltaTime > 0.0)
    {
        double alpha = aDeltaTime / (mMotorTimeConstant + aDeltaTime);
        motor.rpm = motor.rpm + alpha * (targetRPM - motor.rpm);
    }
    else
    {
        motor.rpm = targetRPM;
    }

    // Calculate air density at sea level (simplified)
    double airDensity = AIR_DENSITY_SEA_LEVEL;

    // Calculate thrust using momentum theory
    double n = motor.rpm / 60.0;
    double D = mPropellerDiameter;
    double Ct = mThrustConstant;

    motor.thrust = Ct * airDensity * n * n * D * D * D * D;

    // Calculate torque
    double Cq = mTorqueConstant;
    motor.torque = Cq * airDensity * n * n * D * D * D * D * D;

    // Calculate power and current
    double mechanicalPower = motor.torque * (motor.rpm * 2.0 * std::numbers::pi / 60.0);
    motor.efficiency = ComputeMotorEfficiency(motor.rpm, motor.torque);
    double electricalPower = mechanicalPower / std::max(0.5, motor.efficiency);
    motor.current = electricalPower / mNominalVoltage;

    // Limit current
    double maxCurrent = mMaxPowerPerMotor / mNominalVoltage;
    if (motor.current > maxCurrent)
    {
        motor.current = maxCurrent;
        electricalPower = motor.current * mNominalVoltage;
        double factor = std::sqrt(electricalPower / (mechanicalPower / motor.efficiency));
        motor.thrust *= factor;
    }

    // Update temperature
    UpdateMotorTemperature(aIndex, aDeltaTime, motor.current * mNominalVoltage);
}

// ============================================================================
// Compute motor efficiency
double PropulsionProcessor::ComputeMotorEfficiency(double aRPM, double aTorque)
{
    double maxRPM = mKvRating * mNominalVoltage;
    double rpmRatio = aRPM / maxRPM;

    // Efficiency curve (simplified parabola)
    double rpmFactor = 1.0 - 0.5 * std::pow(rpmRatio - 0.7, 2);
    rpmFactor = std::clamp(rpmFactor, 0.5, 1.0);

    // Torque effect
    double maxTorque = mMaxPowerPerMotor / (maxRPM * 2.0 * std::numbers::pi / 60.0);
    double torqueRatio = aTorque / std::max(0.001, maxTorque);
    double torqueFactor = 1.0 - 0.2 * torqueRatio;
    torqueFactor = std::clamp(torqueFactor, 0.7, 1.0);

    return mPeakMotorEfficiency * rpmFactor * torqueFactor;
}

// ============================================================================
// Update motor temperature
void PropulsionProcessor::UpdateMotorTemperature(int aIndex, double aDeltaTime, double aPower)
{
    if (aIndex < 0 || aIndex >= NUM_MOTORS)
        return;

    MotorData& motor = mMotors[aIndex];

    // Heat generated = power * (1 - efficiency)
    double heatGenerated = aPower * (1.0 - motor.efficiency);

    // Heat dissipated = cooling_rate * (T_motor - T_ambient)
    double heatDissipated = mMotorCoolingRate * (motor.temperature - mAmbientTemperature);

    // Temperature change
    double netHeat = heatGenerated - heatDissipated;
    double tempChange = (netHeat * aDeltaTime) / mMotorThermalMass;

    motor.temperature += tempChange;

    // Clamp to reasonable range
    motor.temperature = std::clamp(motor.temperature, -40.0, 200.0);
}

// ============================================================================
// Check motor health
void PropulsionProcessor::CheckMotorHealth(int aIndex, double aSimTime)
{
    if (aIndex < 0 || aIndex >= NUM_MOTORS)
        return;

    MotorData& motor = mMotors[aIndex];

    // Check for overtemperature
    if (motor.temperature >= mMotorThermalLimit)
    {
        if (motor.state == MotorState::RUNNING)
        {
            motor.state = MotorState::OVERTEMP;
            auto out = ut::log::warning() << "Motor " << aIndex << " OVERTEMP!";
            out.AddNote() << "Platform: " << GetPlatform()->GetName();
            out.AddNote() << "Temperature: " << motor.temperature << " C";
            out.AddNote() << "Time: " << aSimTime << " sec";

            GetPlatform()->GetAuxData().AssignBool("MOTOR_OVERTEMP", true);
        }
    }
    // Allow recovery if temperature drops
    else if (motor.state == MotorState::OVERTEMP &&
             motor.temperature < mMotorThermalLimit - 10.0)
    {
        motor.state = MotorState::RUNNING;
        auto out = ut::log::info() << "Motor " << aIndex << " temperature recovered.";
        out.AddNote() << "Temperature: " << motor.temperature << " C";
    }

    // Check if any motors have failed
    if (HasMotorFailure() && !mMotorFailureWarningIssued)
    {
        auto out = ut::log::warning() << "MOTOR FAILURE DETECTED!";
        out.AddNote() << "Platform: " << GetPlatform()->GetName();
        out.AddNote() << "Operational motors: " << GetOperationalMotorCount();
        mMotorFailureWarningIssued = true;
        GetPlatform()->GetAuxData().AssignBool("MOTOR_FAILURE", true);
    }
}

// ============================================================================
// Get number of operational motors
int PropulsionProcessor::GetOperationalMotorCount() const
{
    int count = 0;
    for (int i = 0; i < mMotorCount; ++i)
    {
        if (mMotors[i].state == MotorState::RUNNING)
        {
            count++;
        }
    }
    return count;
}

// ============================================================================
// Get motor data
const MotorData& PropulsionProcessor::GetMotorData(int aIndex) const
{
    static MotorData empty;
    if (aIndex < 0 || aIndex >= NUM_MOTORS)
        return empty;
    return mMotors[aIndex];
}

// ============================================================================
// Get average motor temperature
double PropulsionProcessor::GetAverageMotorTemperature() const
{
    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < mMotorCount; ++i)
    {
        if (mMotors[i].state != MotorState::OFF)
        {
            sum += mMotors[i].temperature;
            count++;
        }
    }
    return count > 0 ? sum / count : mAmbientTemperature;
}

// ============================================================================
// Get max motor temperature
double PropulsionProcessor::GetMaxMotorTemperature() const
{
    double maxTemp = mAmbientTemperature;
    for (int i = 0; i < mMotorCount; ++i)
    {
        if (mMotors[i].temperature > maxTemp)
        {
            maxTemp = mMotors[i].temperature;
        }
    }
    return maxTemp;
}

// ============================================================================
// Check for motor failure
bool PropulsionProcessor::HasMotorFailure() const
{
    for (int i = 0; i < mMotorCount; ++i)
    {
        if (mMotors[i].state == MotorState::FAILED ||
            mMotors[i].state == MotorState::OVERTEMP)
        {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Check for thermal warning
bool PropulsionProcessor::IsThermalWarning() const
{
    double warnThreshold = mMotorThermalLimit * 0.85;
    for (int i = 0; i < mMotorCount; ++i)
    {
        if (mMotors[i].temperature >= warnThreshold)
        {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Fail a motor
void PropulsionProcessor::FailMotor(int aIndex)
{
    if (aIndex >= 0 && aIndex < NUM_MOTORS)
    {
        mMotors[aIndex].state = MotorState::FAILED;
    }
}

// ============================================================================
// Restore a motor
void PropulsionProcessor::RestoreMotor(int aIndex)
{
    if (aIndex >= 0 && aIndex < NUM_MOTORS)
    {
        mMotors[aIndex].state = MotorState::RUNNING;
        mMotors[aIndex].temperature = mAmbientTemperature;
    }
}

// ============================================================================
// Script class name
const char* PropulsionProcessor::GetScriptClassName() const
{
    return "T150PropulsionProcessor";
}

// ============================================================================
// Compute ground effect factor based on altitude AGL
// Uses Cheeseman-Bennett ground effect model:
//   T_ige/T_oge = 1 / (1 - (R/4h)^2)
// Where R = rotor radius, h = rotor height above ground
// For practical purposes, we model this as an augmented thrust factor
// that increases as the aircraft gets closer to the ground.
double PropulsionProcessor::ComputeGroundEffectFactor(double aAltitudeAGL)
{
    // Ground effect is negligible above 1 rotor diameter
    // Maximum effect occurs at ground level (landing/takeoff)

    if (!mGroundEffectEnabled)
    {
        return 1.0;  // No ground effect
    }

    // Rotor diameter from propeller diameter
    double rotorDiameter = mPropellerDiameter;  // meters (0.76m for T-150)
    double rotorRadius = rotorDiameter / 2.0;

    // Effective rotor height above ground
    // = altitude AGL + height of rotors above platform reference point
    double effectiveHeight = aAltitudeAGL + mRotorHeight;

    // Ground effect significant when height < 1 rotor diameter
    if (effectiveHeight > rotorDiameter)
    {
        return 1.0;  // Out of ground effect (OGE)
    }

    // Prevent division by zero and unrealistic values very close to ground
    double minHeight = rotorRadius * 0.25;  // Minimum height = 25% of rotor radius
    effectiveHeight = std::max(effectiveHeight, minHeight);

    // Cheeseman-Bennett approximation for ground effect
    // T_ige/T_oge = 1 / (1 - (R/4h)^2)
    // This gives thrust augmentation in ground effect
    double heightRatio = rotorRadius / (4.0 * effectiveHeight);
    double thrustRatio = 1.0 / (1.0 - heightRatio * heightRatio);

    // Clamp to maximum ground effect boost
    // Typical values: 1.1-1.3 for helicopters, can be higher for multicopters
    thrustRatio = std::min(thrustRatio, mGroundEffectMaxBoost);

    // Ensure minimum factor is 1.0 (no reduction)
    thrustRatio = std::max(thrustRatio, 1.0);

    return thrustRatio;
}

// ============================================================================
// Compute dynamic power based on flight state (Phase 6)
// This calculates total electrical power based on actual flight dynamics:
//   P_total = P_induced + P_climb + P_parasite + P_accel
//
// This provides more realistic power consumption that varies with maneuvers.
double PropulsionProcessor::ComputeDynamicPower()
{
    double rho = AIR_DENSITY_SEA_LEVEL;  // TODO: altitude correction for density

    // Total rotor disk area (all rotors combined)
    double rotorRadius = mPropellerDiameter / 2.0;
    double singleDiskArea = std::numbers::pi * rotorRadius * rotorRadius;
    double totalDiskArea = singleDiskArea * mMotorCount;

    // ========================================================================
    // 1. Induced Power (hover component)
    // P_induced = k * T^1.5 / sqrt(2 * rho * A)
    // Where k = induced power factor (accounts for non-ideal rotor)
    // ========================================================================
    double bankFactor = 1.0 / std::max(0.5, std::cos(mBankAngle));
    double thrustRequired = mTotalMass * GRAVITY * bankFactor;

    // Induced velocity in hover: v_i = sqrt(T / (2 * rho * A))
    double inducedVelocity = std::sqrt(thrustRequired / (2.0 * rho * totalDiskArea));

    // Induced power: P = k * T * v_i
    double P_induced = mInducedPowerFactor * thrustRequired * inducedVelocity;

    // ========================================================================
    // 2. Climb Power
    // P_climb = m * g * V_climb (rate of change of potential energy)
    // ========================================================================
    double P_climb = mTotalMass * GRAVITY * mClimbRate;

    // ========================================================================
    // 3. Parasite Drag Power (forward flight)
    // P_parasite = 0.5 * rho * Cd * A_frontal * V^3
    // ========================================================================
    double P_parasite = 0.5 * rho * mDragCoefficient * mFrontalArea *
                        std::pow(mForwardSpeed, 3);

    // ========================================================================
    // 4. Acceleration Power
    // P_accel = m * a * V (rate of change of kinetic energy)
    // ========================================================================
    double horizAccel = std::sqrt(mCurrentAccelX * mCurrentAccelX +
                                   mCurrentAccelY * mCurrentAccelY);
    double P_accel = mTotalMass * horizAccel * mForwardSpeed;

    // ========================================================================
    // Total mechanical power
    // ========================================================================
    double mechanicalPower = P_induced + P_climb + P_parasite + P_accel;

    // Ensure power is non-negative (descent can't generate net power in this model)
    mechanicalPower = std::max(1000.0, mechanicalPower);  // Minimum 1 kW for avionics/systems

    // Convert to electrical power (accounting for system efficiency)
    double electricalPower = mechanicalPower / std::max(0.5, mSystemEfficiency);

    // Clamp to physical limits
    double maxPower = mMotorCount * mMaxPowerPerMotor;
    electricalPower = std::clamp(electricalPower, 1000.0, maxPower);

    return electricalPower;
}

// ============================================================================
// Write a row to the CSV output file
void PropulsionProcessor::WriteCSVRow(double aSimTime)
{
    if (!mCsvFile.is_open())
        return;

    // Compute motor averages
    double avgRPM = 0.0;
    double avgTemp = 0.0;
    double avgCurrent = 0.0;
    int runningCount = 0;

    for (int i = 0; i < mMotorCount; ++i)
    {
        if (mMotors[i].state == MotorState::RUNNING)
        {
            avgRPM += mMotors[i].rpm;
            avgTemp += mMotors[i].temperature;
            avgCurrent += mMotors[i].current;
            runningCount++;
        }
    }

    if (runningCount > 0)
    {
        avgRPM /= runningCount;
        avgTemp /= runningCount;
        avgCurrent /= runningCount;
    }

    // Write CSV row
    mCsvFile << aSimTime << ","
             << mTotalThrust << ","
             << mTotalPower << ","
             << mSystemEfficiency << ","
             << mCurrentAltitudeAGL << ","
             << mGroundEffectFactor << ","
             << mTotalMass << ","
             << mPayloadMass << ","
             << mThrustMargin << ","
             << mForwardSpeed << ","
             << mClimbRate << ","
             << (mBankAngle * 57.2957795) << ","  // Convert to degrees
             << avgRPM << ","
             << avgTemp << ","
             << avgCurrent << "\n";
}

} // namespace t150
