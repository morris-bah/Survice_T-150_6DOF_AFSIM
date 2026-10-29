# T-150 UAS Plugin Architecture

## Overview

The T-150 UAS simulation extends AFSIM with custom processor plugins that model the engineering subsystems of a heavy-lift coaxial octocopter. This document describes the architecture, design decisions, and inter-processor communication patterns.

---

## Plugin Summary

| Plugin | Class | Purpose |
|--------|-------|---------|
| `t150_battery` | `BatteryProcessor` | Battery SOC, voltage sag, energy tracking |
| `t150_propulsion` | `PropulsionProcessor` | Motor power, thermal, RPM, thrust modeling |
| `t150_vrs` | `VRSProcessor` | Vortex Ring State detection and warning |

---

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        AFSIM Platform                            │
│                                                                  │
│  ┌──────────────────┐    ┌──────────────────┐    ┌────────────┐ │
│  │  PropulsionProc  │    │   BatteryProc    │    │  VRSProc   │ │
│  │                  │    │                  │    │            │ │
│  │  - Motor states  │    │  - SOC tracking  │    │  - Descent │ │
│  │  - Thermal model │───▶│  - Voltage sag   │    │    rate    │ │
│  │  - Power calc    │    │  - Energy cons.  │    │  - VRS     │ │
│  │                  │    │                  │    │    margin  │ │
│  └────────┬─────────┘    └──────────────────┘    └─────┬──────┘ │
│           │                                            │        │
│           │          AuxData (shared state)            │        │
│           ▼                                            ▼        │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                    Platform AuxData                          ││
│  │  PROP_TOTAL_POWER, PROP_MAX_TEMP, BATTERY_SOC, VRS_MARGIN   ││
│  └─────────────────────────────────────────────────────────────┘│
│           │                                            │        │
│           ▼                                            ▼        │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                    Platform Mover                            ││
│  │              GetVelocityNED(), GetPosition()                 ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
```

---

## Design Principles

### 1. Processor Independence

Each processor is designed to function independently. If one processor is not configured, the others continue to operate with fallback values.

**Example:** `BatteryProcessor` can use fixed `hover_power_w` if `PropulsionProcessor` is not present, or read dynamic power via `PROP_TOTAL_POWER` when available.

```cpp
// BatteryProcessor::Update()
if (mUseDynamicPower) {
    double propPower = 0.0;
    if (GetPlatform()->GetAuxData().TryGet("PROP_TOTAL_POWER", propPower) && propPower > 0.0) {
        mCurrentPower = propPower;  // Dynamic from propulsion
    } else {
        mCurrentPower = mHoverPowerW;  // Fallback
    }
}
```

### 2. AuxData for Inter-Processor Communication

Processors communicate through the platform's `AuxData` container. This provides:
- **Loose coupling**: Processors don't need direct references to each other
- **Script access**: Scenario scripts can read/write AuxData values
- **Debugging**: All values are visible in AFSIM's data inspection tools

**Published AuxData Keys:**

| Key | Type | Source | Description |
|-----|------|--------|-------------|
| `PROP_TOTAL_POWER` | double | Propulsion | Total electrical power draw (W) |
| `PROP_TOTAL_THRUST` | double | Propulsion | Combined motor thrust (N) |
| `PROP_MAX_TEMP` | double | Propulsion | Hottest motor temperature (C) |
| `PROP_AVG_TEMP` | double | Propulsion | Average motor temperature (C) |
| `PROP_MOTOR_COUNT` | int | Propulsion | Operational motor count |
| `PROP_SYSTEM_EFF` | double | Propulsion | Overall system efficiency |
| `PROP_GROUND_EFFECT` | double | Propulsion | Ground effect factor (1.0 = no effect) |
| `PROP_ALTITUDE_AGL` | double | Propulsion | Current altitude above ground (m) |
| `PROP_TOTAL_MASS` | double | Propulsion | Total aircraft mass: empty + payload (kg) |
| `PROP_PAYLOAD_MASS` | double | Propulsion | Current payload mass (kg) |
| `PROP_THRUST_MARGIN` | double | Propulsion | Available thrust margin (0-1, 0=at limit) |
| `PROP_CG_OFFSET_X` | double | Propulsion | Combined CG X offset (m) |
| `PROP_CG_OFFSET_Y` | double | Propulsion | Combined CG Y offset (m) |
| `PROP_CG_OFFSET_Z` | double | Propulsion | Combined CG Z offset (m) |
| `PROP_OVERLOADED` | bool | Propulsion | True if payload exceeds max capacity |
| `BATTERY_SOC` | double | Battery | State of charge (0-1) |
| `BATTERY_POWER_W` | double | Battery | Current power consumption (W) |
| `BATTERY_ENERGY_WH` | double | Battery | Remaining energy (Wh) |
| `BATTERY_VOLTAGE` | double | Battery | Terminal voltage under load (V) |
| `BATTERY_OCV` | double | Battery | Open-circuit voltage (V) |
| `BATTERY_VOLTAGE_SAG` | double | Battery | Voltage drop from IR (V) |
| `BATTERY_CURRENT_A` | double | Battery | Current draw (A) |
| `BATTERY_LOW` | bool | Battery | Low battery warning flag |
| `BATTERY_VOLTAGE_LOW` | bool | Battery | Voltage sag warning flag |
| `VRS_STATE` | int | VRS | Current VRS state enum |
| `VRS_MARGIN` | double | VRS | Safety margin (0=VRS, 1=safe) |
| `VRS_DESCENT_RATE` | double | VRS | Current descent rate (m/s) |

### 3. Reading Platform State from Mover

The VRS processor demonstrates reading actual platform kinematics from AFSIM's mover system:

```cpp
void VRSProcessor::Update(double aSimTime) {
    // Get velocity from platform mover (NED frame)
    WsfTrackUtility::VelocityNED vel;
    if (GetPlatform()->GetVelocityNED(vel)) {
        double descentRate = std::max(0.0, vel.Down());  // Positive = descending
        double forwardSpeed = std::sqrt(vel.North()*vel.North() + vel.East()*vel.East());
        mVRSMargin = ComputeVRSMargin(descentRate, forwardSpeed);
    }
}
```

This ensures VRS detection uses actual simulated velocities, not placeholder values.

---

## Battery Processor Deep Dive

### Voltage Sag Model (Phase 2)

The battery models realistic Li-Ion voltage behavior under load using:

1. **Open-Circuit Voltage (OCV)** - varies with SOC following Li-Ion discharge curve
2. **Internal Resistance (IR)** - increases at low SOC (up to 3x at 0%)
3. **Voltage Sag** - V_drop = I × R

**Quadratic Current Solution (Stable)**

For a constant power load P with internal resistance R:
```
P = V_terminal × I
V_terminal = OCV - I×R
```

Substituting: `P = (OCV - I×R) × I`

Solving quadratic: `I = (OCV - sqrt(OCV² - 4×R×P)) / (2×R)`

This avoids the instability of iterative `I = P/V` calculations where low voltage causes high current, causing more voltage drop, etc.

```cpp
double discriminant = mOpenCircuitVoltage * mOpenCircuitVoltage - 4.0 * resistance * mCurrentPower;
if (discriminant >= 0.0 && resistance > 0.0) {
    mCurrentAmps = (mOpenCircuitVoltage - std::sqrt(discriminant)) / (2.0 * resistance);
    mVoltageSag = mCurrentAmps * resistance;
    mTerminalVoltage = mOpenCircuitVoltage - mVoltageSag;
}
```

### Dynamic Power Mode (Phase 3)

When `use_dynamic_power true` is set, battery reads actual power from propulsion:

```
BatteryProcessor                     PropulsionProcessor
      │                                      │
      │   GetAuxData().TryGet(               │
      │     "PROP_TOTAL_POWER", power)       │
      │◄─────────────────────────────────────│
      │                                      │
      │   Uses power for voltage/SOC calc    │
      ▼                                      ▼
