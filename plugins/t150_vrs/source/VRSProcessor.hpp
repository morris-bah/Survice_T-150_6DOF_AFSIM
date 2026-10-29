// ****************************************************************************
// T-150 UAS Vortex Ring State (VRS) Detection Plugin
//
// This plugin provides real-time VRS detection and modeling for rotorcraft
// simulations. VRS occurs when a rotorcraft descends into its own downwash,
// causing loss of effective lift and potential loss of control.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_VRS_PROCESSOR_HPP
#define T150_VRS_PROCESSOR_HPP

#include "processor/WsfProcessor.hpp"

namespace t150
{

//! VRS state levels
enum class VRSState
{
    CLEAR,      // No VRS risk
    CAUTION,    // Approaching VRS envelope
    ONSET,      // Beginning to enter VRS
    DEVELOPED,  // Full VRS condition
    RECOVERY    // Exiting VRS
};

//! A processor that detects and models Vortex Ring State (VRS) conditions.
class VRSProcessor : public WsfProcessor
{
public:
    //! Constructor
    explicit VRSProcessor(const WsfScenario& aScenario);

    //! Destructor
    ~VRSProcessor() override = default;

    //! Get the class ID for this processor type
    static WsfStringId GetVRSProcessorClassId();

    //! @name Framework methods
    //@{
    WsfProcessor* Clone() const override;
    bool          ProcessInput(UtInput& aInput) override;
    bool          Initialize(double aSimTime) override;
    void          Update(double aSimTime) override;
    const char*   GetScriptClassName() const override;
    //@}

    //! @name VRS State Accessors
    //@{
    VRSState GetVRSState() const { return mVRSState; }
    const char* GetVRSStateString() const;
    double GetVRSMargin() const { return mVRSMargin; }
    double GetLiftFactor() const { return mLiftFactor; }
    double GetControlFactor() const { return mControlFactor; }
    bool IsVRSWarning() const { return mVRSState >= VRSState::CAUTION; }
    bool IsInVRS() const { return mVRSState == VRSState::DEVELOPED; }
    double GetTimeInState() const { return mTimeInState; }
    //@}

protected:
    //! Copy constructor for Clone()
    VRSProcessor(const VRSProcessor& aSrc);

private:
    //! Compute VRS margin based on current flight state
    double ComputeVRSMargin(double aDescentRate, double aForwardSpeed);

    //! Update VRS state machine
    void UpdateVRSState(double aSimTime, double aVRSMargin);

    //! Compute lift degradation factor
    double ComputeLiftFactor(double aVRSMargin);

    //! Compute control authority degradation
    double ComputeControlFactor(double aVRSMargin);

    //! Check and emit VRS warnings
    void CheckVRSWarnings(double aSimTime);

    // Configuration parameters (using plain doubles for API compatibility)
    double mVRSDescentThreshold;   // Descent rate for VRS onset (m/s)
    double mVRSSpeedThreshold;     // Forward speed that prevents VRS (m/s)
    double mCautionMargin;         // Fraction for caution warning
    double mInducedVelocity;       // Hover induced velocity (m/s)
    double mCoaxialFactor;         // VRS resistance for coaxial config
    double mVRSDevelopmentTime;    // Time to develop full VRS (s)
    double mVRSRecoveryTime;       // Time to recover from VRS (s)
    double mMaxLiftDegradation;    // Maximum lift loss in VRS (0-1)
    double mMaxControlDegradation; // Maximum control loss in VRS (0-1)

    // Runtime state
    VRSState mVRSState;          // Current VRS state
    VRSState mPreviousState;     // Previous VRS state
    double   mVRSMargin;         // Current VRS margin (0-1)
    double   mLiftFactor;        // Current lift factor (0-1)
    double   mControlFactor;     // Current control factor (0-1)
    double   mTimeInState;       // Time in current state (s)
    double   mStateEntryTime;    // Time when current state was entered
    double   mLastUpdateTime;    // Time of last update
    double   mVRSDevelopment;    // VRS development level (0-1)
    bool     mCautionWarningIssued;
    bool     mOnsetWarningIssued;
    bool     mDevelopedWarningIssued;
};

} // namespace t150

#endif // T150_VRS_PROCESSOR_HPP
