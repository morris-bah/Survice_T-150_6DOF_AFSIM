# T-150 Simulation Enhancement Ideas: Custom Plugin Concepts

> **NOTE**: Many plugins described in this document have been implemented as of v2.0 (January 2026):
> - ✅ Battery Physics Plugin → `T150_BATTERY_PROCESSOR`
> - ✅ Electric Propulsion Plugin → `T150_PROPULSION_PROCESSOR`
> - ✅ VRS Detection Plugin → `T150_VRS_PROCESSOR`
> - ✅ Payload/Slung Load Plugin → `T150_SLUNG_LOAD_PROCESSOR`
>
> See `ENHANCEMENT_ROADMAP.md` for completed phases and `T150_PHYSICS_REFERENCE.md` for physics documentation.

## Overview

While the current simulation achieves high fidelity using AFSIM's built-in `WSF_FULL_ROTORCRAFT_MOVER` and script-based processors, certain advanced features would benefit from custom C++ plugin development. This document outlines potential plugin enhancements for future development.

---

## 1. Advanced Battery Physics Plugin

### Concept: `WsfBatteryModel`

A dedicated battery modeling plugin that implements detailed Li-Ion battery physics.

### Features Not Achievable with Scripts
- **Cell-Level Modeling**: Model individual cell voltages and balance states
- **Peukert Effect**: Non-linear capacity reduction at high discharge rates
- **Thermal Coupling**: Temperature effects on capacity and internal resistance
- **Regenerative Charging**: Energy recovery during descent
- **State Estimation**: Kalman filter-based SOC/SOH estimation

### Technical Justification
The current script-based energy model uses linear approximations. Real Li-Ion batteries exhibit:
- Non-linear voltage curves (especially near full/empty)
- Temperature-dependent capacity (−20% at 0°C, −5% at 40°C)
- Aging effects reducing capacity over cycles
- Current-dependent efficiency (I²R losses)

### Proposed Interface
```cpp
class WsfBatteryModel : public WsfProcessor
{
public:
    // Configuration
    void SetCellConfiguration(int series, int parallel);
    void SetCellChemistry(CellType type); // LiPo, Li-Ion, LiFePO4
    void SetCapacity(double ah, double wh);

    // State
    double GetVoltage() const;
    double GetSOC() const;           // 0-100%
    double GetSOH() const;           // State of Health
    double GetTemperature() const;
    double GetRemainingEnergy() const;

    // Thermal
    void SetAmbientTemperature(double celsius);
    double GetInternalTemperature() const;
};
```

### Priority: **HIGH**
This would significantly improve simulation fidelity for mission planning and range estimation.

---

## 2. Electric Propulsion System Plugin

### Concept: `WsfElectricPropulsion`

Detailed modeling of the electric motor/ESC/propeller system.

### Features
- **Motor Efficiency Maps**: Efficiency varies with RPM and torque
- **ESC Modeling**: Switching losses, thermal limits
- **Propeller Aerodynamics**: Thrust coefficient tables (CT, CP)
- **Coaxial Interference**: Upper/lower rotor interaction effects
- **Motor Failure Modes**: Single motor loss, ESC failure, propeller damage

### Technical Justification
The T-150 uses 8 T-Motor U15II motors in a coaxial configuration. Real-world effects include:
- ~15-20% efficiency loss from coaxial interference
- Motor efficiency varying from 70-92% depending on operating point
- Propeller efficiency degradation at high altitude (thin air)
- ESC thermal derating at sustained high current

### Proposed Output Variables
```cpp
struct PropulsionState {
    double motor_rpm[8];
    double motor_current[8];
    double motor_temperature[8];
    double propeller_thrust[8];
    double total_thrust;
    double power_consumed;
    double system_efficiency;
};
```

### Priority: **MEDIUM**
Would enable more accurate power consumption modeling and failure mode analysis.

---

## 3. Vortex Ring State (VRS) Detection Plugin

### Concept: `WsfVRSModel`

Implements detection and modeling of Vortex Ring State aerodynamics.

### Background
VRS occurs when a rotorcraft descends into its own downwash, causing:
- Loss of effective lift
- Uncontrollable descent ("settling with power")
- Potential for crash if not recovered

### Features
- **VRS Onset Detection**: Based on descent rate, forward speed, disk loading
- **Severity Modeling**: Partial to full VRS with graduated effects
- **Recovery Modeling**: Required forward speed and altitude loss to exit
- **Warning System**: Alerts before VRS onset

### T-150 Specific Parameters
From the specification documents:
- VRS onset at ~4-5 m/s descent rate with zero forward speed
- Coaxial configuration provides some VRS resistance
- Recovery requires forward acceleration or collective dump

### Proposed Implementation
```cpp
class WsfVRSModel : public WsfProcessor
{
    // VRS State Machine
    enum VRSState { CLEAR, CAUTION, ONSET, DEVELOPED, RECOVERY };

    // Detection thresholds
    double GetVRSMargin() const;  // 0=in VRS, 1=completely clear
    VRSState GetVRSState() const;

    // If in VRS, affects mover performance
    void ModifyLiftCapability(double factor);
    void ModifyControlAuthority(double factor);
};
```

### Priority: **MEDIUM-HIGH**
Critical for realistic descent modeling and safety margin analysis.

---

## 4. Advanced Flight Path Pattern Generator Plugin

### Concept: `WsfPatternGenerator`

Programmatic generation of complex flight patterns.

### Current Limitation
The scenario currently uses manually-calculated waypoints for patterns. A plugin could generate:
- Mathematically precise patterns
- Dynamically adjustable parameters
- Real-time pattern modification

