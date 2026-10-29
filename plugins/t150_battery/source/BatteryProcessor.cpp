// ****************************************************************************
// T-150 UAS Battery Processor Plugin - Implementation
//
// Phase 2 Enhancement: Voltage sag model for realistic Li-Ion behavior
// Phase 4 Enhancement: Generic battery integration with CSV output
//
// Copyright 2025-2026 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "BatteryProcessor.hpp"

#include <cmath>
#include <algorithm>
#include <iomanip>

#include "UtInput.hpp"
#include "UtLog.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

namespace t150
{

// ============================================================================
// Constructor
BatteryProcessor::BatteryProcessor(const WsfScenario& aScenario)
    : WsfProcessor(aScenario),
      mBatteryCapacityWh(7000.0),           // T-150 default: 7000 Wh
      mInitialSOC(1.0),                     // Start fully charged
      mHoverPowerW(18000.0),                // 18 kW hover (fallback)
      mLowBatteryThreshold(0.20),           // 20% warning
      mUseDynamicPower(false),              // Default: use fixed hover power
      // Generic power source (Phase 4)
      mPowerSourceKey("PROP_TOTAL_POWER"),  // Default for backward compatibility
      // Auxiliary systems power (Phase 6 - realistic power model)
      mAuxiliaryPowerW(4000.0),             // 4kW for avionics, 8x ESCs, cooling, radio, camera, etc.
      mMotorEfficiency(0.85),               // 85% - typical brushless motor/ESC efficiency
      mMinFlightPowerFactor(0.50),          // Min 50% of hover power when flying
      mCruisePowerW(17000.0),               // 17 kW at cruise speed (T-150 spec)
      mCruiseSpeedMps(30.0),                // 30 m/s cruise speed (T-150 spec)
      mUseVelocityPowerModel(true),         // Use velocity-based power estimation
      // Voltage model defaults (13S Li-Ion pack)
      mNominalVoltage(46.8),                // 13S * 3.6V nominal
      mFullChargeVoltage(54.6),             // 13S * 4.2V full charge
      mCutoffVoltage(39.0),                 // 13S * 3.0V cutoff
      mInternalResistance(0.025),           // 25 milliohms pack resistance
      mCapacityAh(150.0),                   // 7000Wh / 46.8V ~ 150Ah
      mVoltageSagWarning(42.0),             // Warn below 42V (~3.23V/cell)
      // Runtime state
      mCurrentSOC(1.0),
      mCurrentPower(0.0),
      mEnergyConsumed(0.0),
      mLastUpdateTime(0.0),
      mLowWarningIssued(false),
      // Voltage model state
      mOpenCircuitVoltage(54.6),
      mTerminalVoltage(54.6),
      mVoltageSag(0.0),
      mCurrentAmps(0.0),
      mVoltageSagWarningIssued(false),
      // CSV output (Phase 4)
      mCsvOutputPath(),
      mCsvEnabled(false),
      mCsvLogInterval(1.0),                 // Default: log every 1 second
      mLastCsvLogTime(0.0),
      // Flight statistics (Phase 4)
      mFlightStartTime(0.0),
      mSumPower(0.0),
      mPowerSampleCount(0)
{
    SetUpdateInterval(0.1);  // 100ms update rate
}

// ============================================================================
// Copy constructor for Clone()
BatteryProcessor::BatteryProcessor(const BatteryProcessor& aSrc)
    : WsfProcessor(aSrc),
      mBatteryCapacityWh(aSrc.mBatteryCapacityWh),
      mInitialSOC(aSrc.mInitialSOC),
      mHoverPowerW(aSrc.mHoverPowerW),
      mLowBatteryThreshold(aSrc.mLowBatteryThreshold),
      mUseDynamicPower(aSrc.mUseDynamicPower),
      // Generic power source (Phase 4)
      mPowerSourceKey(aSrc.mPowerSourceKey),
      // Auxiliary systems power (Phase 6)
      mAuxiliaryPowerW(aSrc.mAuxiliaryPowerW),
      mMotorEfficiency(aSrc.mMotorEfficiency),
      mMinFlightPowerFactor(aSrc.mMinFlightPowerFactor),
      mCruisePowerW(aSrc.mCruisePowerW),
      mCruiseSpeedMps(aSrc.mCruiseSpeedMps),
      mUseVelocityPowerModel(aSrc.mUseVelocityPowerModel),
      // Voltage model parameters
      mNominalVoltage(aSrc.mNominalVoltage),
      mFullChargeVoltage(aSrc.mFullChargeVoltage),
      mCutoffVoltage(aSrc.mCutoffVoltage),
      mInternalResistance(aSrc.mInternalResistance),
      mCapacityAh(aSrc.mCapacityAh),
      mVoltageSagWarning(aSrc.mVoltageSagWarning),
      // Runtime state
      mCurrentSOC(aSrc.mCurrentSOC),
      mCurrentPower(aSrc.mCurrentPower),
      mEnergyConsumed(aSrc.mEnergyConsumed),
      mLastUpdateTime(aSrc.mLastUpdateTime),
      mLowWarningIssued(aSrc.mLowWarningIssued),
      // Voltage model state
      mOpenCircuitVoltage(aSrc.mOpenCircuitVoltage),
      mTerminalVoltage(aSrc.mTerminalVoltage),
      mVoltageSag(aSrc.mVoltageSag),
      mCurrentAmps(aSrc.mCurrentAmps),
      mVoltageSagWarningIssued(aSrc.mVoltageSagWarningIssued),
      // CSV output (Phase 4) - don't copy file handle, will open new if needed
      mCsvOutputPath(aSrc.mCsvOutputPath),
      mCsvEnabled(aSrc.mCsvEnabled),
      mCsvLogInterval(aSrc.mCsvLogInterval),
      mLastCsvLogTime(0.0),
      // Flight statistics (Phase 4)
      mFlightStartTime(0.0),
      mSumPower(0.0),
      mPowerSampleCount(0)
{
}

// ============================================================================
// Destructor - write summary and close CSV file
BatteryProcessor::~BatteryProcessor()
{
    WriteSummary();
}

// ============================================================================
// Get the class ID
WsfStringId BatteryProcessor::GetBatteryProcessorClassId()
{
    static WsfStringId classId("T150_BATTERY_PROCESSOR");
    return classId;
}

// ============================================================================
// Clone
WsfProcessor* BatteryProcessor::Clone() const
{
    return new BatteryProcessor(*this);
}

// ============================================================================
// Process input commands
bool BatteryProcessor::ProcessInput(UtInput& aInput)
{
    bool myCommand = true;
    std::string command(aInput.GetCommand());

    if (command == "battery_capacity_wh")
    {
        aInput.ReadValue(mBatteryCapacityWh);
        // Recalculate Ah capacity based on nominal voltage
        mCapacityAh = mBatteryCapacityWh / mNominalVoltage;
    }
    else if (command == "initial_soc")
    {
        aInput.ReadValue(mInitialSOC);
        mInitialSOC = std::clamp(mInitialSOC, 0.0, 1.0);
        mCurrentSOC = mInitialSOC;
    }
    else if (command == "hover_power_w")
    {
        aInput.ReadValue(mHoverPowerW);
    }
    else if (command == "low_battery_warning")
    {
        aInput.ReadValue(mLowBatteryThreshold);
        mLowBatteryThreshold = std::clamp(mLowBatteryThreshold, 0.0, 1.0);
    }
    // Voltage model parameters
    else if (command == "nominal_voltage")
    {
        aInput.ReadValue(mNominalVoltage);
        // Recalculate Ah capacity
        mCapacityAh = mBatteryCapacityWh / mNominalVoltage;
    }
    else if (command == "full_charge_voltage")
    {
        aInput.ReadValue(mFullChargeVoltage);
    }
    else if (command == "cutoff_voltage")
    {
        aInput.ReadValue(mCutoffVoltage);
    }
    else if (command == "internal_resistance")
    {
        aInput.ReadValue(mInternalResistance);
    }
    else if (command == "voltage_sag_warning")
    {
        aInput.ReadValue(mVoltageSagWarning);
    }
    else if (command == "use_dynamic_power")
    {
        mUseDynamicPower = aInput.ReadBool();
    }
    // Phase 4: Generic power source configuration
    else if (command == "power_source_key")
    {
        aInput.ReadValue(mPowerSourceKey);
        // Strip surrounding quotes if present (ReadValue includes them)
        if (mPowerSourceKey.size() >= 2 &&
            mPowerSourceKey.front() == '"' && mPowerSourceKey.back() == '"')
        {
            mPowerSourceKey = mPowerSourceKey.substr(1, mPowerSourceKey.size() - 2);
        }
    }
    // Phase 4: CSV output configuration
    else if (command == "csv_output_file")
    {
        aInput.ReadPath(mCsvOutputPath);
        mCsvEnabled = !mCsvOutputPath.empty();
    }
    else if (command == "csv_log_interval")
    {
        aInput.ReadValue(mCsvLogInterval);
        mCsvLogInterval = std::max(0.1, mCsvLogInterval);  // Minimum 100ms
    }
    // Phase 6: Auxiliary systems and realistic power model
    else if (command == "auxiliary_power_w")
    {
        aInput.ReadValue(mAuxiliaryPowerW);
    }
    else if (command == "motor_efficiency")
    {
        aInput.ReadValue(mMotorEfficiency);
        mMotorEfficiency = std::clamp(mMotorEfficiency, 0.1, 1.0);
    }
    else if (command == "min_flight_power_factor")
    {
        aInput.ReadValue(mMinFlightPowerFactor);
        mMinFlightPowerFactor = std::clamp(mMinFlightPowerFactor, 0.1, 1.0);
    }
    else if (command == "cruise_power_w")
    {
        aInput.ReadValue(mCruisePowerW);
    }
    else if (command == "cruise_speed_mps")
    {
        aInput.ReadValue(mCruiseSpeedMps);
    }
    else if (command == "use_velocity_power_model")
    {
        mUseVelocityPowerModel = aInput.ReadBool();
    }
    else
    {
        myCommand = WsfProcessor::ProcessInput(aInput);
    }

    return myCommand;
}

// ============================================================================
// Initialize
bool BatteryProcessor::Initialize(double aSimTime)
{
    bool ok = WsfProcessor::Initialize(aSimTime);
    if (ok)
    {
        mCurrentSOC = mInitialSOC;
        // Initialize energy consumed based on initial SOC
        // If starting at 30% SOC, 70% of capacity has already been "consumed"
        mEnergyConsumed = (1.0 - mInitialSOC) * mBatteryCapacityWh;
        mCurrentPower = 0.0;
        mLastUpdateTime = aSimTime;
        mLowWarningIssued = false;
        mVoltageSagWarningIssued = false;

        // Initialize voltage model state
        mOpenCircuitVoltage = CalculateOpenCircuitVoltage(mCurrentSOC);
        mTerminalVoltage = mOpenCircuitVoltage;  // No load at start
        mVoltageSag = 0.0;
        mCurrentAmps = 0.0;

        // Phase 4: Initialize flight statistics
        mFlightStartTime = aSimTime;
        mSumPower = 0.0;
        mPowerSampleCount = 0;
        mLastCsvLogTime = aSimTime;

        // Phase 4: Open CSV file and write header
        if (mCsvEnabled && !mCsvOutputPath.empty())
        {
            mCsvFile.open(mCsvOutputPath);
            if (mCsvFile.is_open())
            {
                // Write CSV header
                mCsvFile << "Time_s,SOC_pct,Voltage_V,Current_A,Power_W,Energy_Consumed_Wh,"
                         << "OCV_V,Voltage_Sag_V,Remaining_Energy_Wh\n";
                mCsvFile << std::fixed << std::setprecision(3);
            }
            else
            {
                auto err = ut::log::warning() << "Battery: Failed to open CSV file.";
                err.AddNote() << "Path: " << mCsvOutputPath;
                mCsvEnabled = false;
            }
        }

        // Set auxiliary data on platform for script access
        GetPlatform()->GetAuxData().AssignDouble("BATTERY_SOC", mCurrentSOC);
        GetPlatform()->GetAuxData().AssignDouble("BATTERY_POWER_W", mCurrentPower);
        GetPlatform()->GetAuxData().AssignDouble("BATTERY_ENERGY_WH", GetRemainingEnergy());
        // Voltage model aux data
        GetPlatform()->GetAuxData().AssignDouble("BATTERY_VOLTAGE", mTerminalVoltage);
        GetPlatform()->GetAuxData().AssignDouble("BATTERY_OCV", mOpenCircuitVoltage);
        GetPlatform()->GetAuxData().AssignDouble("BATTERY_VOLTAGE_SAG", mVoltageSag);
        GetPlatform()->GetAuxData().AssignDouble("BATTERY_CURRENT_A", mCurrentAmps);

        auto out = ut::log::info() << "Battery processor initialized.";
        out.AddNote() << "Platform: " << GetPlatform()->GetName();
        out.AddNote() << "Capacity: " << mBatteryCapacityWh << " Wh (" << mCapacityAh << " Ah)";
        out.AddNote() << "Initial SOC: " << (mInitialSOC * 100.0) << "%";
        out.AddNote() << "Voltage: " << mFullChargeVoltage << "V (full) / "
                      << mNominalVoltage << "V (nom) / " << mCutoffVoltage << "V (cutoff)";
        out.AddNote() << "Internal resistance: " << (mInternalResistance * 1000.0) << " mOhm";
        out.AddNote() << "Power source key: " << mPowerSourceKey;
        out.AddNote() << "Dynamic power mode: " << (mUseDynamicPower ? "ENABLED" : "DISABLED (fixed hover power)");
        out.AddNote() << "Motor/ESC efficiency: " << (mMotorEfficiency * 100.0) << "% (mech->elec conversion)";
        out.AddNote() << "Auxiliary systems: " << (mAuxiliaryPowerW / 1000.0) << " kW";
        out.AddNote() << "Min flight power: " << (mMinFlightPowerFactor * 100.0) << "% of hover ("
                      << (mHoverPowerW * mMinFlightPowerFactor) << " W)";
        out.AddNote() << "Cruise power: " << mCruisePowerW << " W @ " << mCruiseSpeedMps << " m/s";
        out.AddNote() << "Velocity power model: " << (mUseVelocityPowerModel ? "ENABLED" : "DISABLED");
        if (mCsvEnabled)
        {
            out.AddNote() << "CSV output: " << mCsvOutputPath;
        }
    }
    return ok;
}

// ============================================================================
// Update - called periodically to update battery state
void BatteryProcessor::Update(double aSimTime)
{
    WsfProcessor::Update(aSimTime);

    // DEBUG: Verify Update() is being called
    static int updateCounter = 0;
    updateCounter++;
    if (updateCounter % 3000 == 0)
    {
        std::cout << "[BATTERY UPDATE] Called! Counter=" << updateCounter
                  << " Time=" << aSimTime << "s" << std::endl;
    }

    if (!IsOperational())
        return;

    double deltaTime = aSimTime - mLastUpdateTime;
    if (deltaTime <= 0.0)
        return;

    // ========================================================================
    // PHASE 6: REALISTIC POWER MODEL
    // ========================================================================
    // Power consumption includes:
    // 1. Propulsion power (from mover or velocity-based estimate)
    // 2. Auxiliary systems (avionics, radio, camera, ECU, sensors)
    // 3. Minimum flight power floor (aircraft can't fly on zero power)
    // ========================================================================

    double propulsionPower = 0.0;

    // Determine propulsion power consumption
    if (mUseDynamicPower)
    {
        // DEBUG: Print diagnostic every 3000 updates
        static int debugCounter = 0;
        debugCounter++;
        bool doDebug = (debugCounter % 3000 == 0);

        if (doDebug)
        {
            std::cout << "[BATTERY POWER READ] DynamicMode=TRUE | PowerSourceKey='"
                      << mPowerSourceKey << "'" << std::endl;
        }

        // Try to read power from configured source with fallback chain
        double sourcePower = 0.0;
        bool powerFound = false;

        // Try configured power source key first
        bool gotKey = GetPlatform()->GetAuxData().TryGet(mPowerSourceKey.c_str(), sourcePower);

        if (doDebug)
        {
            std::cout << "[BATTERY POWER READ] TryGet Result: " << (gotKey ? "SUCCESS" : "FAILED")
                      << " | Value=" << sourcePower << "W | >0? " << (sourcePower > 0.0 ? "YES" : "NO") << std::endl;
        }

        if (gotKey && sourcePower > 0.0)
        {
            propulsionPower = sourcePower;
            powerFound = true;
        }
        // Fallback: try 6DOF mover power
        else if (GetPlatform()->GetAuxData().TryGet("6dof_total_power", sourcePower) && sourcePower > 0.0)
        {
            propulsionPower = sourcePower;
            powerFound = true;
        }
        // Fallback: try PROP_TOTAL_POWER (original propulsion processor)
        else if (GetPlatform()->GetAuxData().TryGet("PROP_TOTAL_POWER", sourcePower) && sourcePower > 0.0)
        {
            propulsionPower = sourcePower;
            powerFound = true;
        }

        if (!powerFound)
        {
            // No power source available, use fallback hover power
            propulsionPower = mHoverPowerW;
        }
    }
    else
    {
        // Use fixed hover power (original behavior)
        propulsionPower = mHoverPowerW;

        // DEBUG
        static int elseCounter = 0;
        elseCounter++;
        if (elseCounter % 3000 == 0)
        {
            std::cout << "[BATTERY POWER READ] DynamicMode=FALSE | Using hover power: "
                      << (mHoverPowerW/1000.0) << " kW" << std::endl;
        }
    }

    // ========================================================================
    // Velocity-based power estimation (OPTIONAL)
    // ========================================================================
    // Calculate power based on velocity using a physics-based model.
    // This ensures realistic power consumption when the mover doesn't report
    // accurate power. However, with a physics-based 6DOF mover, this should
    // be DISABLED to trust the mover's calculations.
    // ========================================================================

    double minFlightPower = mHoverPowerW * mMinFlightPowerFactor;

    // ========================================================================
    // GROUND DETECTION - Always check regardless of velocity model
    // ========================================================================
    // Get velocity and altitude from platform
    double velocityNED[3] = {0.0, 0.0, 0.0};
    GetPlatform()->GetVelocityNED(velocityNED);

    double velHoriz = std::sqrt(velocityNED[0] * velocityNED[0] +
                                 velocityNED[1] * velocityNED[1]);
    double velVert = -velocityNED[2];  // NED: positive Z is down

    double altitude = GetPlatform()->GetAltitude();
    const double groundThreshold = 5.0;  // Below 5m AGL considered "on ground"
    bool isOnGround = (altitude < groundThreshold) && (velHoriz < 1.0) && (std::abs(velVert) < 1.0);

    // If on ground, use minimal ground idle power (auxiliary systems only)
    if (isOnGround)
    {
        propulsionPower = 0.0;
    }
    else if (mUseVelocityPowerModel)
    {
        // Aircraft is in flight AND velocity model enabled - apply velocity-based power estimation
        double speedFactor = std::min(1.0, velHoriz / mCruiseSpeedMps);

        double estimatedPower;

        if (velHoriz < 1.0)
        {
            // Nearly stationary in air - use hover power
            estimatedPower = mHoverPowerW;
        }
        else
        {
            // In flight - interpolate based on speed
            estimatedPower = mHoverPowerW + (mCruisePowerW - mHoverPowerW) * speedFactor;
        }

        // Add climb power (weight * climb_rate / efficiency)
        if (velVert > 0.5)
        {
            double climbPower = 120.0 * 9.81 * velVert / 0.7;
            estimatedPower += climbPower;
        }

        // Use the MAXIMUM of: reported power, estimated power, or minimum floor
        double maxPower = propulsionPower;
        if (estimatedPower > maxPower) maxPower = estimatedPower;
        if (minFlightPower > maxPower) maxPower = minFlightPower;
        propulsionPower = maxPower;
    }
    // else: Velocity model DISABLED and in flight - trust 6DOF mover's power calculations

    // ========================================================================
    // Convert mechanical power to electrical power using motor efficiency
    // ========================================================================
    // The 6DOF mover reports mechanical (shaft) power. Real electrical power
    // is higher due to motor/ESC losses, coaxial rotor interference, etc.
    // Electrical Power = Mechanical Power / Efficiency
    // ========================================================================

    double electricalPropulsionPower = propulsionPower / mMotorEfficiency;

    // ========================================================================
    // Add auxiliary systems power
    // ========================================================================
    // Avionics, flight controller, radio, camera, sensors, ECU, cooling, etc.
    // This is a constant draw regardless of flight state.
    // ========================================================================

    mCurrentPower = electricalPropulsionPower + mAuxiliaryPowerW;

    // Store the requested power - this is what we NEED to fly
    // The voltage sag model may limit deliverable power, but we still
    // need to deplete the battery based on requested power for realistic range
    double requestedPower = mCurrentPower;

    // Track power statistics for average calculation
    mSumPower += mCurrentPower;
    mPowerSampleCount++;

    // ========================================================================
    // VOLTAGE SAG MODEL (Quadratic Solution - Stable)
    // ========================================================================
    //
    // For a constant power load P with internal resistance R and OCV:
    //   P = V_terminal × I
    //   V_terminal = OCV - I×R
    //
    // Substituting: P = (OCV - I×R) × I = OCV×I - R×I²
    // Rearranging:  R×I² - OCV×I + P = 0  (quadratic in I)
    //
    // Using quadratic formula with the smaller root (physical solution):
    //   I = (OCV - sqrt(OCV² - 4×R×P)) / (2×R)
    //
    // This is stable because it directly solves for current without feedback.
    // ========================================================================

    // 1. Calculate open-circuit voltage based on current SOC
    mOpenCircuitVoltage = CalculateOpenCircuitVoltage(mCurrentSOC);

    // 2. Calculate internal resistance (may vary with SOC)
    double resistance = CalculateInternalResistance(mCurrentSOC);

    // 3. Solve for current using quadratic formula (stable solution)
    //    discriminant = OCV² - 4×R×P
    double discriminant = mOpenCircuitVoltage * mOpenCircuitVoltage - 4.0 * resistance * mCurrentPower;

    if (discriminant >= 0.0 && resistance > 0.0)
    {
        // Physical solution: smaller root gives lower current (more efficient)
        mCurrentAmps = (mOpenCircuitVoltage - std::sqrt(discriminant)) / (2.0 * resistance);

        // Calculate voltage sag and terminal voltage
        mVoltageSag = mCurrentAmps * resistance;
        mTerminalVoltage = mOpenCircuitVoltage - mVoltageSag;
    }
    else
    {
        // Power demand exceeds what battery can deliver (discriminant < 0)
        // or resistance is zero - fall back to cutoff clamp
        mTerminalVoltage = mCutoffVoltage;
        mCurrentAmps = (mOpenCircuitVoltage - mCutoffVoltage) / std::max(resistance, 0.001);
        mVoltageSag = mOpenCircuitVoltage - mCutoffVoltage;
        mCurrentPower = mTerminalVoltage * mCurrentAmps;
    }

    // Clamp to cutoff voltage (battery protection would kick in)
    if (mTerminalVoltage < mCutoffVoltage)
    {
        mTerminalVoltage = mCutoffVoltage;
        // Recalculate actual deliverable power at cutoff
        mCurrentAmps = (mOpenCircuitVoltage - mCutoffVoltage) / std::max(resistance, 0.001);
        mVoltageSag = mOpenCircuitVoltage - mCutoffVoltage;
        mCurrentPower = mTerminalVoltage * mCurrentAmps;
    }

    // ========================================================================
    // ENERGY CONSUMPTION
    // ========================================================================
    // Use REQUESTED power for energy depletion, not limited deliverable power.
    // This ensures the battery depletes at a realistic rate based on what
    // the aircraft needs to fly, not what the battery can actually deliver.
    // (If battery can't deliver enough power, aircraft would crash in reality)

    // Calculate energy consumed this timestep (Wh = W * h)
    double energyDelta = requestedPower * (deltaTime / 3600.0);

    // Update state of charge
    mEnergyConsumed += energyDelta;
    mCurrentSOC = std::max(0.0, 1.0 - (mEnergyConsumed / mBatteryCapacityWh));

    // Report requested power in CSV (what aircraft needs, not what battery can deliver)
    mCurrentPower = requestedPower;

    // ========================================================================
    // UPDATE AUX DATA
    // ========================================================================

    GetPlatform()->GetAuxData().AssignDouble("BATTERY_SOC", mCurrentSOC);
    GetPlatform()->GetAuxData().AssignDouble("BATTERY_POWER_W", mCurrentPower);
    GetPlatform()->GetAuxData().AssignDouble("BATTERY_ENERGY_WH", GetRemainingEnergy());
    // Voltage model aux data
    GetPlatform()->GetAuxData().AssignDouble("BATTERY_VOLTAGE", mTerminalVoltage);
    GetPlatform()->GetAuxData().AssignDouble("BATTERY_OCV", mOpenCircuitVoltage);
    GetPlatform()->GetAuxData().AssignDouble("BATTERY_VOLTAGE_SAG", mVoltageSag);
    GetPlatform()->GetAuxData().AssignDouble("BATTERY_CURRENT_A", mCurrentAmps);

    // ========================================================================
    // WARNINGS
    // ========================================================================

    // Check for low battery warning (SOC-based)
    if (mCurrentSOC <= mLowBatteryThreshold && !mLowWarningIssued)
    {
        auto out = ut::log::warning() << "Low battery warning.";
        out.AddNote() << "Platform: " << GetPlatform()->GetName();
        out.AddNote() << "SOC: " << (mCurrentSOC * 100.0) << "%";
        out.AddNote() << "Time: " << aSimTime << " sec";
        mLowWarningIssued = true;

        GetPlatform()->GetAuxData().AssignBool("BATTERY_LOW", true);
    }

    // Check for voltage sag warning (voltage-based)
    if (mTerminalVoltage <= mVoltageSagWarning && !mVoltageSagWarningIssued)
    {
        auto out = ut::log::warning() << "Battery voltage sag warning!";
        out.AddNote() << "Platform: " << GetPlatform()->GetName();
        out.AddNote() << "Terminal voltage: " << mTerminalVoltage << " V";
        out.AddNote() << "Voltage sag: " << mVoltageSag << " V";
        out.AddNote() << "Current draw: " << mCurrentAmps << " A";
        out.AddNote() << "SOC: " << (mCurrentSOC * 100.0) << "%";
        mVoltageSagWarningIssued = true;

        GetPlatform()->GetAuxData().AssignBool("BATTERY_VOLTAGE_LOW", true);
    }

    // Reset voltage warning if voltage recovers (load reduced)
    if (mTerminalVoltage > mVoltageSagWarning + 1.0 && mVoltageSagWarningIssued)
    {
        mVoltageSagWarningIssued = false;
        GetPlatform()->GetAuxData().AssignBool("BATTERY_VOLTAGE_LOW", false);
    }

    // Phase 4: Write CSV row at specified interval
    if (mCsvEnabled && (aSimTime - mLastCsvLogTime >= mCsvLogInterval))
    {
        WriteCSVRow(aSimTime);
        mLastCsvLogTime = aSimTime;
    }

    mLastUpdateTime = aSimTime;
}

// ============================================================================
// Script class name
const char* BatteryProcessor::GetScriptClassName() const
{
    return "T150BatteryProcessor";
}

// ============================================================================
// Calculate open-circuit voltage from SOC using Li-Ion discharge curve
//
// Li-Ion cells have a characteristic discharge curve:
// - 4.2V at 100% SOC (fully charged)
// - ~3.6V at ~50% SOC (nominal)
// - 3.0V at 0% SOC (cutoff)
//
// This uses a polynomial approximation of the Li-Ion discharge curve.
// The curve is relatively flat in the 20-80% SOC range with steeper
// dropoffs at the ends.
double BatteryProcessor::CalculateOpenCircuitVoltage(double soc) const
{
    // Clamp SOC to valid range
    soc = std::clamp(soc, 0.0, 1.0);

    // Per-cell voltage calculation using polynomial fit
    // V_cell = a0 + a1*SOC + a2*SOC^2 + a3*SOC^3
    // Coefficients fitted to typical Li-Ion discharge curve
    const double a0 = 3.0;    // Cutoff voltage per cell
    const double a1 = 1.2;    // Linear term
    const double a2 = -0.5;   // Quadratic term (creates the flat middle region)
    const double a3 = 0.5;    // Cubic term (steeper at ends)

    double soc2 = soc * soc;
    double soc3 = soc2 * soc;

    double cellVoltage = a0 + a1 * soc + a2 * soc2 + a3 * soc3;

    // Scale to pack voltage (based on full charge / cutoff ratio)
    // V_pack = V_cell * (V_full / 4.2)
    // For 13S pack: V_cell * 13
    double cellCount = mFullChargeVoltage / 4.2;

    return cellVoltage * cellCount;
}

// ============================================================================
// Calculate internal resistance (varies with SOC and temperature)
//
// Li-Ion internal resistance increases at:
// - Low SOC (below ~20%)
// - High SOC (above ~90%)
// - Low temperatures (not modeled here)
//
// This creates a U-shaped curve with minimum resistance around 50-70% SOC.
double BatteryProcessor::CalculateInternalResistance(double soc) const
{
    // Clamp SOC to valid range
    soc = std::clamp(soc, 0.0, 1.0);

    // Base resistance (user-configurable)
    double baseResistance = mInternalResistance;

    // Resistance multiplier based on SOC
    // Minimum at ~60% SOC, increases at extremes
    double socFactor;

    if (soc > 0.6)
    {
        // Slight increase at high SOC
        socFactor = 1.0 + 0.3 * std::pow(soc - 0.6, 2) / 0.16;  // peaks ~1.3x at 100%
    }
    else if (soc > 0.2)
    {
        // Relatively flat in middle range
        socFactor = 1.0;
    }
    else
    {
        // Significant increase at low SOC
        // Resistance can double or triple at very low SOC
        socFactor = 1.0 + 2.0 * std::pow(0.2 - soc, 2) / 0.04;  // peaks ~3x at 0%
    }

    return baseResistance * socFactor;
}

// ============================================================================
// Write a row to the CSV output file
void BatteryProcessor::WriteCSVRow(double simTime)
{
    if (!mCsvFile.is_open())
        return;

    // Time_s,SOC_pct,Voltage_V,Current_A,Power_W,Energy_Consumed_Wh,OCV_V,Voltage_Sag_V,Remaining_Energy_Wh
    mCsvFile << simTime << ","
             << (mCurrentSOC * 100.0) << ","
             << mTerminalVoltage << ","
             << mCurrentAmps << ","
             << mCurrentPower << ","
             << mEnergyConsumed << ","
             << mOpenCircuitVoltage << ","
             << mVoltageSag << ","
             << GetRemainingEnergy() << "\n";
}

// ============================================================================
// Write final summary to CSV and log
void BatteryProcessor::WriteSummary()
{
    double flightDuration = mLastUpdateTime - mFlightStartTime;
    double avgPower = (mPowerSampleCount > 0) ? (mSumPower / mPowerSampleCount) : 0.0;

    // Write summary to CSV file
    if (mCsvFile.is_open())
    {
        mCsvFile << "\n# ========================================\n";
        mCsvFile << "# BATTERY SUMMARY\n";
        mCsvFile << "# ========================================\n";
        mCsvFile << "# Flight Duration: " << flightDuration << " seconds\n";
        mCsvFile << "# Initial SOC: " << (mInitialSOC * 100.0) << " %\n";
        mCsvFile << "# Final SOC: " << (mCurrentSOC * 100.0) << " %\n";
        mCsvFile << "# Total Energy Consumed: " << mEnergyConsumed << " Wh\n";
        mCsvFile << "# Average Power: " << avgPower << " W\n";
        mCsvFile << "# Average Power: " << (avgPower / 1000.0) << " kW\n";
        mCsvFile << "# Battery Capacity: " << mBatteryCapacityWh << " Wh\n";
        mCsvFile << "# Energy Efficiency: " << ((mEnergyConsumed / std::max(flightDuration, 1.0)) * 3600.0 / 1000.0) << " kWh/hour\n";
        mCsvFile.close();
    }

    // Write summary to AFSIM log
    if (mPowerSampleCount > 0)
    {
        auto out = ut::log::info() << "Battery simulation complete.";
        out.AddNote() << "Flight duration: " << flightDuration << " seconds";
        out.AddNote() << "Initial SOC: " << (mInitialSOC * 100.0) << "%";
        out.AddNote() << "Final SOC: " << (mCurrentSOC * 100.0) << "%";
        out.AddNote() << "Energy consumed: " << mEnergyConsumed << " Wh";
        out.AddNote() << "Average power: " << (avgPower / 1000.0) << " kW";
        if (mCsvEnabled)
        {
            out.AddNote() << "CSV saved to: " << mCsvOutputPath;
        }
    }
}

} // namespace t150
