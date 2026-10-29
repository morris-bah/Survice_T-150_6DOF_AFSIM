# T-150 UAS Plugin Enhancement Roadmap

## Document Purpose
This roadmap documents the enhancement phases for the T-150 engineering simulation plugins. All planned phases have been completed.

---

## Current State Summary (v2.0 - All Phases Complete)

### What Works Now
| Capability | Status | Phase | Accuracy |
|------------|--------|-------|----------|
| Battery SOC Tracking | **Complete** | 1 | Excellent - matches theoretical |
| Motor Thermal Modeling | **Complete** | 1 | Tuned - thermal equilibrium during hover |
| Motor Time Constants | **Complete** | 1 | 150ms first-order lag for actuator dynamics |
| VRS State Machine | **Complete** | 1 | Reads actual velocity from platform mover |
| Voltage Sag Model | **Complete** | 2 | Li-Ion discharge curve + SOC-dependent resistance |
| Dynamic Power Model | **Complete** | 3 | Battery reads actual power from propulsion |
| Ground Effect | **Complete** | 4 | Cheeseman-Bennett approximation |
| Payload Dynamics | **Complete** | 4 | Mass, CG shift, thrust margin |
| Slung Load | **Complete** | 5 | 2-DOF spherical pendulum dynamics |
| 6DOF Mover | **Complete** | 6 | Physics-based rotorcraft mover |
| Velocity-Based Power | **Complete** | 6 | Power varies with flight state |

### All Issues Resolved
1. ~~Thermal model too aggressive~~ - **FIXED Phase 1**: Thermal mass 1500 J/K, cooling rate 20 W/K
2. ~~VRS uses placeholder values~~ - **FIXED Phase 1**: Reads GetVelocityNED() from platform
3. ~~No motor time constants~~ - **FIXED Phase 1**: 150ms first-order lag
4. ~~No voltage sag~~ - **FIXED Phase 2**: Full Li-Ion voltage model with OCV + IR drop
5. ~~Fixed hover power~~ - **FIXED Phase 3**: Battery reads PROP_TOTAL_POWER from propulsion
6. ~~Fixed mass assumption~~ - **FIXED Phase 4**: Payload mass and CG dynamics
7. ~~No ground effect~~ - **FIXED Phase 4**: Up to 30% thrust boost near ground
8. ~~No slung load~~ - **FIXED Phase 5**: Pendulum dynamics with tension modeling
9. ~~Unrealistic range~~ - **FIXED Phase 6**: Velocity-based power model (36km matches 35-45km spec)

---

## Priority 1: Critical Enhancements (High Impact, Moderate Effort)

### 1.1 Fix Thermal Model Parameters
**Problem:** Motors reach thermal limit in 27 seconds instead of stabilizing.
**Root Cause:** Thermal mass (200 J/K) and cooling rate (5 W/K) are unrealistic for large motors.

**Solution:**
```cpp
// Current values (too aggressive)
mMotorThermalMass(200.0),    // J/K - way too low for U15II
mMotorCoolingRate(5.0),       // W/K - too low

// Proposed values (based on U15II specs)
mMotorThermalMass(2000.0),   // J/K - ~2kg motor mass * 1000 J/kg/K
mMotorCoolingRate(25.0),      // W/K - forced convection from prop wash
```

**Expected Result:** Thermal equilibrium at ~80-90C during sustained hover at 64% load.

**Files to Modify:**
- `plugins/t150_propulsion/source/PropulsionProcessor.cpp` (lines 36-37)

---

### 1.2 Battery Voltage Sag Model
**Problem:** Cannot predict emergency descent capability at low SOC.
**Why It Matters:** At 30% SOC under max load, voltage drops significantly, reducing available torque.

