// ****************************************************************************
// T-150 UAS Slung Load Dynamics Plugin - Implementation
//
// Spherical pendulum model for cable-suspended payload dynamics.
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#include "SlungLoadProcessor.hpp"

#include <cmath>
#include <algorithm>

#include "UtInput.hpp"
#include "UtLog.hpp"
#include "WsfPlatform.hpp"

namespace t150
{

// ============================================================================
// Constants
// ============================================================================
static constexpr double GRAVITY = 9.80665;          // m/s^2
static constexpr double DEG_TO_RAD = 0.017453292519943295;
static constexpr double RAD_TO_DEG = 57.29577951308232;
static constexpr double PI = 3.14159265358979323846;

// ============================================================================
// Static Class ID
// ============================================================================
WsfStringId SlungLoadProcessor::GetSlungLoadProcessorClassId()
{
    static WsfStringId classId("T150_SLUNG_LOAD_PROCESSOR");
    return classId;
}

// ============================================================================
// Constructor
// ============================================================================
SlungLoadProcessor::SlungLoadProcessor(const WsfScenario& aScenario)
    : WsfProcessor(aScenario)
    // Configuration defaults
    , mLoadMass(0.0)                    // No load by default
    , mCableLength(5.0)                 // 5 meter cable
    , mCableDamping(0.1)                // Light damping
    , mAttachPointX(0.0)                // Centered under CG
    , mAttachPointY(0.0)
    , mAttachPointZ(0.5)                // 0.5m below CG
    , mMaxCableTension(5000.0)          // 5000 N max tension
    , mResonanceThreshold(0.35)         // ~20 degrees
    , mUpdateInterval(0.02)             // 50 Hz update for dynamics
    // Dynamic state
    , mLoadState(SlungLoadState::DETACHED)
    , mThetaX(0.0)
    , mThetaY(0.0)
    , mThetaXDot(0.0)
    , mThetaYDot(0.0)
    , mCableTension(0.0)
    , mLoadForceX(0.0)
    , mLoadForceY(0.0)
    , mLoadForceZ(0.0)
    , mLoadPosX(0.0)
    , mLoadPosY(0.0)
    , mLoadPosZ(0.0)
    , mLastUpdateTime(0.0)
    , mLastVelX(0.0)
    , mLastVelY(0.0)
    , mLastVelZ(0.0)
    , mResonanceWarning(false)
    , mPeakSwingAngle(0.0)
    , mSwingFrequency(0.0)
    , mSwingCycleCount(0)
    , mLastZeroCrossTime(0.0)
    , mOverloadWarningIssued(false)
    , mResonanceWarningIssued(false)
{
}

// ============================================================================
// Copy Constructor
// ============================================================================
SlungLoadProcessor::SlungLoadProcessor(const SlungLoadProcessor& aSrc)
    : WsfProcessor(aSrc)
    , mLoadMass(aSrc.mLoadMass)
    , mCableLength(aSrc.mCableLength)
    , mCableDamping(aSrc.mCableDamping)
    , mAttachPointX(aSrc.mAttachPointX)
    , mAttachPointY(aSrc.mAttachPointY)
    , mAttachPointZ(aSrc.mAttachPointZ)
    , mMaxCableTension(aSrc.mMaxCableTension)
    , mResonanceThreshold(aSrc.mResonanceThreshold)
    , mUpdateInterval(aSrc.mUpdateInterval)
    , mLoadState(aSrc.mLoadState)
    , mThetaX(aSrc.mThetaX)
    , mThetaY(aSrc.mThetaY)
    , mThetaXDot(aSrc.mThetaXDot)
    , mThetaYDot(aSrc.mThetaYDot)
    , mCableTension(aSrc.mCableTension)
    , mLoadForceX(aSrc.mLoadForceX)
    , mLoadForceY(aSrc.mLoadForceY)
    , mLoadForceZ(aSrc.mLoadForceZ)
    , mLoadPosX(aSrc.mLoadPosX)
    , mLoadPosY(aSrc.mLoadPosY)
    , mLoadPosZ(aSrc.mLoadPosZ)
    , mLastUpdateTime(aSrc.mLastUpdateTime)
    , mLastVelX(aSrc.mLastVelX)
    , mLastVelY(aSrc.mLastVelY)
    , mLastVelZ(aSrc.mLastVelZ)
    , mResonanceWarning(aSrc.mResonanceWarning)
    , mPeakSwingAngle(aSrc.mPeakSwingAngle)
    , mSwingFrequency(aSrc.mSwingFrequency)
    , mSwingCycleCount(aSrc.mSwingCycleCount)
    , mLastZeroCrossTime(aSrc.mLastZeroCrossTime)
    , mOverloadWarningIssued(aSrc.mOverloadWarningIssued)
    , mResonanceWarningIssued(aSrc.mResonanceWarningIssued)
{
}

// ============================================================================
// Clone
// ============================================================================
WsfProcessor* SlungLoadProcessor::Clone() const
{
    return new SlungLoadProcessor(*this);
}

// ============================================================================
// GetScriptClassName
// ============================================================================
const char* SlungLoadProcessor::GetScriptClassName() const
{
    return "T150_SLUNG_LOAD_PROCESSOR";
}

// ============================================================================
// ProcessInput - Parse scenario configuration
// ============================================================================
bool SlungLoadProcessor::ProcessInput(UtInput& aInput)
{
    bool myCommand = true;
    std::string command(aInput.GetCommand());

    if (command == "load_mass")
    {
        aInput.ReadValue(mLoadMass);
        mLoadMass = std::max(0.0, mLoadMass);
        if (mLoadMass > 0.0)
        {
            mLoadState = SlungLoadState::ATTACHED;
        }
    }
    else if (command == "cable_length")
    {
        aInput.ReadValue(mCableLength);
        mCableLength = std::max(0.5, mCableLength);  // Min 0.5m
    }
    else if (command == "cable_damping")
    {
        aInput.ReadValue(mCableDamping);
        mCableDamping = std::max(0.0, mCableDamping);
    }
    else if (command == "attach_point_x")
    {
        aInput.ReadValue(mAttachPointX);
    }
    else if (command == "attach_point_y")
    {
        aInput.ReadValue(mAttachPointY);
    }
    else if (command == "attach_point_z")
    {
        aInput.ReadValue(mAttachPointZ);
    }
    else if (command == "max_cable_tension")
    {
        aInput.ReadValue(mMaxCableTension);
        mMaxCableTension = std::max(100.0, mMaxCableTension);
    }
    else if (command == "resonance_threshold")
    {
        // Input in degrees, store in radians
        double degrees = 0.0;
        aInput.ReadValue(degrees);
        mResonanceThreshold = degrees * DEG_TO_RAD;
    }
    else if (command == "update_interval")
    {
        // Input in seconds
        aInput.ReadValue(mUpdateInterval);
        mUpdateInterval = std::max(0.001, mUpdateInterval);  // Min 1ms
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
bool SlungLoadProcessor::Initialize(double aSimTime)
{
    bool ok = WsfProcessor::Initialize(aSimTime);

    if (ok)
    {
        mLastUpdateTime = aSimTime;
        mLastVelX = 0.0;
        mLastVelY = 0.0;
        mLastVelZ = 0.0;

        // Initialize pendulum at rest (hanging straight down)
        mThetaX = 0.0;
        mThetaY = 0.0;
        mThetaXDot = 0.0;
        mThetaYDot = 0.0;

        // Compute initial load position
        ComputeLoadPosition();

        // Set initial tension (just supporting the weight)
        if (mLoadMass > 0.0 && mLoadState == SlungLoadState::ATTACHED)
        {
            mCableTension = mLoadMass * GRAVITY;
            mLoadForceZ = mCableTension;  // Downward force on aircraft
        }

        // Publish initial aux data
        GetPlatform()->GetAuxData().AssignInt("SLUNG_LOAD_STATE", static_cast<int>(mLoadState));
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_LOAD_MASS", mLoadMass);
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_CABLE_LENGTH", mCableLength);
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_CABLE_TENSION", mCableTension);
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_SWING_X", mThetaX * RAD_TO_DEG);
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_SWING_Y", mThetaY * RAD_TO_DEG);
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_SWING_MAG", GetSwingMagnitude() * RAD_TO_DEG);
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_X", mLoadForceX);
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_Y", mLoadForceY);
        GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_Z", mLoadForceZ);
        GetPlatform()->GetAuxData().AssignBool("SLUNG_RESONANCE", mResonanceWarning);

        // Log initialization
        auto out = ut::log::info() << "Slung load processor initialized.";
        out.AddNote() << "Platform: " << GetPlatform()->GetName();
        if (mLoadMass > 0.0)
        {
            out.AddNote() << "Load mass: " << mLoadMass << " kg";
            out.AddNote() << "Cable length: " << mCableLength << " m";
            out.AddNote() << "Natural frequency: " << ComputeNaturalFrequency() << " Hz";
            out.AddNote() << "Initial tension: " << mCableTension << " N";
        }
        else
        {
            out.AddNote() << "No load attached";
        }
    }

    return ok;
}

// ============================================================================
// Update - Main dynamics loop
// ============================================================================
void SlungLoadProcessor::Update(double aSimTime)
{
    WsfProcessor::Update(aSimTime);

    if (!IsOperational())
        return;

    // Skip if no load attached
    if (mLoadState == SlungLoadState::DETACHED || mLoadState == SlungLoadState::RELEASED)
    {
        mLastUpdateTime = aSimTime;
        return;
    }

    double deltaTime = aSimTime - mLastUpdateTime;
    if (deltaTime < mUpdateInterval)
        return;  // Not time to update yet

    // Get aircraft velocity (NED frame)
    // NED convention: [0]=North, [1]=East, [2]=Down
    double velocityNED[3] = {0.0, 0.0, 0.0};
    GetPlatform()->GetVelocityNED(velocityNED);
    double velX = velocityNED[0];
    double velY = velocityNED[1];
    double velZ = velocityNED[2];

    // Compute aircraft acceleration from velocity change
    double accelX = (velX - mLastVelX) / deltaTime;
    double accelY = (velY - mLastVelY) / deltaTime;
    double accelZ = (velZ - mLastVelZ) / deltaTime;

    // Limit acceleration to reasonable values (filter noise)
    const double maxAccel = 50.0;  // m/s^2
    accelX = std::clamp(accelX, -maxAccel, maxAccel);
    accelY = std::clamp(accelY, -maxAccel, maxAccel);
    accelZ = std::clamp(accelZ, -maxAccel, maxAccel);

    // Store for next iteration
    mLastVelX = velX;
    mLastVelY = velY;
    mLastVelZ = velZ;

    // Update pendulum dynamics
    UpdatePendulumDynamics(deltaTime, accelX, accelY, accelZ);

    // Compute cable tension
    ComputeCableTension(accelZ);

    // Compute load position
    ComputeLoadPosition();

    // Check for resonance
    CheckResonance(deltaTime);

    // Check for cable overload
    if (mCableTension > mMaxCableTension)
    {
        if (!mOverloadWarningIssued)
        {
            auto warn = ut::log::warning() << "Cable tension exceeded maximum!";
            warn.AddNote() << "Platform: " << GetPlatform()->GetName();
            warn.AddNote() << "Tension: " << mCableTension << " N (max: " << mMaxCableTension << " N)";
            warn.AddNote() << "LOAD RELEASED!";
            mOverloadWarningIssued = true;
            ReleaseLoad();
        }
    }

    // Update load state based on swing magnitude
    double swingMag = GetSwingMagnitude();
    if (swingMag > mResonanceThreshold)
    {
        mLoadState = SlungLoadState::SWINGING;
    }
    else if (mLoadState == SlungLoadState::SWINGING && swingMag < mResonanceThreshold * 0.5)
    {
        mLoadState = SlungLoadState::ATTACHED;  // Stabilized
    }

    // Publish aux data
    GetPlatform()->GetAuxData().AssignInt("SLUNG_LOAD_STATE", static_cast<int>(mLoadState));
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_CABLE_TENSION", mCableTension);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_SWING_X", mThetaX * RAD_TO_DEG);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_SWING_Y", mThetaY * RAD_TO_DEG);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_SWING_MAG", swingMag * RAD_TO_DEG);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_X", mLoadForceX);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_Y", mLoadForceY);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_Z", mLoadForceZ);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_POS_X", mLoadPosX);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_POS_Y", mLoadPosY);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_POS_Z", mLoadPosZ);
    GetPlatform()->GetAuxData().AssignBool("SLUNG_RESONANCE", mResonanceWarning);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_NATURAL_FREQ", ComputeNaturalFrequency());

    mLastUpdateTime = aSimTime;
}

