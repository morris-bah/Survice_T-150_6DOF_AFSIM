// ****************************************************************************
// T-150 UAS Battery Processor Plugin
//
// This plugin provides real-time battery state-of-charge (SOC) modeling
// for the T-150 Heavy-Lift UAS simulation.
//
// Phase 2 Enhancement: Voltage sag model for realistic Li-Ion behavior
//   - Open-circuit voltage varies with SOC (Li-Ion discharge curve)
//   - Internal resistance causes voltage drop under load
//   - Voltage sag warning when terminal voltage drops below threshold
//
// Phase 4 Enhancement: Generic battery integration
//   - Configurable power source key (works with any mover/processor)
//   - CSV output for battery data logging
//   - Flight duration and average power tracking
//
// Copyright 2025-2026 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_BATTERY_PROCESSOR_HPP
#define T150_BATTERY_PROCESSOR_HPP

#include "processor/WsfProcessor.hpp"
#include <string>
#include <fstream>

namespace t150
{

//! A processor that models battery energy consumption and state-of-charge.
//! Includes voltage sag modeling for realistic Li-Ion behavior.
class BatteryProcessor : public WsfProcessor
{
public:
    //! Constructor
    explicit BatteryProcessor(const WsfScenario& aScenario);

    //! Destructor - writes CSV summary and closes file
    ~BatteryProcessor() override;

    //! Get the class ID for this processor type
    static WsfStringId GetBatteryProcessorClassId();

    //! @name Framework methods
    //@{
    WsfProcessor* Clone() const override;
    bool          ProcessInput(UtInput& aInput) override;
    bool          Initialize(double aSimTime) override;
    void          Update(double aSimTime) override;
    const char*   GetScriptClassName() const override;
    //@}

    //! @name Battery State Accessors
    //@{
    double GetSOC() const { return mCurrentSOC; }
    double GetRemainingEnergy() const { return mCurrentSOC * mBatteryCapacityWh; }
    double GetCurrentPower() const { return mCurrentPower; }
    double GetEnergyConsumed() const { return mEnergyConsumed; }
    double GetVoltage() const { return mTerminalVoltage; }
    double GetCurrent() const { return mCurrentAmps; }
    double GetVoltageSag() const { return mVoltageSag; }
    //@}

protected:
    //! Copy constructor for Clone()
    BatteryProcessor(const BatteryProcessor& aSrc);

private:
    //! Calculate open-circuit voltage from SOC using Li-Ion discharge curve
    double CalculateOpenCircuitVoltage(double soc) const;

    //! Calculate internal resistance (varies with SOC and temperature)
    double CalculateInternalResistance(double soc) const;

    //! Write a row to the CSV output file
    void WriteCSVRow(double simTime);

    //! Write final summary to CSV and log
    void WriteSummary();

    // Configuration parameters
    double mBatteryCapacityWh;      // Total capacity in Wh
    double mInitialSOC;             // Initial state of charge (0-1)
    double mHoverPowerW;            // Power consumption in hover (W) - fallback if dynamic disabled
    double mLowBatteryThreshold;    // SOC threshold for warning
    bool   mUseDynamicPower;        // If true, read power from power source aux data

    // Generic power source configuration (Phase 4)
    std::string mPowerSourceKey;    // AuxData key to read power from (default: "PROP_TOTAL_POWER")

    // Auxiliary systems power (Phase 6 - realistic power model)
    double mAuxiliaryPowerW;        // Constant power draw from avionics, radio, camera, ECU, etc.
    double mMotorEfficiency;        // Motor/ESC efficiency (0-1) - converts mechanical to electrical power
    double mMinFlightPowerFactor;   // Minimum flight power as fraction of hover (e.g., 0.5 = 50%)
    double mCruisePowerW;           // Power at cruise speed (for velocity-based estimation)
    double mCruiseSpeedMps;         // Reference cruise speed for power interpolation
    bool   mUseVelocityPowerModel;  // If true, estimate power from velocity when source is low

    // Voltage model parameters (13S Li-Ion pack for T-150)
    double mNominalVoltage;         // Nominal pack voltage (V) - default 46.8V (13S * 3.6V)
    double mFullChargeVoltage;      // Voltage at 100% SOC (V) - default 54.6V (13S * 4.2V)
    double mCutoffVoltage;          // Minimum safe voltage (V) - default 39.0V (13S * 3.0V)
    double mInternalResistance;     // Pack internal resistance (Ohms)
    double mCapacityAh;             // Pack capacity in Amp-hours
    double mVoltageSagWarning;      // Voltage threshold for sag warning (V)

    // Runtime state
    double mCurrentSOC;             // Current state of charge (0-1)
    double mCurrentPower;           // Current power draw (W)
    double mEnergyConsumed;         // Total energy consumed (Wh)
    double mLastUpdateTime;         // Time of last update
    bool   mLowWarningIssued;       // Has low battery warning been issued?

    // Voltage model state
    double mOpenCircuitVoltage;     // Open-circuit voltage at current SOC (V)
    double mTerminalVoltage;        // Actual terminal voltage under load (V)
    double mVoltageSag;             // Voltage drop due to internal resistance (V)
    double mCurrentAmps;            // Current draw (A)
    bool   mVoltageSagWarningIssued; // Has voltage sag warning been issued?

    // CSV output (Phase 4)
    std::string   mCsvOutputPath;   // Path to CSV output file
    std::ofstream mCsvFile;         // CSV file stream
    bool          mCsvEnabled;      // Whether CSV logging is enabled
    double        mCsvLogInterval;  // Interval between CSV rows (seconds)
    double        mLastCsvLogTime;  // Time of last CSV row written

    // Flight statistics (Phase 4)
    double mFlightStartTime;        // Simulation time when flight started
    double mSumPower;               // Sum of power samples for average calculation
    int    mPowerSampleCount;       // Number of power samples taken
};

} // namespace t150

#endif // T150_BATTERY_PROCESSOR_HPP