```

This creates realistic correlation between flight phase and power consumption.

---

## Propulsion Processor Deep Dive

### Motor State Machine

Each motor has an independent state:
- `OFF` - Motor not running
- `RUNNING` - Normal operation
- `FAILED` - Thermal or electrical failure

### Thermal Model

Motors accumulate heat from electrical losses and dissipate via forced convection:

```cpp
// Heat input from electrical losses
double heatInput = motorPower * (1.0 - efficiency);

// Heat output from cooling (prop wash convection)
double heatOutput = mMotorCoolingRate * (temperature - ambientTemp);

// Temperature change
double dT = (heatInput - heatOutput) / mMotorThermalMass * deltaTime;
```

Parameters were tuned for thermal equilibrium during sustained hover:
- Thermal mass: 1500 J/K (realistic for U15II motor)
- Cooling rate: 20 W/K (forced convection from prop wash)

### Motor Time Constants

First-order lag models actuator dynamics:

```cpp
double alpha = deltaTime / (mMotorTimeConstant + deltaTime);
motor.rpm = motor.rpm + alpha * (targetRPM - motor.rpm);
```

With 150ms time constant for large motors with 30" props.

---

## VRS Processor Deep Dive

### VRS Detection Algorithm

Vortex Ring State occurs when:
1. High descent rate (approaching induced velocity)
2. Low forward speed (no fresh air inflow)

```cpp
double VRSProcessor::ComputeVRSMargin(double descentRate, double forwardSpeed) {
    // Normalize descent rate by induced velocity
    double descentRatio = descentRate / mInducedVelocity;

    // Forward speed provides margin (fresh air inflow)
    double speedMargin = forwardSpeed / mVRSSpeedThreshold;

    // Coaxial rotors are more resistant to VRS
    double adjustedThreshold = mVRSDescentThreshold * mCoaxialFactor;

    // Compute margin (0 = VRS onset, 1 = fully safe)
    if (descentRate < adjustedThreshold) {
        return 1.0;  // Safe
    }
    return std::max(0.0, 1.0 - descentRatio + speedMargin);
}
```

---

## Build System

### CMake Structure

```
plugins/
├── CMakeLists.txt           # Top-level, finds AFSIM
├── t150_battery/
│   ├── CMakeLists.txt       # Battery plugin target
│   └── source/
│       ├── BatteryProcessor.cpp
│       └── BatteryProcessor.hpp
├── t150_propulsion/
│   ├── CMakeLists.txt
│   └── source/...
└── t150_vrs/
    ├── CMakeLists.txt
    └── source/...