// ============================================================================
// UpdatePendulumDynamics - Spherical pendulum equations of motion
// ============================================================================
void SlungLoadProcessor::UpdatePendulumDynamics(double aDeltaTime,
                                                  double aAccelX,
                                                  double aAccelY,
                                                  double aAccelZ)
{
    // Spherical pendulum equations (small angle approximation with coupling)
    // For larger angles, we use the full nonlinear equations
    //
    // theta_x'' = -(g/L)*sin(theta_x) - (a_x/L)*cos(theta_x) - c*theta_x'
    // theta_y'' = -(g/L)*sin(theta_y) - (a_y/L)*cos(theta_y) - c*theta_y'
    //
    // Using semi-implicit Euler for stability

    double L = mCableLength;
    double c = mCableDamping;
    double gOverL = GRAVITY / L;

    // X-Z plane (pitch pendulum)
    // Aircraft acceleration in X excites this mode
    double sinThetaX = std::sin(mThetaX);
    double cosThetaX = std::cos(mThetaX);
    double thetaXDotDot = -gOverL * sinThetaX
                          - (aAccelX / L) * cosThetaX
                          - c * mThetaXDot;

    // Y-Z plane (roll pendulum)
    // Aircraft acceleration in Y excites this mode
    double sinThetaY = std::sin(mThetaY);
    double cosThetaY = std::cos(mThetaY);
    double thetaYDotDot = -gOverL * sinThetaY
                          - (aAccelY / L) * cosThetaY
                          - c * mThetaYDot;

    // Semi-implicit Euler integration (velocity first, then position)
    mThetaXDot += thetaXDotDot * aDeltaTime;
    mThetaYDot += thetaYDotDot * aDeltaTime;

    mThetaX += mThetaXDot * aDeltaTime;
    mThetaY += mThetaYDot * aDeltaTime;

    // Clamp angles to reasonable range (prevent numerical blowup)
    const double maxAngle = 1.2;  // ~70 degrees
    mThetaX = std::clamp(mThetaX, -maxAngle, maxAngle);
    mThetaY = std::clamp(mThetaY, -maxAngle, maxAngle);
}