**Solution - Add to BatteryProcessor:**
```cpp
// New member variables
double mInternalResistance;      // Ohms (pack level)
double mNominalVoltage;          // Volts (fully charged)
double mCutoffVoltage;           // Volts (minimum safe)
double mCurrentVoltage;          // Computed each update

// Voltage computation
double ComputeVoltage(double aSOC, double aCurrent) {
    // Open-circuit voltage from SOC (Li-Ion curve)
    double ocv = mNominalVoltage * (0.9 + 0.1 * aSOC);

    // Voltage drop from internal resistance
    double vDrop = aCurrent * mInternalResistance;

    // Apply temperature derating if cold
    double tempFactor = std::clamp(1.0 - 0.005 * (25.0 - mTemperature), 0.7, 1.0);

    return std::max(mCutoffVoltage, (ocv - vDrop) * tempFactor);
}
```

**Scenario Parameters:**
```
battery_internal_resistance  0.05    # 50 milliohms for 13S pack
battery_nominal_voltage      54.6    # 13S fully charged (4.2V/cell)
battery_cutoff_voltage       39.0    # 13S minimum (3.0V/cell)
```

**Files to Modify:**
- `plugins/t150_battery/source/BatteryProcessor.hpp`
- `plugins/t150_battery/source/BatteryProcessor.cpp`

---

### 1.3 Motor Time Constants (Actuator Lag)
**Problem:** RPM changes are instantaneous; real motors have rotational inertia.
**Why It Matters:** Fast control loops assume instant response; large motors lag by 100-200ms.

**Solution - Add first-order lag to PropulsionProcessor:**
```cpp
// New member variable
double mMotorTimeConstant;  // seconds (tau)

// In UpdateMotor():
double targetRPM = noLoadRPM * 0.85;

// First-order lag: dRPM/dt = (target - current) / tau
double rpmRate = (targetRPM - motor.rpm) / mMotorTimeConstant;
motor.rpm += rpmRate * aDeltaTime;
motor.rpm = std::clamp(motor.rpm, 0.0, maxRPM);
```

**Scenario Parameter:**
```
motor_time_constant  0.15   # 150ms for U15II with 30" prop
```

**Files to Modify:**
- `plugins/t150_propulsion/source/PropulsionProcessor.hpp`
- `plugins/t150_propulsion/source/PropulsionProcessor.cpp`

---

### 1.4 VRS Mover Integration
**Problem:** VRS processor uses hardcoded values (descentRate=0, forwardSpeed=10).
**Why It Matters:** Cannot detect actual VRS conditions during descent maneuvers.

**Solution - Read velocity from platform mover:**
```cpp
void VRSProcessor::Update(double aSimTime) {
    // Get platform velocity from mover
    WsfPlatform* platform = GetPlatform();

    // Get velocity in local frame (NED)
    double vn = 0.0, ve = 0.0, vd = 0.0;

    // Access mover if available
    if (platform->Mover()) {
        WsfGeoPoint vel = platform->Mover()->Velocity();
        vn = vel.Latitude();   // North velocity
        ve = vel.Longitude();  // East velocity
        vd = -vel.Altitude();  // Down velocity (positive = descending)
    }

    // Compute descent rate (positive = descending)
    double descentRate = std::max(0.0, vd);

    // Compute forward speed (horizontal)
    double forwardSpeed = std::sqrt(vn*vn + ve*ve);

    // Use actual values for VRS margin
    mVRSMargin = ComputeVRSMargin(descentRate, forwardSpeed);
}
```

**Files to Modify:**
- `plugins/t150_vrs/source/VRSProcessor.cpp` (lines 199-203)

---

## Priority 2: Significant Enhancements (Medium Impact, Higher Effort)

### 2.1 Payload Mass and CoG Model
**Problem:** Fixed 123 kg mass assumption; no CoG shift with payload.