```

### Build and Install Workflow

```bash
cd /home/shepard/afsim_projects/project005_t150_uas

# 1. Build plugins (auto-detects AFSIM)
./ngic_afsim_build.sh

# 2. Install to AFSIM (requires sudo)
./ngic_afsim_install.sh

# 3. Verify installation
./ngic_afsim_install.sh --verify
```

The NGIC build script:
1. Auto-detects AFSIM installation location
2. Runs CMake configuration against AFSIM headers
3. Compiles shared libraries to `lib/`

The NGIC install script:
1. Installs to both `wsf_plugins/` and `mission_plugins/` directories
2. Verifies checksums match between source and installed

---

## Scenario Configuration

### Platform Type Definition

```afsim
platform_type T150_ENDURANCE_TEST T150_UAS

   processor BATTERY_MONITOR T150_BATTERY_PROCESSOR
      battery_capacity_wh    7000.0
      initial_soc            1.0
      hover_power_w          18000.0
      low_battery_warning    0.20
      nominal_voltage        46.8
      full_charge_voltage    54.6
      cutoff_voltage         39.0
      internal_resistance    0.025
      voltage_sag_warning    42.0
      use_dynamic_power      true
   end_processor

   processor PROPULSION_MONITOR T150_PROPULSION_PROCESSOR
      motor_count            8
      kv_rating              100
      max_power_per_motor    3500
      propeller_diameter     0.76
      coaxial_efficiency     0.85
      motor_thermal_limit    150
      nominal_voltage        46.8
   end_processor

   processor VRS_MONITOR T150_VRS_PROCESSOR
      vrs_descent_threshold  4.0
      vrs_speed_threshold    5.0
      induced_velocity       8.0
      coaxial_factor         0.85
   end_processor