### Pattern Types
- **Sawtooth/Zigzag**: Triangular waveform path
- **Square Wave**: Right-angle path segments
- **Sinusoidal**: Smooth sine wave
- **Prolate Cycloid**: Looping pattern (like a Spirograph)
- **Epitrochoid**: Multi-lobed patterns
- **Lissajous**: Complex 2D harmonic patterns
- **Raster Scan**: Grid coverage patterns
- **Spiral**: Expanding/contracting spirals

### Proposed Interface
```cpp
class WsfPatternGenerator : public WsfProcessor
{
    // Pattern definition
    void SetPattern(PatternType type);
    void SetAmplitude(double meters);
    void SetWavelength(double meters);  // or frequency
    void SetPhase(double degrees);

    // Path endpoints
    void SetStartPoint(WsfGeoPoint start);
    void SetEndPoint(WsfGeoPoint end);

    // Generate waypoints
    WsfRoute GenerateRoute(int numPoints);

    // Real-time adjustment
    void ModifyAmplitude(double newAmplitude);
};
```

### Priority: **LOW**
Current script-based approach is adequate; plugin would be a convenience enhancement.

---

## 5. Payload Drop/Delivery Plugin

### Concept: `WsfPayloadSystem`

Models payload carrying, release, and delivery operations.

### T-150 Mission Context
The T-150 is designed for tactical resupply missions carrying up to 68 kg of cargo. Modeling payload operations would enable:
- Dynamic mass changes during mission
- Drop accuracy analysis
- Multi-stop delivery routes
- Pickup operations

### Features
- **Dynamic Mass**: Platform mass changes with payload state
- **Drop Mechanics**: Ballistic trajectory of released payload
- **Winch Operations**: Hover delivery with cable
- **Impact Zone**: Landing accuracy modeling

### Proposed Interface
```cpp
class WsfPayloadSystem : public WsfProcessor
{
    // Payload configuration
    void SetPayloadMass(double kg);
    void SetPayloadVolume(double m3);
    void SetPayloadType(PayloadType type);  // CARGO, MEDICAL, AMMO

    // Operations
    void Release();           // Ballistic drop
    void LowerOnCable(double meters);
    void RetractCable();
    void DetachCable();

    // State
    double GetCurrentMass() const;
    bool IsPayloadAttached() const;
    WsfGeoPoint GetPayloadLocation() const;
};
```

### Priority: **MEDIUM**
Would enable more realistic logistics mission simulations.

---

## 6. Environmental Effects Plugin

### Concept: `WsfEnvironmentalEffects`

Models environmental factors affecting flight performance.

### Effects to Model
- **Wind**: Steady wind, gusts, wind shear
- **Air Density**: Temperature and pressure effects on thrust
- **Precipitation**: Rain reducing lift, icing conditions
- **Turbulence**: Random perturbations to flight path

### T-150 Considerations
- Electric motors are weather-sealed (IP55)
- Large rotors susceptible to gusts
- Battery performance degrades in extreme temperatures
- Documented Arctic and desert operations

### Priority: **LOW-MEDIUM**
Standard AFSIM atmosphere modeling may be sufficient; advanced effects could enhance realism.

---

## 7. Collision Avoidance / Obstacle Detection Plugin

### Concept: `WsfObstacleAvoidance`

Models the "Firefly" autonomy package mentioned in T-150 documentation.

### Features
- **Terrain Following**: Maintain clearance over terrain
- **Obstacle Detection**: Identify and avoid obstacles
- **Path Replanning**: Dynamically adjust route
- **Emergency Stop**: Hover when obstacle detected

### Integration with Existing AFSIM
- Could work with `WSF_TERRAIN_FOLLOWING_PROCESSOR`
- Add sensor-based obstacle detection
- Decision logic for avoidance maneuvers

### Priority: **LOW**
Requires sensor modeling integration; primarily relevant for autonomous operations.

---

## Implementation Recommendations

### Phase 1 (Immediate Value)
1. **Battery Physics Plugin** - Highest impact on simulation fidelity

### Phase 2 (Enhanced Realism)
2. **VRS Detection Plugin** - Critical for safety analysis
3. **Electric Propulsion Plugin** - Better power modeling

### Phase 3 (Mission Capability)
4. **Payload System Plugin** - Enable logistics missions
5. **Pattern Generator Plugin** - Convenience for testing

### Phase 4 (Advanced Features)
6. **Environmental Effects** - Weather modeling
7. **Obstacle Avoidance** - Autonomous operations

---

## Development Notes

### AFSIM Plugin Development Resources
- Headers: `/opt/afsim2.2505/include/`
- Documentation: `/opt/afsim2.2505/documentation/`
- Learn tutorials: `/opt/afsim2.2505/learn/`
- Example plugins: Existing Project001 drone_swarm_plugin

### Estimated Development Effort
| Plugin | Complexity | Est. Development Time |
|--------|------------|----------------------|
| Battery Physics | High | 2-3 weeks |
| Electric Propulsion | High | 2-3 weeks |
| VRS Detection | Medium | 1 week |
| Pattern Generator | Low | 3-5 days |
| Payload System | Medium | 1-2 weeks |
| Environmental Effects | Medium | 1-2 weeks |
| Obstacle Avoidance | High | 3-4 weeks |

---

## Conclusion

The current script-based implementation provides good baseline fidelity for the T-150 simulation. The plugins described here would primarily benefit:
- **Research applications** requiring precise physics
- **Training simulations** needing realistic failure modes
- **Mission planning tools** requiring accurate range/endurance estimates
- **Multi-drone scenarios** where computational efficiency matters

For most demonstration and testing purposes, the current implementation is sufficient. Plugin development should be prioritized based on specific analytical requirements.