// ============================================================================
// ComputeCableTension
// ============================================================================
void SlungLoadProcessor::ComputeCableTension(double aAccelZ)
{
    // Tension = m * (g - a_z + centripetal)
    // For a swinging pendulum:
    // T = m*g*cos(theta) + m*L*omega^2

    double swingMag = GetSwingMagnitude();
    double cosSwing = std::cos(swingMag);

    // Angular velocity magnitude
    double omegaSq = mThetaXDot * mThetaXDot + mThetaYDot * mThetaYDot;

    // Centripetal acceleration term
    double centripetal = mCableLength * omegaSq;

    // Total tension
    // Positive aAccelZ (descending faster) reduces tension
    // Negative aAccelZ (climbing/decelerating descent) increases tension
    mCableTension = mLoadMass * (GRAVITY * cosSwing - aAccelZ + centripetal);

    // Tension cannot be negative (cable can't push)
    mCableTension = std::max(0.0, mCableTension);

    // Compute force components on aircraft (reaction to cable tension)
    // In aircraft body frame (approximately NED for small attitudes)
    mLoadForceX = mCableTension * std::sin(mThetaX);  // Forward/back
    mLoadForceY = mCableTension * std::sin(mThetaY);  // Left/right
    mLoadForceZ = mCableTension * std::cos(swingMag); // Down (positive)
}