end_platform_type
```

---

## Version History

| Version | Phase | Changes |
|---------|-------|---------|
| 1.0 | Initial | Basic SOC, thermal, VRS detection |
| 1.1 | Phase 1 | Thermal tuning, VRS mover integration, motor time constants |
| 1.2 | Phase 2 | Voltage sag model (OCV curve, SOC-dependent IR) |
| 1.3 | Phase 3 | Dynamic power model, quadratic voltage solution |
| 1.4 | Phase 4 | Ground effect model (Cheeseman-Bennett approximation) |
| 1.5 | Phase 4 | Payload dynamics model (mass, CG, thrust margin) |

---

## Ground Effect Model (Phase 4)

The propulsion processor includes a ground effect model that increases thrust when operating near the ground.

### Physics Model

Uses the Cheeseman-Bennett ground effect approximation:

```
T_ige/T_oge = 1 / (1 - (R/4h)^2)
```

Where:
- `T_ige` = Thrust in ground effect
- `T_oge` = Thrust out of ground effect
- `R` = Rotor radius (0.38m for T-150's 30" props)
- `h` = Rotor height above ground

### Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `ground_effect_enabled` | true | Enable/disable ground effect |
| `ground_effect_max_boost` | 1.3 | Maximum thrust multiplier (30% boost) |

### AuxData Output

| Key | Type | Description |
|-----|------|-------------|
| `PROP_GROUND_EFFECT` | double | Current ground effect factor (1.0 = no effect) |
| `PROP_ALTITUDE_AGL` | double | Current altitude above ground (m) |

### Behavior

- **Active region**: Below 1 rotor diameter (0.76m) AGL
- **Maximum effect**: At ground level, up to 30% thrust boost
- **Transition**: Smooth decrease as altitude increases
- **Out of ground effect**: Factor = 1.0 above rotor diameter altitude

---

## Payload Dynamics Model (Phase 4)

The propulsion processor includes a payload dynamics model that tracks aircraft mass, calculates combined center of gravity shifts, and computes thrust margin.

### Physics Model

The combined CG is calculated as a weighted average:

```
CG_combined = (m_payload * CG_payload) / (m_empty + m_payload)
```

Thrust margin indicates available reserve:

```
margin = 1 - (required_hover_thrust / max_thrust)
```

Where `required_hover_thrust = total_mass * g`.

### Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `empty_mass` | 68.0 | Empty aircraft mass (kg) |
| `payload_mass` | 0.0 | Current payload mass (kg) |
| `max_payload_mass` | 55.0 | Maximum payload capacity (kg) |
| `payload_cg_offset_x` | 0.0 | Payload CG forward from aircraft CG (m) |
| `payload_cg_offset_y` | 0.0 | Payload CG right from aircraft CG (m) |
| `payload_cg_offset_z` | 0.2 | Payload CG below aircraft CG (m) |

### AuxData Output

| Key | Type | Description |
|-----|------|-------------|
| `PROP_TOTAL_MASS` | double | Total aircraft mass: empty + payload (kg) |
| `PROP_PAYLOAD_MASS` | double | Current payload mass (kg) |
| `PROP_THRUST_MARGIN` | double | Available thrust margin (0-1, 0=at limit) |
| `PROP_CG_OFFSET_X` | double | Combined CG X offset (m) |
| `PROP_CG_OFFSET_Y` | double | Combined CG Y offset (m) |
| `PROP_CG_OFFSET_Z` | double | Combined CG Z offset (m) |
| `PROP_OVERLOADED` | bool | True if payload exceeds max capacity |

### Behavior

- **Mass tracking**: Total mass = empty mass + payload mass
- **CG shift**: Combined CG offset proportional to payload mass ratio
- **Thrust margin**: Dynamically computed based on total mass
- **Overload warning**: Flag set when payload exceeds max_payload_mass
- **Thrust calculation**: Uses total mass for required hover thrust

### Example Configuration

```afsim
processor PROPULSION_MONITOR T150_PROPULSION_PROCESSOR
   motor_count            8
   max_power_per_motor    3500
   # ... other parameters ...

   # Payload configuration
   empty_mass             68.0        # T-150 empty weight (kg)
   payload_mass           40.0        # 40 kg payload
   max_payload_mass       55.0        # Max 55 kg capacity
   payload_cg_offset_x    0.1         # 10cm forward
   payload_cg_offset_y    0.0         # Centered
   payload_cg_offset_z    0.3         # 30cm below
end_processor
```

---

## Future Enhancements

### Phase 5+ (Roadmap)
- Slung load dynamics
- BEMT rotor model
- Sensor noise injection
