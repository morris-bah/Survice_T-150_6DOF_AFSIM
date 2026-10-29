// ****************************************************************************
// T-150 UAS Vortex Ring State (VRS) Detection Plugin - Implementation
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "VRSProcessor.hpp"

#include <cmath>
#include <algorithm>

#include "UtInput.hpp"
#include "UtLog.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

namespace t150
{

// ============================================================================
// Constructor
VRSProcessor::VRSProcessor(const WsfScenario& aScenario)
    : WsfProcessor(aScenario),
      mVRSDescentThreshold(4.0),        // 4 m/s descent rate threshold
      mVRSSpeedThreshold(5.0),          // 5 m/s forward speed prevents VRS
      mCautionMargin(0.8),              // 80% of threshold for caution
      mInducedVelocity(8.0),            // Estimated hover induced velocity
      mCoaxialFactor(0.85),             // Coaxial rotors have some VRS resistance
      mVRSDevelopmentTime(2.0),         // 2 seconds to develop full VRS
      mVRSRecoveryTime(3.0),            // 3 seconds to recover
      mMaxLiftDegradation(0.6),         // Up to 60% lift loss in full VRS
      mMaxControlDegradation(0.5),      // Up to 50% control authority loss
      mVRSState(VRSState::CLEAR),
      mPreviousState(VRSState::CLEAR),
      mVRSMargin(1.0),
      mLiftFactor(1.0),
      mControlFactor(1.0),
      mTimeInState(0.0),
      mStateEntryTime(0.0),
      mLastUpdateTime(0.0),
      mVRSDevelopment(0.0),
      mCautionWarningIssued(false),
      mOnsetWarningIssued(false),
      mDevelopedWarningIssued(false)
{
    SetUpdateInterval(0.1);  // 100ms update rate
}

// ============================================================================
// Copy constructor for Clone()
VRSProcessor::VRSProcessor(const VRSProcessor& aSrc)
    : WsfProcessor(aSrc),
      mVRSDescentThreshold(aSrc.mVRSDescentThreshold),
      mVRSSpeedThreshold(aSrc.mVRSSpeedThreshold),
      mCautionMargin(aSrc.mCautionMargin),
      mInducedVelocity(aSrc.mInducedVelocity),
      mCoaxialFactor(aSrc.mCoaxialFactor),
      mVRSDevelopmentTime(aSrc.mVRSDevelopmentTime),
      mVRSRecoveryTime(aSrc.mVRSRecoveryTime),
      mMaxLiftDegradation(aSrc.mMaxLiftDegradation),
      mMaxControlDegradation(aSrc.mMaxControlDegradation),
      mVRSState(aSrc.mVRSState),
      mPreviousState(aSrc.mPreviousState),
      mVRSMargin(aSrc.mVRSMargin),
      mLiftFactor(aSrc.mLiftFactor),
      mControlFactor(aSrc.mControlFactor),
      mTimeInState(aSrc.mTimeInState),
      mStateEntryTime(aSrc.mStateEntryTime),
      mLastUpdateTime(aSrc.mLastUpdateTime),
      mVRSDevelopment(aSrc.mVRSDevelopment),
      mCautionWarningIssued(aSrc.mCautionWarningIssued),
      mOnsetWarningIssued(aSrc.mOnsetWarningIssued),
      mDevelopedWarningIssued(aSrc.mDevelopedWarningIssued)
{
}

// ============================================================================
// Get the class ID
WsfStringId VRSProcessor::GetVRSProcessorClassId()
{
    static WsfStringId classId("T150_VRS_PROCESSOR");
    return classId;
}

// ============================================================================
// Clone
WsfProcessor* VRSProcessor::Clone() const
{
    return new VRSProcessor(*this);
}

// ============================================================================
// Process input commands
bool VRSProcessor::ProcessInput(UtInput& aInput)
{
    bool myCommand = true;
    std::string command(aInput.GetCommand());

    if (command == "vrs_descent_threshold")
    {
        aInput.ReadValue(mVRSDescentThreshold);
        mVRSDescentThreshold = std::abs(mVRSDescentThreshold);
    }
    else if (command == "vrs_speed_threshold")
    {
        aInput.ReadValue(mVRSSpeedThreshold);
        mVRSSpeedThreshold = std::abs(mVRSSpeedThreshold);
    }
    else if (command == "caution_margin")
    {
        aInput.ReadValue(mCautionMargin);
        mCautionMargin = std::clamp(mCautionMargin, 0.5, 0.95);
    }
    else if (command == "induced_velocity")
    {
        aInput.ReadValue(mInducedVelocity);
        mInducedVelocity = std::abs(mInducedVelocity);
    }
    else if (command == "coaxial_factor")
    {
        aInput.ReadValue(mCoaxialFactor);
        mCoaxialFactor = std::clamp(mCoaxialFactor, 0.5, 1.0);
    }
    else if (command == "vrs_development_time")
    {
        aInput.ReadValue(mVRSDevelopmentTime);
        mVRSDevelopmentTime = std::max(0.5, mVRSDevelopmentTime);
    }
    else if (command == "vrs_recovery_time")
    {
        aInput.ReadValue(mVRSRecoveryTime);
        mVRSRecoveryTime = std::max(1.0, mVRSRecoveryTime);
    }
    else if (command == "max_lift_degradation")
    {
        aInput.ReadValue(mMaxLiftDegradation);
        mMaxLiftDegradation = std::clamp(mMaxLiftDegradation, 0.0, 1.0);
    }
    else if (command == "max_control_degradation")
    {
        aInput.ReadValue(mMaxControlDegradation);
        mMaxControlDegradation = std::clamp(mMaxControlDegradation, 0.0, 1.0);
    }
    else
    {
        myCommand = WsfProcessor::ProcessInput(aInput);
    }

    return myCommand;
}

// ============================================================================
// Initialize
bool VRSProcessor::Initialize(double aSimTime)
{
    bool ok = WsfProcessor::Initialize(aSimTime);
    if (ok)
    {
        mVRSState = VRSState::CLEAR;
        mPreviousState = VRSState::CLEAR;
        mVRSMargin = 1.0;
        mLiftFactor = 1.0;
        mControlFactor = 1.0;
        mTimeInState = 0.0;
        mStateEntryTime = aSimTime;
        mLastUpdateTime = aSimTime;
        mVRSDevelopment = 0.0;
        mCautionWarningIssued = false;
        mOnsetWarningIssued = false;
        mDevelopedWarningIssued = false;

        // Set auxiliary data on platform for script access
        GetPlatform()->GetAuxData().AssignDouble("VRS_MARGIN", mVRSMargin);
        GetPlatform()->GetAuxData().AssignInt("VRS_STATE", static_cast<int>(mVRSState));
        GetPlatform()->GetAuxData().AssignDouble("VRS_LIFT_FACTOR", mLiftFactor);
        GetPlatform()->GetAuxData().AssignDouble("VRS_CONTROL_FACTOR", mControlFactor);

        auto out = ut::log::info() << "VRS processor initialized.";
        out.AddNote() << "Platform: " << GetPlatform()->GetName();
        out.AddNote() << "Descent threshold: " << mVRSDescentThreshold << " m/s";
        out.AddNote() << "Speed threshold: " << mVRSSpeedThreshold << " m/s";
    }
    return ok;
}

// ============================================================================
// Update
void VRSProcessor::Update(double aSimTime)
{
    WsfProcessor::Update(aSimTime);

    if (!IsOperational())
        return;

    double deltaTime = aSimTime - mLastUpdateTime;
    if (deltaTime <= 0.0)
        return;

    // Get actual velocity from platform mover
    // WsfPlatform inherits from UtEntity which provides GetVelocityNED()
    // NED convention: [0]=North, [1]=East, [2]=Down (positive = descending)
    double velocityNED[3] = {0.0, 0.0, 0.0};
    GetPlatform()->GetVelocityNED(velocityNED);

    // Descent rate is positive when moving downward (NED convention)
    double descentRate = std::max(0.0, velocityNED[2]);

    // Forward/horizontal speed (ground speed in horizontal plane)
    double forwardSpeed = std::sqrt(velocityNED[0] * velocityNED[0] +
                                    velocityNED[1] * velocityNED[1]);

    // Compute VRS margin
    mVRSMargin = ComputeVRSMargin(descentRate, forwardSpeed);

    // Update VRS state machine
    UpdateVRSState(aSimTime, mVRSMargin);

    // Compute performance effects
    mLiftFactor = ComputeLiftFactor(mVRSMargin);
    mControlFactor = ComputeControlFactor(mVRSMargin);

    // Update time in state
    mTimeInState = aSimTime - mStateEntryTime;

    // Update aux data for script access
    GetPlatform()->GetAuxData().AssignDouble("VRS_MARGIN", mVRSMargin);
    GetPlatform()->GetAuxData().AssignInt("VRS_STATE", static_cast<int>(mVRSState));
    GetPlatform()->GetAuxData().AssignDouble("VRS_LIFT_FACTOR", mLiftFactor);
    GetPlatform()->GetAuxData().AssignDouble("VRS_CONTROL_FACTOR", mControlFactor);
    GetPlatform()->GetAuxData().AssignDouble("VRS_DEVELOPMENT", mVRSDevelopment);
    GetPlatform()->GetAuxData().AssignDouble("VRS_DESCENT_RATE", descentRate);
    GetPlatform()->GetAuxData().AssignDouble("VRS_FORWARD_SPEED", forwardSpeed);

    // Check for warnings
    CheckVRSWarnings(aSimTime);

    mLastUpdateTime = aSimTime;
}

// ============================================================================
// Compute VRS margin (0 = in VRS, 1 = completely clear)
double VRSProcessor::ComputeVRSMargin(double aDescentRate, double aForwardSpeed)
{
    double descentMargin = 1.0;
    double speedMargin = 1.0;

    // Descent margin: how far are we from the descent threshold?
    if (aDescentRate <= 0)
    {
        descentMargin = 1.0;  // Climbing or level - no VRS risk
    }
    else if (aDescentRate < mVRSDescentThreshold)
    {
        descentMargin = 1.0 - (aDescentRate / mVRSDescentThreshold);
    }
    else
    {
        // Beyond threshold - in potential VRS
        double excess = aDescentRate - mVRSDescentThreshold;
        descentMargin = -excess / mVRSDescentThreshold;
        descentMargin = std::max(-1.0, descentMargin);
    }

    // Speed margin: forward speed helps prevent VRS
    speedMargin = std::min(1.0, aForwardSpeed / mVRSSpeedThreshold);

    // Combined margin
    double margin;
    if (aForwardSpeed >= mVRSSpeedThreshold)
    {
        margin = 1.0;  // Enough forward speed to avoid VRS
    }
    else
    {
        double speedWeight = 1.0 - speedMargin;
        margin = descentMargin * speedWeight + speedMargin * (1.0 - speedWeight);
    }

    return std::clamp(margin, 0.0, 1.0);
}

// ============================================================================
// Update VRS state machine
void VRSProcessor::UpdateVRSState(double aSimTime, double aVRSMargin)
{
    double deltaTime = aSimTime - mLastUpdateTime;
    mPreviousState = mVRSState;

    switch (mVRSState)
    {
    case VRSState::CLEAR:
        if (aVRSMargin < mCautionMargin)
        {
            mVRSState = VRSState::CAUTION;
            mStateEntryTime = aSimTime;
            mVRSDevelopment = 0.0;
        }
        break;

    case VRSState::CAUTION:
        if (aVRSMargin >= 1.0)
        {
            mVRSState = VRSState::CLEAR;
            mStateEntryTime = aSimTime;
            mVRSDevelopment = 0.0;
        }
        else if (aVRSMargin < 0.5)
        {
            mVRSState = VRSState::ONSET;
            mStateEntryTime = aSimTime;
        }
        break;

    case VRSState::ONSET:
        if (aVRSMargin >= mCautionMargin)
        {
            mVRSState = VRSState::RECOVERY;
            mStateEntryTime = aSimTime;
        }
        else if (aVRSMargin < 0.2)
        {
            mVRSDevelopment += deltaTime / mVRSDevelopmentTime;
            if (mVRSDevelopment >= 1.0)
            {
                mVRSState = VRSState::DEVELOPED;
                mStateEntryTime = aSimTime;
                mVRSDevelopment = 1.0;
            }
        }
        break;

    case VRSState::DEVELOPED:
        if (aVRSMargin >= 0.5)
        {
            mVRSState = VRSState::RECOVERY;
            mStateEntryTime = aSimTime;
        }
        mVRSDevelopment = 1.0;
        break;

    case VRSState::RECOVERY:
        mVRSDevelopment -= deltaTime / mVRSRecoveryTime;
        if (mVRSDevelopment <= 0.0)
        {
            mVRSDevelopment = 0.0;
            if (aVRSMargin >= mCautionMargin)
            {
                mVRSState = VRSState::CLEAR;
                mStateEntryTime = aSimTime;
            }
            else
            {
                mVRSState = VRSState::CAUTION;
                mStateEntryTime = aSimTime;
            }
        }
        else if (aVRSMargin < 0.2)
        {
            mVRSState = VRSState::ONSET;
            mStateEntryTime = aSimTime;
        }
        break;
    }
}

// ============================================================================
// Compute lift degradation factor
double VRSProcessor::ComputeLiftFactor(double aVRSMargin)
{
    if (mVRSState == VRSState::CLEAR)
        return 1.0;

    double degradation = 0.0;

    switch (mVRSState)
    {
    case VRSState::CLEAR:
        degradation = 0.0;
        break;
    case VRSState::CAUTION:
        degradation = 0.05 * (1.0 - aVRSMargin);
        break;
    case VRSState::ONSET:
        degradation = 0.15 * mVRSDevelopment;
        break;
    case VRSState::DEVELOPED:
        degradation = mMaxLiftDegradation;
        break;
    case VRSState::RECOVERY:
        degradation = mMaxLiftDegradation * mVRSDevelopment;
        break;
    }

    return std::clamp(1.0 - degradation, 0.4, 1.0);
}

// ============================================================================
// Compute control authority degradation
double VRSProcessor::ComputeControlFactor(double aVRSMargin)
{
    if (mVRSState == VRSState::CLEAR)
        return 1.0;

    double degradation = 0.0;

    switch (mVRSState)
    {
    case VRSState::CLEAR:
        degradation = 0.0;
        break;
    case VRSState::CAUTION:
        degradation = 0.0;
        break;
    case VRSState::ONSET:
        degradation = 0.1 * mVRSDevelopment;
        break;
    case VRSState::DEVELOPED:
        degradation = mMaxControlDegradation;
        break;
    case VRSState::RECOVERY:
        degradation = mMaxControlDegradation * mVRSDevelopment;
        break;
    }

    return std::clamp(1.0 - degradation, 0.5, 1.0);
}

// ============================================================================
// Check and emit VRS warnings
void VRSProcessor::CheckVRSWarnings(double aSimTime)
{
    if (mVRSState >= VRSState::CAUTION && mVRSState != mPreviousState)
    {
        if (mVRSState == VRSState::DEVELOPED && !mDevelopedWarningIssued)
        {
            auto out = ut::log::warning() << "VRS DEVELOPED - TAKE RECOVERY ACTION!";
            out.AddNote() << "Platform: " << GetPlatform()->GetName();
            out.AddNote() << "VRS Margin: " << mVRSMargin;
            out.AddNote() << "Lift Factor: " << mLiftFactor;
            mDevelopedWarningIssued = true;
            GetPlatform()->GetAuxData().AssignBool("VRS_CRITICAL", true);
        }
        else if (mVRSState == VRSState::ONSET && !mOnsetWarningIssued)
        {
            auto out = ut::log::warning() << "VRS ONSET WARNING!";
            out.AddNote() << "Platform: " << GetPlatform()->GetName();
            out.AddNote() << "VRS Margin: " << mVRSMargin;
            out.AddNote() << "Reduce descent rate or increase forward speed.";
            mOnsetWarningIssued = true;
            GetPlatform()->GetAuxData().AssignBool("VRS_ONSET", true);
        }
        else if (mVRSState == VRSState::CAUTION && !mCautionWarningIssued)
        {
            auto out = ut::log::info() << "VRS Caution - approaching VRS envelope.";
            out.AddNote() << "Platform: " << GetPlatform()->GetName();
            out.AddNote() << "VRS Margin: " << mVRSMargin;
            mCautionWarningIssued = true;
            GetPlatform()->GetAuxData().AssignBool("VRS_CAUTION", true);
        }
    }

    // Clear warnings if we recover
    if (mVRSState == VRSState::CLEAR)
    {
        mCautionWarningIssued = false;
        mOnsetWarningIssued = false;
        mDevelopedWarningIssued = false;
        GetPlatform()->GetAuxData().AssignBool("VRS_CAUTION", false);
        GetPlatform()->GetAuxData().AssignBool("VRS_ONSET", false);
        GetPlatform()->GetAuxData().AssignBool("VRS_CRITICAL", false);
    }
}

// ============================================================================
// Get VRS state as string
const char* VRSProcessor::GetVRSStateString() const
{
    switch (mVRSState)
    {
    case VRSState::CLEAR:     return "CLEAR";
    case VRSState::CAUTION:   return "CAUTION";
    case VRSState::ONSET:     return "ONSET";
    case VRSState::DEVELOPED: return "DEVELOPED";
    case VRSState::RECOVERY:  return "RECOVERY";
    default:                  return "UNKNOWN";
    }
}

// ============================================================================
// Script class name
const char* VRSProcessor::GetScriptClassName() const
{
    return "T150VRSProcessor";
}

} // namespace t150
