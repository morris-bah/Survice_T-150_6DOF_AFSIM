# T-150 UAS Physics Reference Guide

## Comprehensive Documentation for AFSIM Simulation Physics

**Document Version:** 1.1
**Last Updated:** January 2026
**Platform:** T-150 / TRV-150 Heavy-Lift UAS (Coaxial Octocopter)

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [AFSIM Motion Model Architecture](#2-afsim-motion-model-architecture)
3. [Battery Discharge Physics](#3-battery-discharge-physics)
4. [Electric Propulsion Physics](#4-electric-propulsion-physics)
5. [Vortex Ring State (VRS) Aerodynamics](#5-vortex-ring-state-vrs-aerodynamics)
6. [Slung Load Dynamics](#6-slung-load-dynamics)
7. [Performance Tables and Operational Envelope](#7-performance-tables-and-operational-envelope)
8. [Energy Consumption Analysis](#8-energy-consumption-analysis)
9. [Plugin Integration Summary](#9-plugin-integration-summary)
10. [Glossary of Terms](#10-glossary-of-terms)

---

## 1. Executive Summary

The T-150 UAS simulation uses a hybrid physics approach combining:

1. **AFSIM's Kinematic Mover** - Parametric motion model with table-driven performance
2. **Custom Physics Plugins** - Engineering-level subsystem models (battery, propulsion, VRS, slung load)

### Key Physics Questions Answered

| Question | Answer | Source |
|----------|--------|--------|
| How is battery drain calculated? | Energy (Wh) = Power (W) × Time (h); SOC depletes based on actual power draw from propulsion | Battery Processor |
| How does the drone fly through air? | Kinematic tables define acceleration, climb rate, and speed limits as functions of altitude and airspeed | AFSIM Mover |
| What affects flight efficiency? | Flight pattern, payload mass, altitude, ground effect, and thermal conditions | Propulsion + Battery Processors |
| How is thrust calculated? | Momentum theory: T = Ct × ρ × n² × D⁴ with thermal and ground effect modifiers | Propulsion Processor |

---

## 2. AFSIM Motion Model Architecture

### 2.1 Overview

AFSIM uses a **parametric kinematic approach** rather than full 6-DOF physics simulation. The platform's motion is governed by:

```
Desired State (Route Waypoints)
        ↓
Error Computation (Current vs Desired)
        ↓
Gain Application (run_time_gains)
        ↓
PID Control (feedback_control_system)
        ↓
Limit Enforcement (global_limits, performance_limits)
        ↓
Motion Update
```

### 2.2 Why Kinematic vs Physics-Based?

| Aspect | Kinematic (AFSIM) | Physics-Based (6-DOF) |
|--------|-------------------|----------------------|
| Computation | Fast, efficient | Computationally expensive |
| Validation | Easy to match real data | Requires aerodynamic coefficients |
| Scale | Handles many platforms | Limited platform count |
| Accuracy | Good for trajectory | Better for edge cases |
| Use Case | Mission-level simulation | Flight dynamics research |

### 2.3 Core Motion Equations

AFSIM's `WSF_FULL_ROTORCRAFT_MOVER` computes motion using:

**Position Update:**
```
Position(t+dt) = Position(t) + Velocity(t) × dt + 0.5 × Acceleration × dt²
```

**Velocity Update:**
```
Velocity(t+dt) = Velocity(t) + Acceleration × dt
```

**Acceleration Calculation:**
```
Acceleration = min(Commanded_Accel, Table_Lookup(altitude, airspeed))
```

### 2.4 PID Control System

The mover uses PID controllers for smooth flight dynamics:

| Controller | P Gain | I Gain | D Gain | Purpose |
|------------|--------|--------|--------|---------|
| Axial Acceleration | 1.0 | 0.05 | 0.1 | Forward/back motion |
| Lateral Acceleration | 2.0 | 0.0 | 0.5 | Side-to-side motion |
| Vertical Acceleration | 1.5 | 0.1 | 0.2 | Climb/descent |

**PID Output:**
```
Output = Kp × error + Ki × ∫error dt + Kd × (d error/dt)
```

### 2.5 Run-Time Gains

These gains convert navigation errors to rate commands:

| Parameter | Value | Effect |
|-----------|-------|--------|
| `waypoint_proportional_navigation_gain` | 50.0 | Path following aggressiveness |
| `waypoint_velocity_pursuit_gain` | 10.0 | Lead pursuit weighting |
| `vertical_offset_to_speed_gain` | 0.12 | Altitude error → climb rate |
| `deceleration_table_adjustment_gain` | 1.8 | Approach deceleration strength |
| `hover_damping_deceleration_gain` | 0.90 | Final positioning damping |

---

## 3. Battery Discharge Physics

### 3.1 Overview

The `T150_BATTERY_PROCESSOR` plugin models a 13S Li-Ion battery pack with:
- State of Charge (SOC) tracking
- Open-Circuit Voltage (OCV) modeling
- Voltage sag under load
- Internal resistance variation

### 3.2 Energy Consumption Model

**Basic Energy Equation:**
```
Energy_consumed (Wh) = Power (W) × Time (s) / 3600
```

**SOC Update:**
```
SOC = 1.0 - (Energy_consumed / Battery_capacity)
```

**T-150 Battery Parameters:**
| Parameter | Value | Notes |
|-----------|-------|-------|
| Capacity | 7000 Wh | 13S Li-Ion pack |
| Nominal Voltage | 46.8 V | 13 cells × 3.6V |
| Full Charge | 54.6 V | 13 cells × 4.2V |
| Cutoff | 39.0 V | 13 cells × 3.0V |
| Capacity (Ah) | ~150 Ah | 7000 Wh ÷ 46.8V |

### 3.3 Open-Circuit Voltage Model

Li-Ion cells follow a characteristic discharge curve. The OCV is calculated using a polynomial fit:

```
V_cell = a₀ + a₁×SOC + a₂×SOC² + a₃×SOC³
```

**Coefficients:**
- a₀ = 3.0 V (cutoff voltage per cell)
- a₁ = 1.2 (linear term)
- a₂ = -0.5 (creates flat middle region)
- a₃ = 0.5 (steeper at ends)

**Pack Voltage:**
```
V_pack = V_cell × Cell_count
       = V_cell × 13 (for 13S configuration)
```

**Visual Representation of Li-Ion Discharge Curve:**
```
V_cell (V)
  4.2 ┤████
  4.0 ┤    ████                 ← Steep initial decline
  3.8 ┤        █████
  3.6 ┤             ████████████ ← Flat plateau (20-80% SOC)
  3.4 ┤                        ██
  3.2 ┤                          ██
  3.0 ┤                            ██ ← Steep final decline
      └────────────────────────────────
      100%                         0%  SOC
```

### 3.4 Voltage Sag Model

Under load, terminal voltage drops due to internal resistance:

**Problem:**
```
V_terminal = OCV - I × R
P = V_terminal × I
```

This creates a feedback loop where low voltage causes high current, which causes more voltage drop.

**Stable Quadratic Solution:**

Combining the equations:
```
P = (OCV - I×R) × I
P = OCV×I - R×I²
R×I² - OCV×I + P = 0
```

Using the quadratic formula:
```
I = (OCV - √(OCV² - 4×R×P)) / (2×R)
```

The smaller root is the physical solution (lower current is more efficient).

**Terminal Voltage:**
```
V_terminal = OCV - I × R
V_sag = I × R
```

### 3.5 Internal Resistance Variation

Internal resistance varies with SOC following a U-shaped curve:

```
Resistance Multiplier
  3.0 ┤██
  2.5 ┤  █                     ← High R at low SOC
  2.0 ┤   █
  1.5 ┤    █
  1.0 ┤     ███████████████████ ← Minimum at ~60% SOC
  1.3 ┤                       ██ ← Slight increase at high SOC
      └────────────────────────────
      0%    20%   60%    100%  SOC
```

**Implementation:**
```cpp
if (SOC > 0.6) {
    factor = 1.0 + 0.3 × ((SOC - 0.6)² / 0.16)  // peaks ~1.3x at 100%
} else if (SOC > 0.2) {
    factor = 1.0  // flat in middle range
} else {
    factor = 1.0 + 2.0 × ((0.2 - SOC)² / 0.04)  // peaks ~3x at 0%
}
R_effective = R_base × factor
```

### 3.6 Power Source Modes

| Mode | Description | Use Case |
|------|-------------|----------|
| Fixed | Uses constant `hover_power_w` | Simple scenarios |
| Dynamic | Reads `PROP_TOTAL_POWER` from propulsion | High-fidelity scenarios |

**Dynamic Power Flow:**
```
PropulsionProcessor → PROP_TOTAL_POWER → BatteryProcessor
        ↑                                      ↓
    Motor states                          SOC update
```

---

## 4. Electric Propulsion Physics

### 4.1 Overview

The `T150_PROPULSION_PROCESSOR` models the complete electric propulsion system:
- 8 T-Motor U15II motors (coaxial octocopter configuration)
- Individual motor state tracking (RPM, temperature, efficiency)
- Thrust calculation using momentum theory
- Ground effect modeling
- Payload dynamics

### 4.2 Motor Dynamics

**RPM Calculation:**
```
RPM_no_load = KV × Voltage
RPM_target = RPM_no_load × 0.85  (accounting for load)
```

**First-Order Lag (Actuator Dynamics):**
```
α = dt / (τ + dt)
RPM_new = RPM_old + α × (RPM_target - RPM_old)
```

Where τ = 0.15 seconds (150ms time constant for large motors with 30" props)

**T-150 Motor Parameters:**
| Parameter | Value | Notes |
|-----------|-------|-------|
| Motor Count | 8 | 4 arms × 2 (coaxial) |
| KV Rating | 100 RPM/V | T-Motor U15II |
| Max Power/Motor | 3,500 W | 28 kW total |
| Propeller Diameter | 0.76 m | 30 inches |
| Peak Efficiency | 92% | At ~70% RPM |

### 4.3 Thrust Calculation (Momentum Theory)

**Thrust Equation:**
```
T = Ct × ρ × n² × D⁴
```

Where:
- T = Thrust (N)
- Ct = Thrust coefficient (0.10)
- ρ = Air density (1.225 kg/m³ at sea level)
- n = Rotational speed (rev/s = RPM/60)
- D = Propeller diameter (m)

**Torque Equation:**
```
Q = Cq × ρ × n² × D⁵
```

Where Cq = 0.012 (torque coefficient)

**Power Calculation:**
```
P_mechanical = Q × ω = Q × (RPM × 2π / 60)
P_electrical = P_mechanical / η
I_motor = P_electrical / V_nominal
```

### 4.4 Motor Efficiency Model

Efficiency varies with operating point:

**RPM Effect (Parabolic):**
```
η_rpm = 1.0 - 0.5 × (RPM_ratio - 0.7)²
```
Peak efficiency at 70% of maximum RPM.

**Torque Effect (Linear):**
```
η_torque = 1.0 - 0.2 × Torque_ratio
```
Higher torque slightly reduces efficiency.

**Combined Efficiency:**
```
η = η_peak × η_rpm × η_torque
```

### 4.5 Thermal Model

Motors accumulate heat from electrical losses:

**Heat Generation:**
```
Q_in = P_electrical × (1 - η)
```

**Heat Dissipation (Forced Convection):**
```
Q_out = k_cooling × (T_motor - T_ambient)
```

**Temperature Change:**
```
dT/dt = (Q_in - Q_out) / C_thermal
```

**T-150 Thermal Parameters:**
| Parameter | Value | Notes |
|-----------|-------|-------|
| Thermal Mass | 1500 J/K | ~1.5 kg motor × 1000 J/kg/K |
| Cooling Rate | 20 W/K | Forced convection from prop wash |
| Thermal Limit | 120°C | Motor overtemperature threshold |
| ESC Limit | 90°C | ESC overtemperature threshold |

### 4.6 Coaxial Rotor Efficiency

Coaxial configurations experience rotor interference:

```
η_system = η_motor × η_coaxial
```

Where η_coaxial = 0.85 (lower rotors receive disturbed air from upper rotors)

### 4.7 Ground Effect Model

**Cheeseman-Bennett Approximation:**
```
T_IGE / T_OGE = 1 / (1 - (R / 4h)²)
```

Where:
- T_IGE = Thrust in ground effect
- T_OGE = Thrust out of ground effect
- R = Rotor radius (0.38 m for 30" props)
- h = Rotor height above ground

**Behavior:**
- Active below 1 rotor diameter (0.76 m) AGL
- Maximum boost: 30% at ground level
- Smooth transition out of effect

```
Thrust Factor
  1.30 ┤█
  1.25 ┤ █
  1.20 ┤  ██                   ← Ground effect zone
  1.15 ┤    ███
  1.10 ┤       ████
  1.05 ┤           ████████
  1.00 ┤                   █████████ ← Out of ground effect
       └────────────────────────────
       0    0.2   0.4   0.6   0.8  1.0  Altitude/Diameter
```

### 4.8 Payload Dynamics

**Mass Properties:**
```
M_total = M_empty + M_payload
        = 68 kg + (0-55 kg)
```

**Center of Gravity Shift:**
```
CG_x = (M_payload × Offset_x) / M_total
CG_y = (M_payload × Offset_y) / M_total
CG_z = (M_payload × Offset_z) / M_total
```

**Thrust Margin:**
```
Thrust_required = M_total × g
Thrust_margin = 1 - (Thrust_required / Thrust_max)
```

### 4.9 Dynamic Power Model (Phase 6)

The propulsion processor calculates power requirements based on actual flight dynamics rather than static hover power. This enables realistic battery voltage sag response to maneuvers.

**Total Power Equation:**
```
P_total = P_induced + P_climb + P_parasite + P_accel
```

**Component Breakdown:**

| Component | Equation | Description |
|-----------|----------|-------------|
| **Induced** | `P = k × T × v_i` | Base hover power |
| **Climb** | `P = m × g × V_climb` | Rate of potential energy change |
| **Parasite** | `P = 0.5 × ρ × Cd × A_f × V³` | Drag in forward flight |
| **Accel** | `P = m × a × V` | Kinetic energy change |

**Induced Power Calculation:**
```
Disk Area: A = π × R² × N_motors
Induced Velocity: v_i = √(T / (2 × ρ × A))
P_induced = k × T × v_i
```

Where k = 1.15 (induced power factor for non-ideal rotors).

**Thrust Adjustment for Bank Angle:**

During turns, the rotor must produce additional thrust to maintain altitude:
```
T_required = m × g / cos(bank_angle)
```

| Bank Angle | Thrust Multiplier | Power Multiplier (T^1.5) |
|------------|-------------------|--------------------------|
| 0° | 1.00x | 1.00x |
| 30° | 1.15x | 1.24x |
| 45° | 1.41x | 1.68x |
| 60° | 2.00x | 2.83x |

**Bank Angle Estimation:**

Bank angle is estimated from horizontal acceleration using the coordinated turn relationship:
```
a_centripetal = g × tan(bank)
bank = atan(a_horiz / g)
```

Where horizontal acceleration is derived from velocity differentiation.

**T-150 Aerodynamic Parameters:**

| Parameter | Value | Derivation |
|-----------|-------|------------|
| Drag Coefficient (Cd) | 1.0 | Bluff body estimate |
| Frontal Area | 0.30 m² | Back-calculated from 17 kW @ 30 m/s cruise |
| Induced Power Factor | 1.15 | Non-ideal rotor losses |

**Power Variation by Flight Phase:**

| Maneuver | Power Change | Physics |
|----------|--------------|---------|
| Steady Hover | Baseline | P_induced only |
| 5 m/s Climb | +4 kW | P_climb = 123kg × 9.81 × 5 = 6kW (minus efficiency) |
| 30 m/s Cruise | -2 kW | Lower induced power, some parasite drag |
| 30° Bank Turn | +24% | Thrust increase for load factor |
| 3 m/s Descent | -6 kW | Negative P_climb contribution |
| Acceleration | Variable | P_accel = m × a × V |

---

## 5. Vortex Ring State (VRS) Aerodynamics

### 5.1 Overview

Vortex Ring State is a dangerous aerodynamic condition where a rotorcraft descends into its own downwash, causing severe loss of lift and control. The `T150_VRS_PROCESSOR` detects and models this phenomenon.

### 5.2 VRS Physics

**When VRS Occurs:**
```
VRS Risk = f(Descent_rate, Forward_speed, Induced_velocity)
```

Conditions for VRS:
1. Descent rate approaches induced velocity (≥4 m/s)
2. Low forward airspeed (<5 m/s)
3. Power applied (not autorotation)

**Induced Velocity (Hover):**
```
v_i = √(T / (2 × ρ × A))
```

Where:
- v_i = Induced velocity (~8 m/s for T-150)
- T = Thrust
- ρ = Air density
- A = Rotor disk area

### 5.3 VRS Margin Calculation

**Descent Margin:**
```
if (descent_rate ≤ 0):
    margin_descent = 1.0  (climbing - no risk)
else if (descent_rate < threshold):
    margin_descent = 1.0 - (descent_rate / threshold)
else:
    margin_descent = -(descent_rate - threshold) / threshold
```

**Speed Margin:**
```
margin_speed = min(1.0, forward_speed / speed_threshold)
```

**Combined Margin:**
```
if (forward_speed ≥ speed_threshold):
    margin = 1.0  (enough forward speed prevents VRS)
else:
    weight = 1.0 - margin_speed
    margin = margin_descent × weight + margin_speed × (1 - weight)
```

### 5.4 VRS State Machine

```
                    margin < 0.8
    ┌─────────┐ ─────────────────→ ┌───────────┐
    │  CLEAR  │                    │  CAUTION  │
    └─────────┘ ←───────────────── └───────────┘
                    margin ≥ 1.0        │
                                        │ margin < 0.5
                                        ↓
    ┌───────────┐                  ┌─────────┐
    │ RECOVERY  │ ←─────────────── │  ONSET  │
    └───────────┘   margin ≥ 0.8   └─────────┘
         │                              │
         │ margin < 0.2                 │ margin < 0.2
         │ or development               │ for 2 seconds
         │ cleared                      ↓
         └─────────────────────→ ┌───────────┐
                                 │ DEVELOPED │
           margin ≥ 0.5          └───────────┘
         └─────────────────────────────┘
```

### 5.5 Performance Degradation

**Lift Factor:**
| State | Lift Degradation | Factor |
|-------|------------------|--------|
| CLEAR | 0% | 1.0 |
| CAUTION | ~5% | 0.95 |
| ONSET | ~15% (progressive) | 0.85 |
| DEVELOPED | Up to 60% | 0.40 |
| RECOVERY | Proportional to development | 0.4-1.0 |

**Control Authority Factor:**
| State | Control Loss | Factor |
|-------|--------------|--------|
| CLEAR | 0% | 1.0 |
| CAUTION | 0% | 1.0 |
| ONSET | ~10% | 0.90 |
| DEVELOPED | Up to 50% | 0.50 |
| RECOVERY | Proportional | 0.5-1.0 |

### 5.6 VRS Thresholds

| Parameter | Value | Notes |
|-----------|-------|-------|
| Descent Threshold | 4.0 m/s | VRS risk begins |
| Speed Threshold | 5.0 m/s | Forward speed provides immunity |
| Caution Margin | 0.8 | 80% of threshold triggers warning |
| Coaxial Factor | 0.85 | Coaxial rotors have some VRS resistance |
| Development Time | 2.0 s | Time to full VRS |
| Recovery Time | 3.0 s | Time to clear VRS |

### 5.7 Recovery Actions

1. **Increase forward speed** (most effective) - Fresh air inflow
2. **Reduce collective/power** - Allow autorotation
3. **Lateral movement** - Exit the recirculating flow

---

## 6. Slung Load Dynamics

### 6.1 Overview

The `T150_SLUNG_LOAD_PROCESSOR` models cable-suspended payload dynamics using a 2-DOF spherical pendulum model.

### 6.2 Spherical Pendulum Equations

**Equations of Motion (Nonlinear):**
```
θ_x'' = -(g/L)×sin(θ_x) - (a_x/L)×cos(θ_x) - c×θ_x'
θ_y'' = -(g/L)×sin(θ_y) - (a_y/L)×cos(θ_y) - c×θ_y'
```

Where:
- θ_x, θ_y = Swing angles in X and Y planes (radians)
- g = 9.81 m/s² (gravity)
- L = Cable length (m)
- a_x, a_y = Aircraft acceleration (excitation source)
- c = Cable damping coefficient (1/s)

**Natural Frequency:**
```
f_n = (1/2π) × √(g/L)
```

For a 5m cable: f_n = 0.223 Hz (4.5 second period)

### 6.3 Integration Method

**Semi-Implicit Euler (Symplectic):**
```cpp
// Update velocity first
θ_x_dot += θ_x_dot_dot × dt
θ_y_dot += θ_y_dot_dot × dt

// Then update position
θ_x += θ_x_dot × dt
θ_y += θ_y_dot × dt
```

This method preserves energy better than explicit Euler for oscillatory systems.

### 6.4 Cable Tension Model

**Static + Centripetal:**
```
T = m × (g×cos(θ) - a_z + L×ω²)
```

Where:
- m = Load mass (kg)
- θ = Swing magnitude = √(θ_x² + θ_y²)
- a_z = Aircraft vertical acceleration (positive down)
- ω = Angular velocity = √(θ_x_dot² + θ_y_dot²)

**Constraints:**
- T ≥ 0 (cable can only pull, not push)
- T ≤ T_max (cable strength limit)

### 6.5 Force Coupling to Aircraft

**Reaction Forces (Body Frame):**
```
F_x = T × sin(θ_x)   (forward/back force)
F_y = T × sin(θ_y)   (left/right force)
F_z = T × cos(θ)     (downward force)
```

These forces affect aircraft trim and handling.

### 6.6 Load Position

**Relative to Aircraft CG:**
```
X_load = X_attach + L × sin(θ_x)
Y_load = Y_attach + L × sin(θ_y)
Z_load = Z_attach + L × cos(θ)
```

### 6.7 Resonance Detection

**Warning Conditions:**
1. Swing frequency ≈ Natural frequency (±0.1 Hz)
2. Peak swing angle > Threshold (~20°)

**Resonance can occur when:**
- Maneuvering at the natural frequency
- Turbulence excitation
- Abrupt speed changes

### 6.8 Load States

| State | Description | Trigger |
|-------|-------------|---------|
| DETACHED | No load attached | Initial or after release |
| ATTACHED | Load stable | Swing < threshold |
| SWINGING | Active oscillation | Swing > threshold |
| RESONANCE | Dangerous resonance | Frequency match + large swing |
| RELEASED | Load dropped | Manual or overload |

### 6.9 Configuration Parameters

| Parameter | Default | Range | Notes |
|-----------|---------|-------|-------|
| Load Mass | 0 kg | 0-55 kg | T-150 payload capacity |
| Cable Length | 5.0 m | 0.5-20 m | Affects natural frequency |
| Cable Damping | 0.1 1/s | 0-1 | Air damping |
| Max Tension | 5000 N | 100-10000 N | Cable breaking strength |
| Resonance Threshold | 20° | 10-45° | Warning trigger |
| Update Rate | 50 Hz | 10-100 Hz | Dynamics timestep |

---

## 7. Performance Tables and Operational Envelope

### 7.1 Maximum Acceleration Table

Varies with altitude and airspeed (m/s²):

| Altitude | 0 m/s | 15 m/s | 30 m/s |
|----------|-------|--------|--------|
| 0m | 2.5 | 2.3 | 1.8 |
| 1000m | 2.3 | 2.1 | 1.6 |
| 2000m | 2.0 | 1.8 | 1.3 |
| 3000m | 1.5 | 1.3 | 0.8 |

### 7.2 Rate of Ascent Table

Varies with altitude and airspeed (m/s):

| Altitude | 0 m/s | 10 m/s | 20 m/s | 30 m/s |
|----------|-------|--------|--------|--------|
| 0m | 5.0 | 5.0 | 4.5 | 4.0 |
| 1000m | 4.5 | 4.5 | 4.0 | 3.5 |
| 2000m | 3.5 | 3.5 | 3.0 | 2.5 |
| 3000m | 1.5 | 1.5 | 1.0 | 0.5 |

### 7.3 Rate of Descent Table

Limited by VRS avoidance (m/s):

| Altitude | Hover | 10 m/s | 20 m/s | 30 m/s |
|----------|-------|--------|--------|--------|
| All | 3.0 | 3.5 | 4.0 | 4.0 |

Higher forward speed allows faster descent (VRS margin).

### 7.4 Cruise Airspeed Table

Altitude-dependent (m/s):

| Altitude | Max Cruise Speed |
|----------|-----------------|
| 0m | 30 |
| 1000m | 29 |
| 2000m | 26 |
| 3000m | 20 |

### 7.5 Global Limits (Do Not Exceed)

| Parameter | Limit | Notes |
|-----------|-------|-------|
| Max Airspeed | 40 m/s | Aerodynamic limit |
| Max Crab Speed | 12 m/s | Sideways flight |
| Max Rate of Ascent | 8 m/s | Battery current limit |
| Max Rate of Descent | 4 m/s | VRS onset threshold |
| Max Bank Angle | 60° | Demonstrated in testing |
| Max Altitude | 3000 m | Service ceiling |
| Max Load Factor | 2.0 g | Structural limit |
| Max Turn Rate | 45°/s | Control authority |

---

## 8. Energy Consumption Analysis

### 8.1 Power by Flight Phase

| Phase | Power (kW) | Notes |
|-------|------------|-------|
| Hover | 18-25 | Load dependent |
| Climb (5 m/s) | 22 | ~4 kW above hover |
| Cruise (30 m/s) | 16-17 | Most efficient phase |
| Descent | 12 | Reduced power |
| Maneuvering | 20-25 | Pattern dependent |

### 8.2 Pattern Efficiency Comparison

| Pattern | Wh/km | Efficiency vs Straight |
|---------|-------|----------------------|
| Straight Line | 315 | Baseline |
| Sawtooth | 415 | +32% |
| Sine Wave | 516 | +64% |
| Cycloid Loops | 685 | +117% |
| Square Wave | 802 | +155% |

### 8.3 Energy Budget Calculation

**Example Mission (7-Phase Demo):**
```
Phase 1 - Takeoff/Climb:  22 kW × 100s / 3600 =  611 Wh
Phase 2 - Cruise:         17 kW × 133s / 3600 =  630 Wh
Phase 3 - Sawtooth:       20 kW × 200s / 3600 = 1111 Wh
Phase 4 - Square Wave:    22 kW × 300s / 3600 = 1833 Wh
Phase 5 - Sine Wave:      20 kW × 200s / 3600 = 1111 Wh
Phase 6 - Cycloid:        21 kW × 250s / 3600 = 1458 Wh
Phase 7 - Return/Land:    15 kW × 150s / 3600 =  625 Wh
─────────────────────────────────────────────────────
Total:                                         ~7380 Wh
Battery:                                        7000 Wh
Deficit:                                         380 Wh (5.4%)
```

### 8.4 Factors Affecting Energy Consumption

1. **Altitude** - Higher = less dense air = more power needed
2. **Payload** - Heavier = more thrust = more power
3. **Flight Pattern** - Maneuvers consume more energy
4. **Wind** - Headwind increases power requirement
5. **Temperature** - Hot = less dense air = more power
6. **Battery Age** - Degraded capacity and higher resistance

---

## 9. Plugin Integration Summary

### 9.1 Data Flow Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      AFSIM Platform                          │
│  ┌─────────────┐   ┌─────────────┐   ┌─────────────────────┐│
│  │ Propulsion  │──→│   Battery   │   │  VRS Processor      ││
│  │ Processor   │   │  Processor  │   │                     ││
│  │             │   │             │   │  Reads velocity     ││
│  │ Calculates: │   │ Calculates: │   │  from mover         ││
│  │ - Thrust    │   │ - SOC       │   │  Outputs: margin,   ││
│  │ - Power     │   │ - Voltage   │   │  state, factors     ││
│  │ - Temps     │   │ - Current   │   └─────────────────────┘│
│  └─────────────┘   └─────────────┘                          │
│        │                 │                                   │
│        └────────────┬────┘                                   │
│                     ▼                                        │
│  ┌─────────────────────────────────────────────────────────┐│
│  │                    AuxData Container                     ││
│  │  PROP_TOTAL_POWER, BATTERY_SOC, VRS_MARGIN, etc.        ││
│  └─────────────────────────────────────────────────────────┘│
│        ↑                                                     │
│  ┌─────────────────────────────────────────────────────────┐│
│  │              Slung Load Processor                        ││
│  │  Reads: velocity from mover                              ││
│  │  Outputs: tension, swing angles, forces                  ││
│  └─────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
```

### 9.2 AuxData Keys Reference

**Battery Processor Outputs:**
| Key | Type | Description |
|-----|------|-------------|
| `BATTERY_SOC` | double | State of charge (0-1) |
| `BATTERY_POWER_W` | double | Current power draw (W) |
| `BATTERY_ENERGY_WH` | double | Remaining energy (Wh) |
| `BATTERY_VOLTAGE` | double | Terminal voltage under load (V) |
| `BATTERY_OCV` | double | Open-circuit voltage (V) |
| `BATTERY_VOLTAGE_SAG` | double | Voltage drop from IR (V) |
| `BATTERY_CURRENT_A` | double | Current draw (A) |
| `BATTERY_LOW` | bool | Low battery warning flag |
| `BATTERY_VOLTAGE_LOW` | bool | Voltage sag warning flag |

**Propulsion Processor Outputs:**
| Key | Type | Description |
|-----|------|-------------|
| `PROP_TOTAL_THRUST` | double | Total thrust (N) |
| `PROP_TOTAL_POWER` | double | Total electrical power (W) |
| `PROP_SYSTEM_EFF` | double | System efficiency (0-1) |
| `PROP_MOTOR_COUNT` | int | Operational motor count |
| `PROP_MAX_TEMP` | double | Hottest motor temperature (°C) |
| `PROP_AVG_TEMP` | double | Average motor temperature (°C) |
| `PROP_GROUND_EFFECT` | double | Ground effect factor (1.0-1.3) |
| `PROP_ALTITUDE_AGL` | double | Altitude above ground (m) |
| `PROP_TOTAL_MASS` | double | Aircraft + payload mass (kg) |
| `PROP_PAYLOAD_MASS` | double | Payload mass (kg) |
| `PROP_THRUST_MARGIN` | double | Available thrust reserve (0-1) |
| `PROP_CG_OFFSET_X/Y/Z` | double | Combined CG offset (m) |
| `PROP_OVERLOADED` | bool | Payload exceeds capacity |
| `PROP_FORWARD_SPEED` | double | Horizontal speed (m/s) |
| `PROP_CLIMB_RATE` | double | Vertical speed, positive=up (m/s) |
| `PROP_BANK_ANGLE` | double | Estimated bank angle (degrees) |
| `PROP_ACCEL_MAG` | double | Horizontal acceleration magnitude (m/s²) |

**VRS Processor Outputs:**
| Key | Type | Description |
|-----|------|-------------|
| `VRS_MARGIN` | double | Safety margin (0=VRS, 1=safe) |
| `VRS_STATE` | int | Current state (0-4) |
| `VRS_LIFT_FACTOR` | double | Lift degradation factor (0.4-1.0) |
| `VRS_CONTROL_FACTOR` | double | Control authority factor (0.5-1.0) |
| `VRS_DEVELOPMENT` | double | VRS development level (0-1) |
| `VRS_DESCENT_RATE` | double | Current descent rate (m/s) |
| `VRS_FORWARD_SPEED` | double | Current forward speed (m/s) |
| `VRS_CAUTION/ONSET/CRITICAL` | bool | Warning flags |

**Slung Load Processor Outputs:**
| Key | Type | Description |
|-----|------|-------------|
| `SLUNG_LOAD_STATE` | int | Load state (0-4) |
| `SLUNG_LOAD_MASS` | double | Load mass (kg) |
| `SLUNG_CABLE_LENGTH` | double | Cable length (m) |
| `SLUNG_CABLE_TENSION` | double | Cable tension (N) |
| `SLUNG_SWING_X/Y` | double | Swing angles (degrees) |
| `SLUNG_SWING_MAG` | double | Total swing magnitude (degrees) |
| `SLUNG_FORCE_X/Y/Z` | double | Forces on aircraft (N) |
| `SLUNG_POS_X/Y/Z` | double | Load position (m) |
| `SLUNG_NATURAL_FREQ` | double | Natural frequency (Hz) |
| `SLUNG_RESONANCE` | bool | Resonance warning flag |

### 9.3 Scenario Configuration Example

```afsim
platform_type T150_FULL_PHYSICS T150_UAS

   # Battery with voltage sag model
   processor BATTERY_MONITOR T150_BATTERY_PROCESSOR
      battery_capacity_wh    7000.0
      initial_soc            1.0
      nominal_voltage        46.8
      internal_resistance    0.025
      use_dynamic_power      true
   end_processor

   # Propulsion with thermal and ground effect
   processor PROPULSION_MONITOR T150_PROPULSION_PROCESSOR
      motor_count            8
      kv_rating              100
      max_power_per_motor    3500
      propeller_diameter     0.76
      coaxial_efficiency     0.85
      motor_thermal_limit    120
      ground_effect_enabled  true
      empty_mass             68.0
      payload_mass           40.0
   end_processor

   # VRS detection
   processor VRS_MONITOR T150_VRS_PROCESSOR
      vrs_descent_threshold  4.0
      vrs_speed_threshold    5.0
      coaxial_factor         0.85
   end_processor

   # Slung load dynamics
   processor SLUNG_LOAD T150_SLUNG_LOAD_PROCESSOR
      load_mass              25.0
      cable_length           6.0
      cable_damping          0.1
   end_processor

end_platform_type
```

---

## 10. Glossary of Terms

| Term | Definition |
|------|------------|
| **AGL** | Above Ground Level - altitude measured from terrain |
| **AuxData** | AFSIM auxiliary data container for inter-processor communication |
| **Coaxial** | Rotor configuration with two rotors on the same axis |
| **IGE** | In Ground Effect - near-ground flight with thrust augmentation |
| **Induced Velocity** | Downward airflow velocity through rotor disk |
| **KV** | Motor constant: RPM per volt under no load |
| **MSL** | Mean Sea Level - altitude measured from sea level |
| **MTOW** | Maximum Takeoff Weight |
| **NED** | North-East-Down coordinate frame |
| **OCV** | Open-Circuit Voltage - battery voltage with no load |
| **OGE** | Out of Ground Effect - flight above ground effect zone |
| **PID** | Proportional-Integral-Derivative controller |
| **SOC** | State of Charge - battery charge level (0-100%) |
| **VRS** | Vortex Ring State - dangerous descent into own downwash |

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01 | Claude | Initial comprehensive physics documentation |
| 1.1 | 2026-01 | Claude | Added Section 4.9 (Dynamic Power Model), new AuxData keys |

---

## References

1. AFSIM Documentation, `/opt/afsim2.2505/documentation/`
2. AAIB Accident Investigation Report (South Scarle, 2023)
3. T-Motor U15II Motor Specifications
4. US Patent US20240262499A1 (Battery topology)
5. Cheeseman, I.C. and Bennett, W.E., "The Effect of Ground on a Helicopter Rotor in Forward Flight", ARC R&M 3021, 1955
6. Johnson, W., "Helicopter Theory", Princeton University Press, 1980