**Solution - Add PayloadProcessor or extend PropulsionProcessor:**
```cpp
// Configuration
double mEmptyMass;           // kg (airframe only)
double mPayloadMass;         // kg (current payload)
double mMaxPayloadMass;      // kg (structural limit)
double mPayloadArmX;         // meters from CoG
double mPayloadArmY;
double mPayloadArmZ;

// Computed
double mTotalMass;
double mCoGShiftX, mCoGShiftY, mCoGShiftZ;

// MoI tensor (simplified principal axes)
double mIxx, mIyy, mIzz;     // kg*m^2

void UpdateMassProperties() {
    mTotalMass = mEmptyMass + mPayloadMass;

    // CoG shift
    double payloadFraction = mPayloadMass / mTotalMass;
    mCoGShiftX = mPayloadArmX * payloadFraction;
    // ... etc

    // Update MoI (simplified - payload as point mass)
    double r2 = mPayloadArmX*mPayloadArmX + mPayloadArmY*mPayloadArmY;
    mIzz = mEmptyIzz + mPayloadMass * r2;
}
```

---

### 2.2 Ground Effect Model
**Problem:** No thrust augmentation near ground.

**Solution:**
```cpp
double ComputeGroundEffectFactor(double aAltitudeAGL) {
    // h/D ratio (altitude / rotor diameter)
    double hD = aAltitudeAGL / mRotorDiameter;

    // Ground effect significant below h/D = 1.0
    if (hD >= 1.5) return 1.0;  // No ground effect

    // Cheeseman-Bennett approximation
    double geFactor = 1.0 / (1.0 - 0.25 * std::pow(mRotorDiameter / (4.0 * aAltitudeAGL), 2));

    return std::clamp(geFactor, 1.0, 1.3);  // Up to 30% thrust boost
}
```

---

### 2.3 Dynamic Power Model
**Problem:** Battery uses fixed hover power; doesn't vary with flight phase.

**Solution - Read power from propulsion processor:**
```cpp
// In BatteryProcessor::Update()
double actualPower = mHoverPowerW;  // Default

// Try to read from propulsion processor aux data
if (GetPlatform()->GetAuxData().IsDouble("PROP_TOTAL_POWER")) {
    actualPower = GetPlatform()->GetAuxData().Double("PROP_TOTAL_POWER");
}

mCurrentPower = actualPower;
```

---

## Priority 3: Advanced Enhancements (Lower Priority, High Effort)

### 3.1 Slung Load Dynamics
- Cable tension vector modeling
- Pendulum motion (2-DOF swing)
- Load coupling to airframe attitude

### 3.2 BEMT Rotor Model
- Blade Element Momentum Theory for thrust
- Inflow modeling
- Tip loss corrections

### 3.3 Sensor Noise Injection
- IMU noise (gyro bias, accel noise)
- GPS drift and latency
- Barometer noise

### 3.4 Structural Flexibility
- Arm bending modes
- Vibration coupling to IMU

---

## Implementation Order (All Complete)

| Phase | Enhancement | Status | Commit |
|-------|-------------|--------|--------|
| **Phase 1** | Thermal parameters fix | ✅ Complete | 1.1 |
| **Phase 1** | VRS mover integration | ✅ Complete | 1.1 |
| **Phase 1** | Motor time constants | ✅ Complete | 1.1 |
| **Phase 2** | Voltage sag model | ✅ Complete | 1.2 |
| **Phase 3** | Dynamic power model | ✅ Complete | 1.3 |
| **Phase 4** | Ground effect | ✅ Complete | 1.4 |
| **Phase 4** | Payload dynamics | ✅ Complete | 1.4 |
| **Phase 5** | Slung load dynamics | ✅ Complete | 1.5 |
| **Phase 6** | Velocity-based power | ✅ Complete | 17b463a |
| **Phase 6** | 6DOF physics mover | ✅ Complete | d57ee37 |

---

## Validation Approach

After each enhancement:
1. Run `t150_capability_demo.afsim` to verify no regressions
2. Create specific test scenario for new capability
3. Compare simulation output to analytical predictions
4. Document any parameter tuning required

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-11-29 | Initial plugin release |
| 1.1 | 2025-11-29 | Thermal fix, VRS mover integration, motor time constants |
| 1.2 | 2025-11-29 | Voltage sag model (Li-Ion OCV curve, SOC-dependent resistance) |
| 1.3 | 2025-11-30 | Dynamic power model - battery reads from propulsion |
| 1.4 | 2025-11-30 | Ground effect + Payload dynamics |
| 1.5 | 2025-11-30 | Slung load dynamics (2-DOF pendulum) |
| 2.0 | 2026-01-08 | Phase 6: Velocity-based power model, 6DOF mover, NGIC scripts |

