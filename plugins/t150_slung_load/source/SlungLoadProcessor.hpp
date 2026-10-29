// ****************************************************************************
// T-150 UAS Slung Load Dynamics Plugin
//
// This plugin models a cable-suspended payload beneath the T-150 UAS.
// It implements a spherical pendulum model with:
// - 2-DOF swing angles (theta_x, theta_y)
// - Cable tension calculation
// - Damping from aerodynamic drag
// - Force coupling to aircraft via AuxData
//
// Physics Model:
// The slung load is modeled as a point mass on an inextensible cable.
// Equations of motion derived from Lagrangian mechanics:
//   theta_x'' = -(g/L)*sin(theta_x) - (a_x/L)*cos(theta_x) - c*theta_x'
//   theta_y'' = -(g/L)*sin(theta_y) - (a_y/L)*cos(theta_y) - c*theta_y'
// Where:
//   g = gravity, L = cable length, a_x/a_y = aircraft acceleration
//   c = damping coefficient (from aerodynamic drag)
//
// Copyright 2025 - Project 005 T-150 UAS Simulation
// ****************************************************************************

#ifndef T150_SLUNG_LOAD_PROCESSOR_HPP
#define T150_SLUNG_LOAD_PROCESSOR_HPP

#include "WsfProcessor.hpp"

namespace t150
{

//! Slung load state enumeration
enum class SlungLoadState
{
    DETACHED,       // No load attached
    ATTACHED,       // Load attached and stable
    SWINGING,       // Load in significant motion
    RESONANCE,      // Dangerous resonance detected
    RELEASED        // Load intentionally released
};

//! A processor that models slung load dynamics for the T-150 UAS.
class SlungLoadProcessor : public WsfProcessor
{
public:
    //! Constructor
    explicit SlungLoadProcessor(const WsfScenario& aScenario);

    //! Destructor
    ~SlungLoadProcessor() override = default;

    //! Get the class ID for this processor type
    static WsfStringId GetSlungLoadProcessorClassId();

    //! @name Framework methods
    //@{
    WsfProcessor* Clone() const override;
    bool          ProcessInput(UtInput& aInput) override;
    bool          Initialize(double aSimTime) override;
    void          Update(double aSimTime) override;
    const char*   GetScriptClassName() const override;
    //@}

    //! @name Load State Accessors
    //@{
    SlungLoadState GetLoadState() const { return mLoadState; }
    double GetLoadMass() const { return mLoadMass; }
    double GetCableLength() const { return mCableLength; }
    double GetCableTension() const { return mCableTension; }
    double GetSwingAngleX() const { return mThetaX; }
    double GetSwingAngleY() const { return mThetaY; }
    double GetSwingMagnitude() const;
    bool IsResonanceWarning() const { return mResonanceWarning; }
    //@}

    //! @name Load Control
    //@{
    void AttachLoad(double aMass);
    void ReleaseLoad();
    //@}

    //! @name Force Outputs (for propulsion coupling)
    //@{
    double GetLoadForceX() const { return mLoadForceX; }
    double GetLoadForceY() const { return mLoadForceY; }
    double GetLoadForceZ() const { return mLoadForceZ; }
    //@}

protected:
    //! Copy constructor for Clone()
    SlungLoadProcessor(const SlungLoadProcessor& aSrc);

private:
    //! Compute pendulum dynamics for one timestep
    void UpdatePendulumDynamics(double aDeltaTime, double aAccelX, double aAccelY, double aAccelZ);

    //! Compute cable tension from current state
    void ComputeCableTension(double aAccelZ);

    //! Compute load position relative to aircraft
    void ComputeLoadPosition();

    //! Check for resonance conditions
    void CheckResonance(double aDeltaTime);

    //! Compute natural frequency of the pendulum
    double ComputeNaturalFrequency() const;

    // =========================================================================
    // Configuration parameters
    // =========================================================================
    double mLoadMass;               // Payload mass (kg)
    double mCableLength;            // Cable length (m)
    double mCableDamping;           // Damping coefficient (1/s) - aerodynamic drag
    double mAttachPointX;           // Attachment point offset from CG - forward (m)
    double mAttachPointY;           // Attachment point offset from CG - right (m)
    double mAttachPointZ;           // Attachment point offset from CG - down (m)
    double mMaxCableTension;        // Maximum cable tension before failure (N)
    double mResonanceThreshold;     // Swing angle threshold for resonance warning (rad)
    double mUpdateInterval;         // Update interval (seconds)

    // =========================================================================
    // Dynamic state
    // =========================================================================
    SlungLoadState mLoadState;      // Current load state

    // Pendulum angles (radians) - measured from vertical
    double mThetaX;                 // Swing angle in X-Z plane (pitch)
    double mThetaY;                 // Swing angle in Y-Z plane (roll)
    double mThetaXDot;              // Angular velocity X (rad/s)
    double mThetaYDot;              // Angular velocity Y (rad/s)

    // Computed values
    double mCableTension;           // Current cable tension (N)
    double mLoadForceX;             // Load force on aircraft - X (N)
    double mLoadForceY;             // Load force on aircraft - Y (N)
    double mLoadForceZ;             // Load force on aircraft - Z (N)
    double mLoadPosX;               // Load position relative to aircraft CG (m)
    double mLoadPosY;
    double mLoadPosZ;

    // Tracking
    double mLastUpdateTime;
    double mLastVelX;               // Previous velocity for acceleration calc
    double mLastVelY;
    double mLastVelZ;

    // Resonance detection
    bool   mResonanceWarning;
    double mPeakSwingAngle;         // Maximum swing in recent history
    double mSwingFrequency;         // Measured swing frequency
    int    mSwingCycleCount;        // Count of swing cycles for frequency measurement
    double mLastZeroCrossTime;      // Time of last zero crossing

    // Warning flags
    bool   mOverloadWarningIssued;
    bool   mResonanceWarningIssued;
};

} // namespace t150

#endif // T150_SLUNG_LOAD_PROCESSOR_HPP
