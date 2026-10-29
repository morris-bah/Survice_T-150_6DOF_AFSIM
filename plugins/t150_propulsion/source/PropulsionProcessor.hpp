// ****************************************************************************
// T-150 UAS Electric Propulsion System Plugin
//
// This plugin models the electric motor, ESC, and propeller system for
// the T-150 coaxial octocopter. It provides detailed propulsion state
// including per-motor current, temperature, and efficiency.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_PROPULSION_PROCESSOR_HPP
#define T150_PROPULSION_PROCESSOR_HPP

#include "processor/WsfProcessor.hpp"

#include <array>
#include <fstream>
#include <string>

namespace t150
{

//! Number of motors in T-150 (coaxial octocopter)
constexpr int NUM_MOTORS = 8;

//! Motor state enumeration
enum class MotorState
{
    OFF,
    STARTING,
    RUNNING,
    OVERTEMP,
    FAILED
};

//! Individual motor state data
struct MotorData
{
    MotorState state = MotorState::OFF;
    double rpm = 0.0;              // Current RPM
    double current = 0.0;          // Current draw (A)
    double temperature = 25.0;     // Winding temperature (C)
    double efficiency = 0.85;      // Current efficiency
    double thrust = 0.0;           // Thrust output (N)
    double torque = 0.0;           // Torque output (Nm)
    double throttle = 0.0;         // Commanded throttle (0-1)
};

//! A processor that models the electric propulsion system.
class PropulsionProcessor : public WsfProcessor
{
public:
    //! Constructor
    explicit PropulsionProcessor(const WsfScenario& aScenario);

    //! Destructor
    ~PropulsionProcessor() override = default;

    //! Get the class ID for this processor type
    static WsfStringId GetPropulsionProcessorClassId();

    //! @name Framework methods
    //@{
    WsfProcessor* Clone() const override;
    bool          ProcessInput(UtInput& aInput) override;
    bool          Initialize(double aSimTime) override;
    void          Update(double aSimTime) override;
    const char*   GetScriptClassName() const override;
    //@}

    //! @name System State Accessors
    //@{
    double GetTotalThrust() const { return mTotalThrust; }
    double GetTotalPower() const { return mTotalPower; }
    double GetSystemEfficiency() const { return mSystemEfficiency; }
    int GetOperationalMotorCount() const;
    const MotorData& GetMotorData(int aIndex) const;
    double GetAverageMotorTemperature() const;
    double GetMaxMotorTemperature() const;
    bool HasMotorFailure() const;
    bool IsThermalWarning() const;
    //@}

    //! @name Motor Control
    //@{
    void FailMotor(int aIndex);
    void RestoreMotor(int aIndex);
    //@}

protected:
    //! Copy constructor for Clone()
    PropulsionProcessor(const PropulsionProcessor& aSrc);

private:
    //! Compute required thrust based on flight state
    double ComputeRequiredThrust(double aSimTime);

    //! Distribute thrust across motors
    void DistributeThrust(double aTotalThrust);

    //! Update individual motor state
    void UpdateMotor(int aIndex, double aDeltaTime);

    //! Compute motor efficiency at operating point
    double ComputeMotorEfficiency(double aRPM, double aTorque);

    //! Update motor temperature
    void UpdateMotorTemperature(int aIndex, double aDeltaTime, double aPower);

    //! Check motor health
    void CheckMotorHealth(int aIndex, double aSimTime);

    //! Compute ground effect factor based on altitude AGL
    double ComputeGroundEffectFactor(double aAltitudeAGL);

    //! Compute dynamic power based on flight state (Phase 6)
    double ComputeDynamicPower();

    // Configuration parameters (plain doubles for API compatibility)
    int    mMotorCount;             // Number of motors
    double mKvRating;               // Motor KV (RPM/V)
    double mMaxPowerPerMotor;       // Max motor power (W)
    double mPropellerDiameter;      // Propeller diameter (m)
    double mCoaxialEfficiency;      // Coaxial interference factor
    double mMotorThermalLimit;      // Motor temp limit (C)
    double mESCThermalLimit;        // ESC temp limit (C)
    double mMotorThermalMass;       // Motor thermal mass (J/K)
    double mMotorCoolingRate;       // Motor cooling rate (W/K)
    double mPeakMotorEfficiency;    // Peak motor efficiency
    double mNominalVoltage;         // System voltage (V)
    double mThrustConstant;         // Thrust coefficient
    double mTorqueConstant;         // Torque coefficient
    double mMotorTimeConstant;      // Motor spin-up/down time constant (sec)