---

## Phase 7: Dynamic Mass Properties (Planned)

### 7.1 Dynamic Inertia Based on Payload

**Problem:** Moment of inertia is static - does not change when payload mass changes.

**Current Behavior:**
- Inertia values (Ixx, Iyy, Izz) are set at scenario start and remain constant
- A 50 kg payload addition should increase rotational inertia, making the aircraft respond more sluggishly
- Currently, rotational response is identical regardless of payload

**Physical Impact:**
| Payload | Expected Ixx | Current Ixx | Error |
|---------|--------------|-------------|-------|
| 0 kg (empty) | ~2.5 kg·m² | 4.5 kg·m² | +80% |
| 68 kg (full) | ~5.0 kg·m² | 4.5 kg·m² | -10% |

**Solution - Update inertia dynamically using parallel axis theorem:**
```cpp
// In Rotorcraft6DOF_Mover - add method to update mass properties
void UpdateMassPropertiesForPayload(double payloadMass, Vector3 payloadPosition)
{
    // Base aircraft inertia (empty weight)
    const double baseIxx = 2.5;  // kg*m^2
    const double baseIyy = 2.5;
    const double baseIzz = 4.0;

    // Parallel axis theorem: I_total = I_base + m * r^2
    // For payload at position (x, y, z) from CG:
    double r_yz_sq = payloadPosition.y * payloadPosition.y +
                     payloadPosition.z * payloadPosition.z;
    double r_xz_sq = payloadPosition.x * payloadPosition.x +
                     payloadPosition.z * payloadPosition.z;
    double r_xy_sq = payloadPosition.x * payloadPosition.x +
                     payloadPosition.y * payloadPosition.y;

    mRigidBody.GetMassProperties().Ixx = baseIxx + payloadMass * r_yz_sq;
    mRigidBody.GetMassProperties().Iyy = baseIyy + payloadMass * r_xz_sq;
    mRigidBody.GetMassProperties().Izz = baseIzz + payloadMass * r_xy_sq;

    // Also update total mass
    mRigidBody.GetMassProperties().mass = mEmptyMass + payloadMass;

    // Recalculate hover throttle for new mass
    RecalculateHoverThrottle();
}
```

**Configuration Parameters to Add:**
```
# In platform definition
empty_mass 55.0              # kg - aircraft without payload
base_inertia_xx 2.5          # kg*m^2 - empty aircraft roll inertia
base_inertia_yy 2.5          # kg*m^2 - empty aircraft pitch inertia
base_inertia_zz 4.0          # kg*m^2 - empty aircraft yaw inertia
payload_position_x 0.0       # m - payload CG offset from aircraft CG
payload_position_y 0.0       # m
payload_position_z 0.3       # m - typically below aircraft CG
```

**Integration with Propulsion Processor:**
- Read `PROP_PAYLOAD_MASS` from AuxData
- Update inertia when payload changes (e.g., slung load release)

**Files to Modify:**
- `plugins/rotorcraft_6dof/source/Rotorcraft6DOF_Mover.cpp`
- `plugins/rotorcraft_6dof/source/Rotorcraft6DOF_Mover.hpp`
- `plugins/rotorcraft_6dof/source/RigidBody6DOF.hpp`
- `platforms/t150_6dof_mover.afsim`

**Validation:**
- Compare attitude step response with empty vs full payload
- Verify sluggish response with heavy payload
- Check energy consumption (more work to rotate heavier system)

---

## Future Enhancements (Post-Phase 7)

These items remain for potential future development:

### Advanced Physics
- BEMT rotor model (Blade Element Momentum Theory)
- Structural flexibility and vibration
- Advanced sensor noise injection
- Fuel consumption modeling (for hybrid/fuel-cell variants)

### Operational Features
- Multi-drone coordination
- Wind modeling integration
- Battery degradation over cycles
- Dynamic payload drop/pickup