// ============================================================================
// ComputeLoadPosition - Position of load relative to aircraft CG
// ============================================================================
void SlungLoadProcessor::ComputeLoadPosition()
{
    // Load position in aircraft body frame
    // Starting from attach point, go down cable at current swing angles
    mLoadPosX = mAttachPointX + mCableLength * std::sin(mThetaX);
    mLoadPosY = mAttachPointY + mCableLength * std::sin(mThetaY);
    mLoadPosZ = mAttachPointZ + mCableLength * std::cos(GetSwingMagnitude());
}

// ============================================================================
// CheckResonance - Detect dangerous swing conditions
// ============================================================================
void SlungLoadProcessor::CheckResonance(double aDeltaTime)
{
    double swingMag = GetSwingMagnitude();

    // Track peak swing
    if (swingMag > mPeakSwingAngle)
    {
        mPeakSwingAngle = swingMag;
    }
    else
    {
        // Decay peak tracking
        mPeakSwingAngle *= 0.995;
    }

    // Zero crossing detection for frequency measurement
    static double lastThetaX = 0.0;
    if (lastThetaX * mThetaX < 0.0 && mThetaX > 0.0)  // Positive zero crossing
    {
        if (mLastZeroCrossTime > 0.0)
        {
            double period = mLastUpdateTime - mLastZeroCrossTime;
            if (period > 0.1)  // Reasonable period
            {
                mSwingFrequency = 1.0 / period;
            }
        }
        mLastZeroCrossTime = mLastUpdateTime;
        mSwingCycleCount++;
    }
    lastThetaX = mThetaX;

    // Check for resonance conditions
    double naturalFreq = ComputeNaturalFrequency();
    bool frequencyMatch = std::abs(mSwingFrequency - naturalFreq) < 0.1;  // Within 0.1 Hz
    bool largeSwing = mPeakSwingAngle > mResonanceThreshold;

    mResonanceWarning = frequencyMatch && largeSwing;

    if (mResonanceWarning && !mResonanceWarningIssued)
    {
        auto warn = ut::log::warning() << "Slung load resonance detected!";
        warn.AddNote() << "Platform: " << GetPlatform()->GetName();
        warn.AddNote() << "Swing magnitude: " << (mPeakSwingAngle * RAD_TO_DEG) << " deg";
        warn.AddNote() << "Swing frequency: " << mSwingFrequency << " Hz";
        warn.AddNote() << "Natural frequency: " << naturalFreq << " Hz";
        warn.AddNote() << "Reduce maneuver rate or change cable length";
        mResonanceWarningIssued = true;
        mLoadState = SlungLoadState::RESONANCE;
    }
    else if (!mResonanceWarning && mResonanceWarningIssued)
    {
        // Reset warning flag when resonance clears
        mResonanceWarningIssued = false;
    }
}