    // Ground effect parameters (Phase 4)
    bool   mGroundEffectEnabled;    // Enable ground effect modeling
    double mRotorHeight;            // Height of rotors above ground when landed (m)
    double mGroundEffectMaxBoost;   // Maximum thrust boost factor in ground effect

    // Payload dynamics parameters (Phase 4)
    double mEmptyMass;              // Empty aircraft mass (kg)
    double mPayloadMass;            // Current payload mass (kg)
    double mMaxPayloadMass;         // Maximum payload capacity (kg)
    double mPayloadCgOffsetX;       // Payload CG offset forward from aircraft CG (m)
    double mPayloadCgOffsetY;       // Payload CG offset right from aircraft CG (m)
    double mPayloadCgOffsetZ;       // Payload CG offset down from aircraft CG (m)

    // Runtime state
    double mGroundEffectFactor;     // Current ground effect multiplier (1.0 = no effect)
    double mCurrentAltitudeAGL;     // Current altitude above ground (m)
    std::array<MotorData, NUM_MOTORS> mMotors;
    double mTotalThrust;
    double mTotalPower;
    double mSystemEfficiency;
    double mAmbientTemperature;
    double mLastUpdateTime;
    bool   mThermalWarningIssued;
    bool   mMotorFailureWarningIssued;

    // Payload runtime state (Phase 4)
    double mTotalMass;              // Current total mass (empty + payload) (kg)
    double mCombinedCgX;            // Combined CG X offset (m)
    double mCombinedCgY;            // Combined CG Y offset (m)
    double mCombinedCgZ;            // Combined CG Z offset (m)
    double mThrustMargin;           // Available thrust margin (0-1, 0=at limit)

    // Dynamic flight state (Phase 6 - Dynamic Power Model)
    double mLastVelNorth;           // Previous velocity North (m/s)
    double mLastVelEast;            // Previous velocity East (m/s)
    double mLastVelDown;            // Previous velocity Down (m/s)
    double mCurrentAccelX;          // Current acceleration X/North (m/s^2)
    double mCurrentAccelY;          // Current acceleration Y/East (m/s^2)
    double mCurrentAccelZ;          // Current acceleration Z/Down (m/s^2)
    double mForwardSpeed;           // Horizontal speed (m/s)
    double mClimbRate;              // Vertical speed, positive=up (m/s)
    double mBankAngle;              // Estimated bank angle (rad)

    // Aerodynamic parameters (Phase 6)
    double mDragCoefficient;        // Drag coefficient (default 1.0)
    double mFrontalArea;            // Frontal area (m^2, default 0.30)
    double mInducedPowerFactor;     // Non-ideal rotor factor (default 1.15)

    // CSV output
    std::string mCsvOutputPath;
    std::ofstream mCsvFile;
    bool mCsvEnabled;
    double mCsvLogInterval;
    double mLastCsvLogTime;

    //! Write a row to the CSV output file
    void WriteCSVRow(double aSimTime);

public:
    //! @name Payload Accessors (Phase 4)
    //@{
    double GetTotalMass() const { return mTotalMass; }
    double GetPayloadMass() const { return mPayloadMass; }
    double GetThrustMargin() const { return mThrustMargin; }
    double GetCombinedCgX() const { return mCombinedCgX; }
    double GetCombinedCgY() const { return mCombinedCgY; }
    double GetCombinedCgZ() const { return mCombinedCgZ; }
    bool IsOverloaded() const { return mPayloadMass > mMaxPayloadMass; }
    //@}

    //! @name Dynamic Flight State Accessors (Phase 6)
    //@{
    double GetForwardSpeed() const { return mForwardSpeed; }
    double GetClimbRate() const { return mClimbRate; }
    double GetBankAngle() const { return mBankAngle; }
    //@}
};

} // namespace t150

#endif // T150_PROPULSION_PROCESSOR_HPP