// ============================================================================
// ComputeNaturalFrequency
// ============================================================================
double SlungLoadProcessor::ComputeNaturalFrequency() const
{
    // f = (1/2*pi) * sqrt(g/L)
    return (1.0 / (2.0 * PI)) * std::sqrt(GRAVITY / mCableLength);
}

// ============================================================================
// GetSwingMagnitude
// ============================================================================
double SlungLoadProcessor::GetSwingMagnitude() const
{
    // Combined swing angle magnitude
    return std::sqrt(mThetaX * mThetaX + mThetaY * mThetaY);
}

// ============================================================================
// AttachLoad
// ============================================================================
void SlungLoadProcessor::AttachLoad(double aMass)
{
    mLoadMass = aMass;
    mLoadState = SlungLoadState::ATTACHED;
    mThetaX = 0.0;
    mThetaY = 0.0;
    mThetaXDot = 0.0;
    mThetaYDot = 0.0;
    mCableTension = mLoadMass * GRAVITY;
    mOverloadWarningIssued = false;
    mResonanceWarningIssued = false;

    GetPlatform()->GetAuxData().AssignDouble("SLUNG_LOAD_MASS", mLoadMass);
    GetPlatform()->GetAuxData().AssignInt("SLUNG_LOAD_STATE", static_cast<int>(mLoadState));

    auto info = ut::log::info() << "Load attached";
    info.AddNote() << "Platform: " << GetPlatform()->GetName();
    info.AddNote() << "Mass: " << mLoadMass << " kg";
}

// ============================================================================
// ReleaseLoad
// ============================================================================
void SlungLoadProcessor::ReleaseLoad()
{
    mLoadState = SlungLoadState::RELEASED;
    mCableTension = 0.0;
    mLoadForceX = 0.0;
    mLoadForceY = 0.0;
    mLoadForceZ = 0.0;

    GetPlatform()->GetAuxData().AssignInt("SLUNG_LOAD_STATE", static_cast<int>(mLoadState));
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_CABLE_TENSION", 0.0);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_X", 0.0);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_Y", 0.0);
    GetPlatform()->GetAuxData().AssignDouble("SLUNG_FORCE_Z", 0.0);

    auto info = ut::log::info() << "Load released";
    info.AddNote() << "Platform: " << GetPlatform()->GetName();
    info.AddNote() << "Load mass was: " << mLoadMass << " kg";
}

} // namespace t150
