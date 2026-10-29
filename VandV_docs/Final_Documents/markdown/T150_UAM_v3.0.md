[CLASSIFICATION: UNCLASSIFIED]

# User and Analyst Manual (UAM)

## T-150/TRV-150 Heavy-Lift UAS Simulation Model

### AFSIM Project 005

| Field | Value |
|-------|-------|
| **Document Version** | 3.0 |
| **Date** | March 5, 2026 |
| **Classification** | UNCLASSIFIED |
| **Sponsoring Organization** | [PLACEHOLDER - Organization Name] |
| **Program Manager** | [PLACEHOLDER - Name/Title] |
| **Document Preparer** | [PLACEHOLDER - Organization/Contract] |
| **Distribution Statement** | [PLACEHOLDER - Distribution Statement] |
| **M&S Name and Version** | T-150 UAS AFSIM Plugin Suite v2.0 |

---

## Record of Changes

| Version | Date | Changes |
|---------|------|---------|
| 3.0 | 2026-03-05 | Complete rewrite per NGIC 4.0 template. Updated for all 6 development phases. Added MIL-STD-3022/DoDI 5000.61 citations. |
| 2.0 | 2026-01-15 | Added 6DOF mover, Phase 6 velocity-based power model, Phase 1 validation results. |
| 1.0 | 2025-12-19 | Initial release covering Phases 1-5 (battery, propulsion, VRS, slung load). |
| 0.9 | 2025-12-01 | Draft for review. |

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Threat System Description](#2-threat-system-description)
3. [Model Description](#3-model-description)
4. [Model Components](#4-model-components)
5. [Model Setup and Configuration](#5-model-setup-and-configuration)
6. [Analytic Outputs](#6-analytic-outputs)
7. [Sources, Assumptions, Limitations, and Gaps](#7-sources-assumptions-limitations-and-gaps)
8. [References](#8-references)
9. [Acronyms](#9-acronyms)

---

## 1. Introduction

### 1.1 Purpose

This User and Analyst Manual (UAM) provides comprehensive guidance for analysts, operators, and intelligence professionals using the T-150/TRV-150 Heavy-Lift Unmanned Aerial System (UAS) simulation model within the Advanced Framework for Simulation, Integration, and Modeling (AFSIM). This document describes the model's intended use, functional behavior, key performance indicators, configurable parameters, physics models, and assumptions and limitations that affect analytical conclusions.

This document is prepared in accordance with:

- **DoDI 5000.61**, "DoD Modeling and Simulation Verification, Validation, and Accreditation," September 17, 2024 [1]
- **MIL-STD-3022**, "Documentation of Verification, Validation, and Accreditation (VV&A) for Models and Simulations," January 28, 2008 [2]
- **NGIC 4.0 Documentation Templates**, January 30, 2026

### 1.2 Scope

This manual covers the T-150 UAS AFSIM Plugin Suite v2.0, which consists of five custom C++ plugins:

| Plugin | AFSIM Type Name | Purpose |
|--------|-----------------|---------|
| `rotorcraft_6dof` (.dll/.so) | `Rotorcraft6DOF_Mover` | Physics-based 6DOF flight dynamics |
| `t150_battery` (.dll/.so) | `T150_BATTERY_PROCESSOR` | Li-Ion battery with voltage sag modeling |
| `t150_propulsion` (.dll/.so) | `T150_PROPULSION_PROCESSOR` | Motor power, thermal, RPM, ground effect |
| `t150_vrs` (.dll/.so) | `T150_VRS_PROCESSOR` | Vortex Ring State detection and modeling |
| `t150_slung_load` (.dll/.so) | `T150_SLUNG_LOAD_PROCESSOR` | Cable-suspended payload dynamics |

**Total Code**: 7,736 lines of C++ across 26 source files.

### 1.3 Model Intended Use

The T-150 UAS simulation model is intended for use in the following analytical contexts:

1. **Intelligence Analysis**: Assessing the operational capabilities, performance envelope, and limitations of the T-150/TRV-150 heavy-lift UAS system to support intelligence community questions regarding UAS threats and capabilities.

2. **Mission Planning Analysis**: Evaluating flight endurance, range, energy consumption, and payload capacity under various mission profiles to understand operational employment concepts.

3. **Counter-UAS (C-UAS) Assessment**: Providing high-fidelity UAS flight dynamics for evaluating detection, tracking, and engagement scenarios against heavy-lift UAS threats.

4. **Vulnerability Analysis**: Analyzing system degradation behavior including Vortex Ring State, battery depletion, motor thermal limits, and slung load instability.

5. **Concept of Operations (CONOPS) Development**: Supporting development of tactical employment concepts for heavy-lift UAS logistics and resupply operations.

> **Note**: This model represents a specific UAS platform (T-150/TRV-150) based on open-source research. Parameters may be adapted for other coaxial multirotor platforms per the adaptation guidance in Section 5.

### 1.4 Confidence Summary

The following table summarizes the confidence level for each modeled capability based on the quality of source data and validation testing performed.

| Capability | Confidence | Basis | Phase |
|------------|------------|-------|-------|
| Battery SOC Tracking | **High** | Energy conservation validated < 0.01% error (Test 09) | 1 |
| Voltage Sag Model | **High** | Li-Ion OCV polynomial fits published literature; SOC-dependent resistance validated (Test 06) | 2 |
| Motor Thermal Dynamics | **Medium** | Parameters estimated from motor class data; equilibrium behavior validated | 1 |
| Motor Time Constants | **Medium** | 150ms estimate based on motor class; no direct T-150 data | 1 |
| VRS Detection | **Medium-High** | State machine based on established aerodynamic theory [9, 10]; threshold values derived from momentum theory | 1 |
| Ground Effect | **Medium** | Cheeseman-Bennett approximation [8]; validated for general behavior (Test 07) | 4 |
| Payload Dynamics | **Medium** | Mass/CG model based on first principles; no specific T-150 loading data | 4 |
| Slung Load Dynamics | **Medium** | Spherical pendulum model well-established; no T-150-specific cable data | 5 |
| 6DOF Flight Dynamics | **Medium** | Newton-Euler physics validated for stability; inertia values estimated | 6 |
| Dynamic Power Model | **Medium-High** | Momentum theory + empirical validation; range within 10% of spec | 6 |
| Flight Endurance | **Medium** | Simulated 14.3 min vs. 36 min spec; calibration gap documented | 6 |
| Cruise Speed | **High** | Matches manufacturer specification of 30 m/s | - |
| Max Payload | **High** | Matches manufacturer specification of 68 kg (150 lbs) | - |

---

## 2. Threat System Description

### 2.1 Platform Overview

The T-150 (also designated TRV-150) is a heavy-lift unmanned aerial system manufactured by Malloy Aeronautics (UK) and marketed by SURVICE Engineering as the TRV-150. It is designed for autonomous cargo delivery, logistics resupply, and aerial transport missions.

The T-150 employs a coaxial octocopter configuration with eight rotors mounted in four coaxial pairs on four arms extending from a central fuselage. This configuration provides redundancy and high thrust-to-weight ratio suitable for heavy payload operations.

### 2.2 Platform Specifications

| Parameter | Value | Source |
|-----------|-------|--------|
| Configuration | Coaxial Octocopter (8 rotors, 4 arms) | AAIB Report [4] |
| Motors | 8x T-Motor U15II KV100 | FAA Registry / Technical Docs [6] |
| Total Thrust Capability | 240 kg (2,354 N) | AAIB Report [4] |
| Maximum Takeoff Weight (MTOW) | 123 kg (271 lbs) | AAIB Report [4] |
| Maximum Payload | 68 kg (150 lbs) | Manufacturer [11] |
| Empty Weight | ~55 kg (estimated) | Derived (MTOW - Max Payload) |
| Cruise Speed | 30 m/s (108 km/h, 58 kts) | Manufacturer [11] |
| Maximum Airspeed | 40+ m/s (demonstrated) | AAIB Report [4] |
| Maximum Rate of Climb | 8 m/s | Derived from thrust margin |
| Maximum Rate of Descent | 4 m/s (VRS limited) | Aerodynamic analysis |
| Battery Configuration | 13S Li-Ion | US Patent [5] |
| Battery Nominal Voltage | 46.8 V (13 x 3.6V) | US Patent [5] |
| Battery Capacity | ~7 kWh (148.75 Ah) | US Patent [5] |
| Flight Time | 36 minutes (nominal) | Manufacturer [11] |
| Propeller Diameter | 30 inches (0.76 m) | Technical Documentation |
| Arm Length | 0.5 m | Technical Documentation |
| Service Ceiling | 3,000 m | Derived from performance tables |

### 2.3 Concept of Operations (CONOPS)

The T-150 is designed for autonomous point-to-point cargo delivery missions. A typical operational profile consists of:

1. **Takeoff**: Vertical climb from ground level to operational altitude (typically 50-500 m AGL)
2. **Transit**: Cruise flight at 15-30 m/s to destination waypoint
3. **Delivery**: Descend to delivery point; payload release or landing
4. **Return**: Return transit to base location
5. **Landing**: Staged descent with final vertical landing

**Operational constraints**:
- Weather: Limited operations in high winds (>15 m/s sustained)
- VRS avoidance: Descent rates must remain below 4 m/s at low forward speeds
- Payload: Maximum 68 kg; center of gravity must remain within limits
- Battery: 20% minimum reserve recommended for safety margins

### 2.4 Threat Context

As a heavy-lift UAS, the T-150 represents a class of platforms with growing proliferation and operational significance. Intelligence questions relevant to this simulation include:

- What are the operational range and endurance limitations under various payload configurations?
- How does flight pattern efficiency affect mission planning (straight-line vs. evasive maneuvers)?
- What are the vulnerable flight regimes (VRS, low battery, thermal limits)?
- How do slung load dynamics affect stability and performance?
- What detection signatures (speed, altitude, flight profile) characterize this platform class?

---

## 3. Model Description

### 3.1 Model Functional Block Diagram

The T-150 simulation model consists of five plugin components communicating through the AFSIM platform's AuxData key-value store. The following diagram shows the data flow architecture:

```
+------------------------------------------------------------------+
|                       AFSIM Platform                              |
|                                                                   |
|  +------------------------------------------------------------+  |
|  |           Rotorcraft6DOF_Mover (Physics Engine)             |  |
|  |                                                              |  |
|  |  +--------------+  +---------------+  +-----------------+   |  |
|  |  | RigidBody6DOF|  | RotorModel[8] |  | FlightController|   |  |
|  |  | - Position   |  | - Thrust      |  | - Altitude PID  |   |  |
|  |  | - Velocity   |  | - Torque      |  | - Position PD   |   |  |
|  |  | - Quaternion |  | - RPM         |  | - Attitude PD   |   |  |
|  |  +--------------+  +---------------+  +-----------------+   |  |
|  +-----------------------------+------------------------------+   |
|                                |                                  |
|                    6dof_total_power (AuxData)                     |
|                                v                                  |
|  +----------------+   +----------------+   +------------------+  |
|  | T150_PROPULSION|   | T150_BATTERY   |   | T150_VRS         |  |
|  | _PROCESSOR     |   | _PROCESSOR     |   | _PROCESSOR       |  |
|  |                |   |                |   |                  |  |
|  | Motor states   |-->| SOC tracking   |   | Descent rate     |  |
|  | Thermal model  |   | Voltage sag    |   | VRS margin       |  |
|  | Ground effect  |   | Energy usage   |   | State machine    |  |
|  | Payload dyn.   |   | Current calc   |   | Lift/ctrl factor |  |
|  +-------+--------+   +--------+-------+   +--------+---------+  |
|          |                     |                     |            |
|          v                     v                     v            |
|  +-----------------------------------------------------------+   |
|  |                  Platform AuxData                          |   |
|  |  PROP_*, BATTERY_*, VRS_*, SLUNG_*, 6dof_*                 |   |
|  +-----------------------------------------------------------+   |
|          ^                                                        |
|  +------------------+                                             |
|  | T150_SLUNG_LOAD  |                                             |
|  | _PROCESSOR       |                                             |
|  |                  |                                             |
|  | Pendulum dyn.    |                                             |
|  | Cable tension    |                                             |
|  | Force coupling   |                                             |
|  +------------------+                                             |
+------------------------------------------------------------------+
```

**Data Flow Summary**:
1. The **6DOF Mover** computes rigid body dynamics and writes total power to AuxData
2. The **Battery Processor** reads power from AuxData (or from the Propulsion Processor) and computes SOC, voltage, current
3. The **Propulsion Processor** models individual motor states, thermal dynamics, and ground effect
4. The **VRS Processor** reads platform velocity from the mover and computes VRS safety margins
5. The **Slung Load Processor** reads platform acceleration and computes pendulum dynamics
6. All processors write their state to **AuxData** for cross-plugin access and scenario script queries

### 3.2 Model Fidelity Log

The model was developed iteratively over six enhancement phases:

| Phase | Date | Enhancement | Impact on Fidelity |
|-------|------|-------------|-------------------|
| 1 | 2025-11-29 | Thermal parameter fixes, VRS mover integration, motor time constants | Fixed unrealistic thermal behavior; enabled actual VRS detection |
| 2 | 2025-11-29 | Battery voltage sag model (Li-Ion OCV + SOC-dependent resistance) | Added voltage-dependent power delivery modeling |
| 3 | 2025-11-30 | Dynamic power model (battery reads from propulsion) | Battery drain now reflects actual flight conditions |
| 4 | 2025-11-30 | Ground effect (Cheeseman-Bennett) + payload dynamics | Added near-ground thrust augmentation and mass properties |
| 5 | 2025-11-30 | Slung load dynamics (2-DOF spherical pendulum) | Added cable-suspended payload behavior |
| 6 | 2026-01-08 | 6DOF physics mover, velocity-based power model | Full physics-based flight dynamics; power varies with maneuvers |

### 3.3 Key Performance Indicators (KPIs)

The following KPIs are the primary metrics used to assess model fidelity and to support analytical conclusions:

| KPI | Definition | Measured By | Expected Range | Validation Status |
|-----|------------|-------------|----------------|-------------------|
| Flight Endurance | Time from takeoff to battery depletion | BATTERY_SOC reaching 0% | 14-36 minutes | Partial (14.3 min simulated) |
| Cruise Range | Distance traveled at cruise speed before battery depletion | Distance integral during cruise | 25-45 km | Partial (matches at ~36 km with tuning) |
| Hover Power | Electrical power during stationary hover | PROP_TOTAL_POWER in hover | 17-25 kW | Validated (Test 08) |
| VRS Onset Speed | Descent rate at which VRS begins | VRS_MARGIN < 0.8 | ~4 m/s descent | Validated (VRS suite) |
| Battery Voltage Sag | Terminal voltage drop under load | BATTERY_VOLTAGE_SAG | 0-15 V | Validated (Test 06) |
| Motor Temperature | Peak motor temperature during operation | PROP_MAX_TEMP | 40-120 C | Validated (Test 01) |
| Slung Load Swing | Maximum swing angle of suspended payload | SLUNG_SWING_MAG | 0-45 deg | Validated (slung load suite) |
| Energy Efficiency | Energy consumed per straight-line km | Wh/km calculation | 315-800 Wh/km | Validated (pattern tests) |

---

## 4. Model Components

### 4.1 Rotorcraft 6DOF Mover

#### 4.1.1 Component Overview

The Rotorcraft6DOF_Mover is a custom AFSIM mover plugin that replaces the built-in `WSF_FULL_ROTORCRAFT_MOVER` with a full six-degree-of-freedom (6DOF) physics simulation. It provides physics-based flight dynamics using Newton-Euler rigid body equations, momentum theory rotor aerodynamics, and cascaded flight controllers.

This mover extends the AFSIM `WsfRouteMover` base class, enabling integration with AFSIM's route/waypoint navigation system while computing motion from first-principles physics rather than parametric tables.

**Plugin File**: `rotorcraft_6dof.dll` (Windows) / `librotorcraft_6dof.so` (Linux)
**AFSIM Type Name**: `Rotorcraft6DOF_Mover`
**Lines of Code**: 2,939 across 6 source files

#### 4.1.2 Functional Behavior

The 6DOF mover computes platform motion through the following physics pipeline executed each simulation timestep:

1. **Navigation**: Read next waypoint from AFSIM route; compute desired position/velocity
2. **Position Controller**: PD controller converts position error to desired velocity
3. **Velocity Controller**: P controller converts velocity error to desired tilt angles
4. **Attitude Controller**: PD controller converts attitude error to desired body moments
5. **Rotor Model**: Compute individual rotor thrust and torque from throttle commands
6. **Force Aggregation**: Sum all forces (gravity, rotor thrust, drag) in NED frame
7. **Rigid Body Integration**: Semi-implicit Euler integration of Newton-Euler equations
8. **State Update**: Update AFSIM platform position, velocity, and attitude

**Rigid Body Equations of Motion**:

Translational dynamics (Newton's second law in NED frame):

$$\mathbf{F}_{net} = m\mathbf{a}$$

$$\mathbf{F}_{net} = \mathbf{F}_{gravity} + R_{body \to NED} \cdot \mathbf{F}_{thrust} + \mathbf{F}_{drag}$$

Where:
- $m$ = total mass (kg)
- $\mathbf{a}$ = acceleration in NED frame (m/s^2)
- $R_{body \to NED}$ = rotation matrix from body to NED (derived from quaternion)
- $\mathbf{F}_{gravity} = [0, 0, mg]^T$ (NED, down positive)

Rotational dynamics (Euler's equation):

$$\mathbf{M} = I\dot{\boldsymbol{\omega}} + \boldsymbol{\omega} \times (I\boldsymbol{\omega})$$

Where:
- $\mathbf{M}$ = net moment vector (Nm)
- $I$ = inertia tensor (diagonal: $I_{xx}$, $I_{yy}$, $I_{zz}$)
- $\boldsymbol{\omega}$ = angular velocity vector (rad/s)

**Attitude Representation**: Quaternion-based (avoids gimbal lock):

$$\dot{q} = \frac{1}{2} q \otimes \boldsymbol{\omega}_q$$

Where $\boldsymbol{\omega}_q = [0, \omega_x, \omega_y, \omega_z]$ is the angular velocity as a pure quaternion.

**Integration Method**: Semi-implicit Euler (symplectic):

$$\boldsymbol{\omega}(t+\Delta t) = \boldsymbol{\omega}(t) + \dot{\boldsymbol{\omega}} \cdot \Delta t$$
$$q(t+\Delta t) = q(t) + \dot{q}(t+\Delta t) \cdot \Delta t$$
$$\mathbf{v}(t+\Delta t) = \mathbf{v}(t) + \mathbf{a} \cdot \Delta t$$
$$\mathbf{p}(t+\Delta t) = \mathbf{p}(t) + \mathbf{v}(t+\Delta t) \cdot \Delta t$$

This method preserves energy better than explicit Euler for oscillatory systems.

**Rotor Thrust (Momentum Theory)**:

$$T = C_T \cdot \rho \cdot n^2 \cdot D^4$$

Where:
- $C_T$ = thrust coefficient (0.10)
- $\rho$ = air density (1.225 kg/m^3 at sea level)
- $n$ = rotational speed (rev/s = RPM/60)
- $D$ = propeller diameter (0.76 m)

**Rotor Torque**:

$$Q = C_Q \cdot \rho \cdot n^2 \cdot D^5$$

Where $C_Q$ = torque coefficient (0.008).

#### 4.1.3 Key Performance Indicators

| KPI | Measure | Expected | Validation |
|-----|---------|----------|------------|
| Position hold accuracy | Drift during hover (m) | < 1.0 m | Test 03: ~1e-17 m/s drift |
| Attitude recovery | Time to recover from disturbance | < 5 s | Test 02: Clean recovery |
| Velocity tracking | Steady-state error (m/s) | < 1.0 m/s | Test 04: 14.49 vs 15.0 m/s |
| Numerical stability | No NaN/divergence under stress | Bounded | Test 05: Stable under extremes |
| Max tilt angle | Maximum bank/pitch during maneuvers | 25 deg | Test 05: 25 deg achieved |

#### 4.1.4 Parameters

| Parameter | Default | Units | Description | User Tunable |
|-----------|---------|-------|-------------|--------------|
| `mass` | 123 | kg | Total platform mass (MTOW) | Yes (scenario) |
| `inertia_xx` | 2.5 | kg*m^2 | Roll moment of inertia | Yes (scenario) |
| `inertia_yy` | 2.5 | kg*m^2 | Pitch moment of inertia | Yes (scenario) |
| `inertia_zz` | 4.5 | kg*m^2 | Yaw moment of inertia | Yes (scenario) |
| `rotor_diameter` | 0.76 | m | Propeller diameter | Yes (scenario) |
| `arm_length` | 0.5 | m | Motor arm length from CG | Yes (scenario) |
| `thrust_coefficient` | 0.10 | - | Ct for momentum theory | Yes (scenario) |
| `torque_coefficient` | 0.008 | - | Cq for reaction torque | Yes (scenario) |
| `max_rpm` | 4500 | RPM | Maximum motor RPM | Yes (scenario) |
| `max_speed` | 30 | m/s | Maximum horizontal speed | Yes (scenario) |
| `max_climb_rate` | 8 | m/s | Maximum climb rate | Yes (scenario) |
| `max_descent_rate` | 4 | m/s | Maximum descent rate | Yes (scenario) |
| `max_tilt_angle` | 25 | deg | Maximum bank/pitch angle | Yes (scenario) |
| `altitude_kp` | 0.3 | - | Altitude PID proportional gain | Yes (scenario) |
| `altitude_ki` | 0.05 | - | Altitude PID integral gain | Yes (scenario) |
| `altitude_kd` | 1.2 | - | Altitude PID derivative gain | Yes (scenario) |
| `position_kp` | 1.0 | - | Position PD proportional gain | Yes (scenario) |
| `position_kd` | 0.1 | - | Position PD derivative gain | Yes (scenario) |
| `velocity_kp` | 3.0 | - | Velocity P gain | Yes (scenario) |
| `attitude_kp` | 8.0 | - | Attitude PD proportional gain | Yes (scenario) |
| `attitude_kd` | 2.0 | - | Attitude PD derivative gain | Yes (scenario) |

#### 4.1.5 Assumptions, Limitations, and Gaps

**Assumptions**:
- Rigid body dynamics (no structural flexibility)
- Constant air density (no altitude-dependent atmospheric model)
- Symmetric inertia tensor (diagonal only; no cross-coupling products)
- Instantaneous force application (no actuator dynamics beyond motor time constant)

**Limitations**:
- Maximum tilt angle capped at 25 degrees (actual aircraft may achieve more aggressive attitudes)
- No wind model — all flight occurs in still air
- No sensor noise injection (perfect state knowledge for controllers)
- Coaxial rotor interference modeled as a constant factor (0.85), not dynamic

**Gaps**:
- Inertia values are estimated, not measured from actual T-150 hardware
- No BEMT (Blade Element Momentum Theory) rotor model — uses simplified momentum theory
- No structural vibration or aeroelastic effects

---

### 4.2 Battery Processor

#### 4.2.1 Component Overview

The T150_BATTERY_PROCESSOR models a 13S Lithium-Ion battery pack with state-of-charge tracking, open-circuit voltage modeling, voltage sag under load, and SOC-dependent internal resistance. It reads dynamic power consumption from the 6DOF mover or Propulsion Processor via AuxData and computes real-time battery state.

**Plugin File**: `t150_battery.dll` (Windows) / `libt150_battery_processor.so` (Linux)
**AFSIM Type Name**: `T150_BATTERY_PROCESSOR`
**Lines of Code**: 1,139

#### 4.2.2 Functional Behavior

The Battery Processor executes the following computation each update cycle:

1. **Read Power Demand**: Read `6dof_total_power` or `PROP_TOTAL_POWER` from AuxData (falls back to configured `hover_power_w` if unavailable)
2. **Apply Efficiency**: Divide by motor efficiency to get electrical power
3. **Add Auxiliary Load**: Add avionics/cooling power
4. **Compute OCV**: Calculate open-circuit voltage from SOC using polynomial fit
5. **Compute Internal Resistance**: Apply SOC-dependent resistance multiplier
6. **Solve for Current**: Use quadratic formula to find stable current solution
7. **Compute Terminal Voltage**: OCV minus IR drop
8. **Update Energy Consumed**: Integrate power over timestep
9. **Update SOC**: Calculate remaining state of charge
10. **Check Warnings**: Evaluate low-battery and voltage sag thresholds
11. **Write to AuxData**: Publish all battery state variables

**Open-Circuit Voltage (OCV) Model**:

The OCV follows a polynomial fit to a typical Li-Ion discharge curve:

$$V_{cell} = a_0 + a_1 \cdot SOC + a_2 \cdot SOC^2 + a_3 \cdot SOC^3$$

Where:
- $a_0 = 3.0$ V (cutoff voltage per cell)
- $a_1 = 1.2$ (linear term)
- $a_2 = -0.5$ (creates flat middle region)
- $a_3 = 0.5$ (steeper at ends)

Pack voltage:
$$V_{pack} = V_{cell} \times 13$$

**Voltage Sag Model**:

Under load, terminal voltage drops due to internal resistance:

$$V_{terminal} = V_{OCV} - I \cdot R_{internal}$$

Since $P = V_{terminal} \cdot I$, combining equations:

$$R \cdot I^2 - V_{OCV} \cdot I + P = 0$$

Solving via quadratic formula:

$$I = \frac{V_{OCV} - \sqrt{V_{OCV}^2 - 4 \cdot R \cdot P}}{2 \cdot R}$$

The smaller root is selected as the physically meaningful solution (lower current, higher efficiency).

**SOC-Dependent Internal Resistance**:

Internal resistance follows a U-shaped curve:

$$R_{eff} = R_{base} \times f(SOC)$$

Where:
- SOC > 0.6: $f = 1.0 + 0.3 \times \frac{(SOC - 0.6)^2}{0.16}$ (peaks ~1.3x at 100%)
- 0.2 < SOC < 0.6: $f = 1.0$ (minimum in middle range)
- SOC < 0.2: $f = 1.0 + 2.0 \times \frac{(0.2 - SOC)^2}{0.04}$ (peaks ~3.0x at 0%)

**Energy Consumption**:

$$E_{consumed}(t) = E_{consumed}(t-1) + P_{total} \cdot \frac{\Delta t}{3600}$$

$$SOC = 1.0 - \frac{E_{consumed}}{E_{capacity}}$$

#### 4.2.3 Key Performance Indicators

| KPI | Measure | Expected | Validation |
|-----|---------|----------|------------|
| Energy conservation | SOC tracking error | < 0.01% | Test 09: < 0.01% |
| Voltage sag at full load | Voltage drop (V) | 5-15 V | Test 06: Validated |
| Low SOC tracking | Correct SOC from non-100% start | Monotonic decrease | Test 06: Fixed and validated |
| Flight endurance | Time to battery depletion | 14-36 min | Test 10: 14.3 min |
| Warning triggers | Low battery / voltage warnings | At configured thresholds | Test 10: All triggered |

#### 4.2.4 Parameters

| Parameter | Default | Units | Description | User Tunable |
|-----------|---------|-------|-------------|--------------|
| `battery_capacity_wh` | 7000.0 | Wh | Total battery energy capacity | Yes (scenario) |
| `initial_soc` | 1.0 | 0-1 | Initial state of charge | Yes (scenario) |
| `hover_power_w` | 18000.0 | W | Fallback hover power (if dynamic unavailable) | Yes (scenario) |
| `motor_efficiency` | 0.85 | 0-1 | Motor/ESC electrical efficiency | Yes (scenario) |
| `auxiliary_power_w` | 4000.0 | W | Avionics, cooling, other loads | Yes (scenario) |
| `low_battery_warning` | 0.20 | 0-1 | SOC threshold for low-battery warning | Yes (scenario) |
| `critical_battery_level` | 0.10 | 0-1 | SOC threshold for critical warning | Yes (scenario) |
| `nominal_voltage` | 46.8 | V | Pack nominal voltage (13S x 3.6V) | Yes (scenario) |
| `full_charge_voltage` | 54.6 | V | Pack full charge voltage (13S x 4.2V) | Yes (scenario) |
| `cutoff_voltage` | 39.0 | V | Pack cutoff voltage (13S x 3.0V) | Yes (scenario) |
| `internal_resistance` | 0.025 | Ohm | Pack internal resistance | Yes (scenario) |
| `voltage_sag_warning` | 42.0 | V | Voltage threshold for sag warning | Yes (scenario) |
| `use_dynamic_power` | true | bool | Read power from mover/propulsion AuxData | Yes (scenario) |
| `power_source_key` | "6dof_total_power" | string | AuxData key for dynamic power input | Yes (scenario) |
| `csv_output_file` | "" | path | Optional CSV logging file path | Yes (scenario) |
| `update_interval` | 0.1 | sec | Processor update rate | Yes (scenario) |

#### 4.2.5 Assumptions, Limitations, and Gaps

**Assumptions**:
- Li-Ion chemistry with characteristic polynomial discharge curve
- Uniform cell aging across all 13 series cells
- Constant temperature effects on internal resistance (no thermal feedback from environment)
- Motor efficiency is constant across operating range

**Limitations**:
- No battery degradation over charge/discharge cycles
- No temperature-dependent capacity derating
- Internal resistance model is simplified (no Randles circuit or EIS-based model)
- No cell-level balancing effects modeled

**Gaps**:
- Actual T-150 battery pack resistance not measured; 25 mOhm is estimated for pack-level resistance
- OCV polynomial coefficients are generic Li-Ion, not specific to T-150 cell chemistry
- Auxiliary power (4 kW) may be high; typical avionics draw 0.5-2 kW

---

### 4.3 Propulsion Processor

#### 4.3.1 Component Overview

The T150_PROPULSION_PROCESSOR models the 8-motor coaxial propulsion system including individual motor state tracking (RPM, temperature, efficiency), thrust calculation via momentum theory, motor thermal dynamics, ground effect thrust augmentation, and payload mass management.

**Plugin File**: `t150_propulsion.dll` (Windows) / `libt150_propulsion.so` (Linux)
**AFSIM Type Name**: `T150_PROPULSION_PROCESSOR`
**Lines of Code**: 1,281

#### 4.3.2 Functional Behavior

The Propulsion Processor computes the following each update cycle:

1. **Motor RPM**: Calculate target RPM from voltage and KV rating; apply first-order lag
2. **Motor Thrust**: Compute per-motor thrust using momentum theory
3. **Motor Torque**: Compute per-motor reaction torque
4. **Motor Efficiency**: Parabolic model based on RPM ratio
5. **Electrical Power**: Mechanical power divided by efficiency
6. **Thermal Update**: Compute heat generation and dissipation
7. **Ground Effect**: Apply Cheeseman-Bennett thrust augmentation if near ground
8. **Payload**: Track total mass and CG effects
9. **Dynamic Power**: Calculate total power from flight state (velocity, climb, bank)
10. **Write AuxData**: Publish all propulsion state variables

**Motor RPM with First-Order Lag**:

$$RPM_{target} = KV \times V_{nominal} \times 0.85$$

$$\alpha = \frac{\Delta t}{\tau + \Delta t}$$

$$RPM_{new} = RPM_{old} + \alpha \times (RPM_{target} - RPM_{old})$$

Where $\tau = 0.15$ s (150 ms time constant for U15II with 30" propeller).

**Motor Efficiency (Parabolic Model)**:

$$\eta_{RPM} = 1.0 - 0.5 \times (RPM_{ratio} - 0.7)^2$$

$$\eta_{torque} = 1.0 - 0.2 \times Torque_{ratio}$$

$$\eta = \eta_{peak} \times \eta_{RPM} \times \eta_{torque}$$

Peak efficiency occurs at approximately 70% of maximum RPM.

**Thermal Model**:

$$\dot{Q}_{in} = P_{electrical} \times (1 - \eta)$$

$$\dot{Q}_{out} = k_{cooling} \times (T_{motor} - T_{ambient})$$

$$\frac{dT}{dt} = \frac{\dot{Q}_{in} - \dot{Q}_{out}}{C_{thermal}}$$

Where:
- $C_{thermal} = 1500$ J/K (motor thermal mass)
- $k_{cooling} = 20$ W/K (forced convection from prop wash)

**Ground Effect (Cheeseman-Bennett Approximation)**:

$$\frac{T_{IGE}}{T_{OGE}} = \frac{1}{1 - \left(\frac{R}{4h}\right)^2}$$

Where:
- $R$ = rotor radius (0.38 m)
- $h$ = rotor height above ground

Active below 1 rotor diameter (0.76 m) AGL. Clamped to maximum 30% thrust boost.

**Dynamic Power Model (Phase 6)**:

$$P_{total} = P_{induced} + P_{climb} + P_{parasite} + P_{accel}$$

| Component | Equation |
|-----------|----------|
| Induced Power | $P_{induced} = k \cdot T \cdot v_i$ where $v_i = \sqrt{T/(2\rho A)}$ |
| Climb Power | $P_{climb} = m \cdot g \cdot V_{climb}$ |
| Parasite Power | $P_{parasite} = 0.5 \cdot \rho \cdot C_d \cdot A_f \cdot V^3$ |
| Acceleration Power | $P_{accel} = m \cdot a \cdot V$ |

Thrust adjustment for bank angle (coordinated turns):

$$T_{required} = \frac{m \cdot g}{\cos(\phi)}$$

Where $\phi$ = bank angle estimated from horizontal acceleration:
$$\phi = \arctan\left(\frac{a_{horizontal}}{g}\right)$$

#### 4.3.3 Key Performance Indicators

| KPI | Measure | Expected | Validation |
|-----|---------|----------|------------|
| Hover power | Electrical power in hover | 17-25 kW | Test 08: 19.5-20.1 kW mechanical |
| Motor temperature | Equilibrium temperature | 60-90 C | Test 01: Stable |
| Ground effect factor | Thrust multiplier near ground | 1.0-1.3 | Test 07: Functional |
| Power vs. speed | Power variation with airspeed | Cruise < hover | Validated (pattern tests) |

#### 4.3.4 Parameters

| Parameter | Default | Units | Description | User Tunable |
|-----------|---------|-------|-------------|--------------|
| `motor_count` | 8 | - | Number of motors | Yes (scenario) |
| `kv_rating` | 100.0 | RPM/V | Motor KV rating | Yes (scenario) |
| `max_power_per_motor` | 3500.0 | W | Maximum power per motor | Yes (scenario) |
| `propeller_diameter` | 0.76 | m | Propeller diameter (30 in) | Yes (scenario) |
| `coaxial_efficiency` | 0.85 | 0-1 | Coaxial rotor interference factor | Yes (scenario) |
| `motor_thermal_limit` | 120.0 | C | Motor overtemperature threshold | Yes (scenario) |
| `motor_thermal_mass` | 1500.0 | J/K | Motor thermal mass | Yes (scenario) |
| `motor_cooling_rate` | 20.0 | W/K | Forced convection cooling rate | Yes (scenario) |
| `motor_time_constant` | 0.15 | s | RPM response time constant | Yes (scenario) |
| `ground_effect_enabled` | true | bool | Enable ground effect modeling | Yes (scenario) |
| `empty_mass` | 68.0 | kg | Empty aircraft mass | Yes (scenario) |
| `payload_mass` | 0.0 | kg | Current payload mass | Yes (scenario) |
| `max_payload_mass` | 55.0 | kg | Maximum payload capacity | Yes (scenario) |
| `update_interval` | 0.1 | sec | Processor update rate | Yes (scenario) |

#### 4.3.5 Assumptions, Limitations, and Gaps

**Assumptions**:
- All 8 motors operate identically (no individual motor failures modeled)
- Coaxial interference is a constant factor (0.85), not RPM/inflow-dependent
- Air density is constant (sea-level ISA, 1.225 kg/m^3)
- Motor efficiency peaks at 70% RPM (generic assumption for large brushless motors)

**Limitations**:
- No individual motor failure or degradation modes
- Propeller aerodynamics use simplified momentum theory (no BEMT)
- No blade pitch variation (fixed-pitch propellers)
- Ground effect uses single-rotor approximation (does not model multi-rotor ground effect interaction)

**Gaps**:
- Motor thermal parameters estimated from motor class, not specific U15II test data
- Coaxial interference factor (0.85) is generic; actual T-150 interference may differ
- No wind/gust model affects propulsion performance

---

### 4.4 VRS Processor

#### 4.4.1 Component Overview

The T150_VRS_PROCESSOR detects and models Vortex Ring State (VRS) conditions, a dangerous aerodynamic phenomenon where a rotorcraft descends into its own rotor downwash, causing severe loss of lift and control authority. The processor monitors descent rate and forward speed, implements a 5-state machine, and computes lift degradation and control authority factors.

**Plugin File**: `t150_vrs.dll` (Windows) / `libt150_vrs.so` (Linux)
**AFSIM Type Name**: `T150_VRS_PROCESSOR`
**Lines of Code**: 697

#### 4.4.2 Functional Behavior

The VRS Processor reads platform velocity from the AFSIM mover each update cycle and evaluates VRS risk:

1. **Read Velocity**: Get NED velocity from platform mover
2. **Compute Descent Rate**: $V_d = \max(0, V_{down})$ (positive = descending)
3. **Compute Forward Speed**: $V_{fwd} = \sqrt{V_N^2 + V_E^2}$
4. **Compute VRS Margin**: Combined margin from descent rate and forward speed
5. **Update State Machine**: Transition between 5 VRS states
6. **Compute Degradation**: Calculate lift and control authority factors
7. **Write AuxData**: Publish VRS state, margin, and factors

**VRS Margin Calculation**:

Descent margin:
$$m_{descent} = \begin{cases} 1.0 & \text{if } V_d \leq 0 \\ 1.0 - V_d / V_{threshold} & \text{if } 0 < V_d < V_{threshold} \\ -(V_d - V_{threshold}) / V_{threshold} & \text{if } V_d \geq V_{threshold} \end{cases}$$

Speed margin:
$$m_{speed} = \min(1.0, V_{fwd} / V_{speed\_threshold})$$

Combined margin:
$$m = \begin{cases} 1.0 & \text{if } V_{fwd} \geq V_{speed\_threshold} \\ m_{descent} \cdot w + m_{speed} \cdot (1 - w) & \text{otherwise} \end{cases}$$

Where $w = 1.0 - m_{speed}$ (weighting factor).

**State Machine**:

```
                    margin < 0.8
    +---------+ -------------------> +---------+
    |  CLEAR  |                     | CAUTION |
    +---------+ <------------------ +---------+
                    margin >= 1.0        |
                                         | margin < 0.5
                                         v
    +-----------+                   +---------+
    | RECOVERY  | <---------------- |  ONSET  |
    +-----------+   margin >= 0.8   +---------+
         |                               |
         | development cleared           | margin < 0.2
         |                               | for 2 seconds
         v                               v
    +-----------+                   +-----------+
    |   CLEAR   |                   | DEVELOPED |
    +-----------+   margin >= 0.5   +-----------+
                  <------------------
```

**Performance Degradation Factors**:

| State | Lift Factor | Control Factor |
|-------|-------------|----------------|
| CLEAR | 1.00 | 1.00 |
| CAUTION | 0.95 | 1.00 |
| ONSET | 0.85 | 0.90 |
| DEVELOPED | 0.40 | 0.50 |
| RECOVERY | Proportional to development | Proportional |

#### 4.4.3 Parameters

| Parameter | Default | Units | Description | User Tunable |
|-----------|---------|-------|-------------|--------------|
| `vrs_descent_threshold` | 4.0 | m/s | Descent rate for VRS risk | Yes (scenario) |
| `vrs_speed_threshold` | 5.0 | m/s | Forward speed for VRS immunity | Yes (scenario) |
| `caution_margin` | 0.8 | 0-1 | Margin threshold for CAUTION state | Yes (scenario) |
| `induced_velocity` | 8.5 | m/s | Rotor induced velocity in hover | Yes (scenario) |
| `coaxial_factor` | 0.85 | 0-1 | Coaxial VRS resistance factor | Yes (scenario) |
| `update_interval` | 0.05 | sec | High-rate update for VRS detection (20 Hz) | Yes (scenario) |

#### 4.4.4 Assumptions, Limitations, and Gaps

**Assumptions**:
- VRS boundaries based on momentum theory induced velocity ratio
- Coaxial configuration provides inherent VRS resistance (factor 0.85)
- Recovery requires either forward speed increase or descent rate reduction

**Limitations**:
- VRS is detected but does not feed back to modify mover dynamics (informational only)
- No turbulence-induced VRS modeling
- Development time (2s to full VRS) is estimated, not measured

**Gaps**:
- No coaxial-specific VRS boundary data; factor is estimated
- Lift and control degradation factors are representative, not from T-150 flight test

---

### 4.5 Slung Load Processor

#### 4.5.1 Component Overview

The T150_SLUNG_LOAD_PROCESSOR models cable-suspended payload dynamics using a 2-DOF spherical pendulum model. It computes swing angles, cable tension, and reaction forces on the aircraft from suspended load motion excited by aircraft maneuvers.

**Plugin File**: `t150_slung_load.dll` (Windows) / `libt150_slung_load.so` (Linux)
**AFSIM Type Name**: `T150_SLUNG_LOAD_PROCESSOR`
**Lines of Code**: 825

#### 4.5.2 Functional Behavior

The Slung Load Processor integrates pendulum dynamics at 50 Hz:

1. **Read Aircraft State**: Get velocity and compute acceleration from mover
2. **Compute Excitation**: Aircraft acceleration drives pendulum
3. **Integrate Pendulum Equations**: Semi-implicit Euler at 50 Hz
4. **Compute Cable Tension**: Static + centripetal components
5. **Compute Reaction Forces**: Forces on aircraft from load
6. **Check States**: Detect resonance, overload, or release conditions
7. **Write AuxData**: Publish all slung load state variables

**Spherical Pendulum Equations of Motion**:

$$\ddot{\theta}_x = -\frac{g}{L}\sin(\theta_x) - \frac{a_x}{L}\cos(\theta_x) - c \cdot \dot{\theta}_x$$

$$\ddot{\theta}_y = -\frac{g}{L}\sin(\theta_y) - \frac{a_y}{L}\cos(\theta_y) - c \cdot \dot{\theta}_y$$

Where:
- $\theta_x$, $\theta_y$ = swing angles in X and Y planes (rad)
- $g = 9.81$ m/s^2
- $L$ = cable length (m)
- $a_x$, $a_y$ = aircraft acceleration (excitation source)
- $c$ = cable damping coefficient (1/s)

**Natural Frequency**:

$$f_n = \frac{1}{2\pi}\sqrt{\frac{g}{L}}$$

For a 5 m cable: $f_n = 0.223$ Hz (4.5 second period).

**Cable Tension**:

$$T = m \cdot (g \cos(\theta) - a_z + L \cdot \omega^2)$$

Where $\omega = \sqrt{\dot{\theta}_x^2 + \dot{\theta}_y^2}$ (angular velocity magnitude).

Constraints: $T \geq 0$ (cable cannot push) and $T \leq T_{max}$ (cable strength limit).

**Reaction Forces on Aircraft** (body frame):

$$F_x = T \sin(\theta_x), \quad F_y = T \sin(\theta_y), \quad F_z = T \cos(\theta)$$

#### 4.5.3 Parameters

| Parameter | Default | Units | Description | User Tunable |
|-----------|---------|-------|-------------|--------------|
| `load_mass` | 0.0 | kg | Suspended load mass | Yes (scenario) |
| `cable_length` | 5.0 | m | Cable length | Yes (scenario) |
| `cable_damping` | 0.1 | 1/s | Air/friction damping coefficient | Yes (scenario) |
| `attach_point_z` | 0.5 | m | Attach point below CG | Yes (scenario) |
| `max_cable_tension` | 5000.0 | N | Maximum cable tension before failure | Yes (scenario) |
| `resonance_threshold` | 20.0 | deg | Swing angle for resonance warning | Yes (scenario) |
| `update_interval` | 0.02 | sec | Dynamics update rate (50 Hz) | Yes (scenario) |

#### 4.5.4 Assumptions, Limitations, and Gaps

**Assumptions**:
- Cable is massless and inextensible
- Load is a point mass (no aerodynamic drag on load)
- Small-angle behavior for linearized natural frequency
- Cable damping is constant (not velocity-dependent)

**Limitations**:
- 2-DOF spherical pendulum only (no cable elasticity or bending)
- No aerodynamic drag on the suspended load
- No load release/pickup dynamics
- Reaction forces are informational; do not feed back to mover dynamics

**Gaps**:
- No T-150-specific cable data; damping coefficient is estimated
- Cable breaking strength (5000 N) is generic
- No multi-point suspension modeling

---

## 5. Model Setup and Configuration

### 5.1 Scenario Configuration

The T-150 model is configured through AFSIM scenario files (.afsim). There is no GUI; all parameters are set in text-based scenario scripts.

**Note**: This section replaces the ODESSA GUI setup section. AFSIM uses text-based scenario configuration rather than a graphical interface.

#### 5.1.1 Installation

```bash
# Clone the repository
git clone https://github.com/DSGarrett/project005_t150_uas.git
cd project005_t150_uas

# Build plugins
./ngic_afsim_build.sh

# Install to AFSIM (requires elevated privileges)
sudo ./ngic_afsim_install.sh
```

Plugins are installed to:
- Linux: `/opt/afsim*/lib/wsf_plugins/` and `/opt/afsim*/bin/mission_plugins/`
- Windows: `C:\afsim\AFSIM-2.2505\lib\wsf_plugins\` and `C:\afsim\AFSIM-2.2505\bin\mission_plugins\`

#### 5.1.2 Minimal Platform Definition

```afsim
platform_type T150_BASIC WSF_PLATFORM
    icon t150

    mover Rotorcraft6DOF_Mover
        mass 123 kg
        max_speed 30 m/s
    end_mover

    processor BATTERY T150_BATTERY_PROCESSOR
        battery_capacity_wh 7000
        use_dynamic_power true
    end_processor

end_platform_type
```

#### 5.1.3 Full Configuration Example

```afsim
platform_type T150_FULL_PHYSICS T150_UAS

    // Battery with voltage sag model
    processor BATTERY_MONITOR T150_BATTERY_PROCESSOR
        battery_capacity_wh    7000.0
        initial_soc            1.0
        nominal_voltage        46.8
        internal_resistance    0.025
        use_dynamic_power      true
    end_processor

    // Propulsion with thermal and ground effect
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

    // VRS detection
    processor VRS_MONITOR T150_VRS_PROCESSOR
        vrs_descent_threshold  4.0
        vrs_speed_threshold    5.0
        coaxial_factor         0.85
    end_processor

    // Slung load dynamics
    processor SLUNG_LOAD T150_SLUNG_LOAD_PROCESSOR
        load_mass              25.0
        cable_length           6.0
        cable_damping          0.1
    end_processor

end_platform_type
```

### 5.2 Model Parameters (Master Table)

The following table provides the complete list of all configurable parameters across all plugins:

| Variable | Description | Default | Component | User Tunable | Source |
|----------|-------------|---------|-----------|--------------|--------|
| `mass` | Total platform mass | 123 kg | 6DOF Mover | Yes | [4] |
| `inertia_xx` | Roll moment of inertia | 2.5 kg*m^2 | 6DOF Mover | Yes | Estimated |
| `inertia_yy` | Pitch moment of inertia | 2.5 kg*m^2 | 6DOF Mover | Yes | Estimated |
| `inertia_zz` | Yaw moment of inertia | 4.5 kg*m^2 | 6DOF Mover | Yes | Estimated |
| `rotor_diameter` | Propeller diameter | 0.76 m | 6DOF Mover | Yes | [6] |
| `thrust_coefficient` | Ct for momentum theory | 0.10 | 6DOF Mover | Yes | [9] |
| `max_speed` | Maximum horizontal speed | 30 m/s | 6DOF Mover | Yes | [11] |
| `battery_capacity_wh` | Total battery capacity | 7000 Wh | Battery | Yes | [5] |
| `nominal_voltage` | Pack nominal voltage | 46.8 V | Battery | Yes | [5] |
| `internal_resistance` | Pack resistance | 0.025 Ohm | Battery | Yes | Estimated |
| `motor_efficiency` | Motor/ESC efficiency | 0.85 | Battery | Yes | [6] |
| `motor_count` | Number of motors | 8 | Propulsion | No | [4] |
| `kv_rating` | Motor KV rating | 100 RPM/V | Propulsion | Yes | [6] |
| `coaxial_efficiency` | Coaxial interference | 0.85 | Propulsion | Yes | [9] |
| `motor_thermal_mass` | Motor thermal mass | 1500 J/K | Propulsion | Yes | Estimated |
| `motor_cooling_rate` | Cooling rate | 20 W/K | Propulsion | Yes | Estimated |
| `vrs_descent_threshold` | VRS descent rate | 4.0 m/s | VRS | Yes | [10] |
| `vrs_speed_threshold` | VRS forward speed | 5.0 m/s | VRS | Yes | [10] |
| `load_mass` | Slung load mass | 0.0 kg | Slung Load | Yes | Scenario |
| `cable_length` | Cable length | 5.0 m | Slung Load | Yes | Scenario |
| `cable_damping` | Cable damping | 0.1 1/s | Slung Load | Yes | Estimated |

### 5.3 Model Specific Log Files

The simulation can generate the following output files:

| Output | Format | Content | Configuration |
|--------|--------|---------|---------------|
| Battery CSV | CSV | SOC, voltage, current, power per timestep | `csv_output_file` parameter |
| AFSIM AER | AER (binary) | Full simulation event records | `event_pipe` in scenario |
| AFSIM Log | Text | Console output with processor messages | `log_file` in scenario |
| Telemetry CSV | CSV | 6DOF state: position, velocity, attitude | Optional 6DOF logging |

---

## 6. Analytic Outputs

### 6.1 AuxData Output Keys

All model state is published to the AFSIM AuxData container, accessible from scenario scripts and other processors. The following tables list all output keys by component.

**Battery Processor Outputs**:

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `BATTERY_SOC` | double | 0-1 | State of charge |
| `BATTERY_POWER_W` | double | W | Current power draw |
| `BATTERY_ENERGY_WH` | double | Wh | Remaining energy |
| `BATTERY_VOLTAGE` | double | V | Terminal voltage under load |
| `BATTERY_OCV` | double | V | Open-circuit voltage |
| `BATTERY_VOLTAGE_SAG` | double | V | Voltage drop from IR |
| `BATTERY_CURRENT_A` | double | A | Current draw |
| `BATTERY_LOW` | bool | - | Low battery warning flag |
| `BATTERY_VOLTAGE_LOW` | bool | - | Voltage sag warning flag |

**Propulsion Processor Outputs**:

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `PROP_TOTAL_THRUST` | double | N | Total thrust output |
| `PROP_TOTAL_POWER` | double | W | Total electrical power |
| `PROP_SYSTEM_EFF` | double | 0-1 | System efficiency |
| `PROP_MOTOR_COUNT` | int | - | Operational motor count |
| `PROP_MAX_TEMP` | double | C | Hottest motor temperature |
| `PROP_AVG_TEMP` | double | C | Average motor temperature |
| `PROP_GROUND_EFFECT` | double | 1.0-1.3 | Ground effect factor |
| `PROP_ALTITUDE_AGL` | double | m | Altitude above ground |
| `PROP_TOTAL_MASS` | double | kg | Aircraft + payload mass |
| `PROP_PAYLOAD_MASS` | double | kg | Payload mass |
| `PROP_THRUST_MARGIN` | double | 0-1 | Available thrust reserve |
| `PROP_CG_OFFSET_X` | double | m | CG offset X |
| `PROP_CG_OFFSET_Y` | double | m | CG offset Y |
| `PROP_CG_OFFSET_Z` | double | m | CG offset Z |
| `PROP_OVERLOADED` | bool | - | Payload exceeds capacity |
| `PROP_FORWARD_SPEED` | double | m/s | Horizontal speed |
| `PROP_CLIMB_RATE` | double | m/s | Vertical speed (positive=up) |
| `PROP_BANK_ANGLE` | double | deg | Estimated bank angle |
| `PROP_ACCEL_MAG` | double | m/s^2 | Horizontal acceleration |

**VRS Processor Outputs**:

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `VRS_MARGIN` | double | 0-1 | Safety margin (0=VRS, 1=safe) |
| `VRS_STATE` | int | 0-4 | State (CLEAR=0, CAUTION=1, ONSET=2, DEVELOPED=3, RECOVERY=4) |
| `VRS_LIFT_FACTOR` | double | 0.4-1.0 | Lift degradation factor |
| `VRS_CONTROL_FACTOR` | double | 0.5-1.0 | Control authority factor |
| `VRS_DEVELOPMENT` | double | 0-1 | VRS development level |
| `VRS_DESCENT_RATE` | double | m/s | Current descent rate |
| `VRS_FORWARD_SPEED` | double | m/s | Current forward speed |
| `VRS_CAUTION` | bool | - | Caution warning flag |
| `VRS_ONSET` | bool | - | Onset warning flag |
| `VRS_CRITICAL` | bool | - | Critical (developed VRS) flag |

**Slung Load Processor Outputs**:

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `SLUNG_LOAD_STATE` | int | 0-4 | Load state (DETACHED=0, ATTACHED=1, SWINGING=2, RESONANCE=3, RELEASED=4) |
| `SLUNG_LOAD_MASS` | double | kg | Load mass |
| `SLUNG_CABLE_LENGTH` | double | m | Cable length |
| `SLUNG_CABLE_TENSION` | double | N | Cable tension |
| `SLUNG_SWING_X` | double | deg | Swing angle X plane |
| `SLUNG_SWING_Y` | double | deg | Swing angle Y plane |
| `SLUNG_SWING_MAG` | double | deg | Total swing magnitude |
| `SLUNG_FORCE_X` | double | N | Reaction force on aircraft X |
| `SLUNG_FORCE_Y` | double | N | Reaction force on aircraft Y |
| `SLUNG_FORCE_Z` | double | N | Reaction force on aircraft Z |
| `SLUNG_POS_X` | double | m | Load position X relative to aircraft |
| `SLUNG_POS_Y` | double | m | Load position Y |
| `SLUNG_POS_Z` | double | m | Load position Z |
| `SLUNG_NATURAL_FREQ` | double | Hz | Natural frequency |
| `SLUNG_RESONANCE` | bool | - | Resonance warning flag |

### 6.2 Analytic Use of Outputs

**Flight Endurance Analysis**: Monitor `BATTERY_SOC` over time to determine total flight time under specific mission profiles. Compare energy consumption rates across different flight patterns.

**Vulnerability Assessment**: Monitor `VRS_MARGIN` and `VRS_STATE` during descent maneuvers to identify vulnerable flight regimes. Low margins indicate flight conditions where the platform is susceptible to loss of control.

**Payload Impact Analysis**: Compare `PROP_THRUST_MARGIN` and `BATTERY_SOC` depletion rates with and without payload to quantify the operational cost of payload carriage.

**Slung Load Stability**: Monitor `SLUNG_SWING_MAG` and `SLUNG_CABLE_TENSION` during maneuvers to identify flight profiles that induce dangerous load oscillations.

---

## 7. Sources, Assumptions, Limitations, and Gaps

### 7.1 Sources

All model parameters are derived from the following open-source references:

| # | Reference | Content Used |
|---|-----------|-------------|
| [1] | DoDI 5000.61, Sep 2024 | VV&A policy and documentation requirements |
| [2] | MIL-STD-3022, Jan 2008 | VV&A documentation templates |
| [3] | Army Directive 2024-02 | Software development policy |
| [4] | AAIB South Scarle Accident Report, 2023 | T-150 specifications: MTOW, thrust, configuration |
| [5] | US Patent US20240262499A1, 2024 | Battery topology: 13S, capacity, voltage |
| [6] | T-Motor U15II KV100 Specifications | Motor parameters: KV, max power, dimensions |
| [7] | AFSIM 2.2505 Documentation | Plugin API, mover/processor architecture |
| [8] | Cheeseman & Bennett, ARC R&M 3021, 1955 | Ground effect model |
| [9] | Johnson, Helicopter Theory, 1980 | Momentum theory, rotor aerodynamics |
| [10] | Leishman, Principles of Helicopter Aerodynamics, 2006 | VRS theory and boundaries |
| [11] | SURVICE TRV-150 Product Data Sheet, 2024 | Manufacturer specifications |

### 7.2 Overall Assumptions

1. The T-150 operates in standard atmosphere (ISA) at sea level unless altitude effects are specified
2. All 8 motors are identical and operate symmetrically
3. The airframe is rigid (no structural deformation under load)
4. No wind, turbulence, or atmospheric disturbances are modeled
5. Sensor and navigation systems provide perfect state knowledge
6. Battery cells are uniform and exhibit identical characteristics
7. Payload is rigidly attached (except when using slung load processor)

### 7.3 Overall Limitations

1. **Flight Endurance Gap**: Simulated endurance (14.3 min) is approximately 40% of manufacturer specification (36 min). This is attributed to conservative power modeling and high auxiliary power estimates. Analysts should apply appropriate scaling when using endurance results.

2. **No Environmental Effects**: Wind, temperature, humidity, and altitude effects on air density are not modeled. Results represent calm, sea-level conditions.

3. **No Sensor Noise**: All state measurements are perfect. Real-world navigation, attitude estimation, and control will introduce errors not captured in simulation.

4. **Informational VRS**: VRS detection is informational; it does not feed back to modify flight dynamics. In reality, VRS causes significant and potentially unrecoverable loss of lift.

5. **Informational Slung Load Forces**: Slung load reaction forces are computed but do not modify aircraft motion. In reality, a swinging load significantly affects aircraft stability.

### 7.4 Overall Gaps

1. **No flight test data**: No actual T-150 flight test data is available for model validation. All validation is against manufacturer specifications, first-principles physics, and analytical predictions.

2. **Inertia values**: Moments of inertia are estimated from platform geometry and mass distribution, not measured.

3. **Battery chemistry specifics**: OCV curve and internal resistance characteristics use generic Li-Ion parameters, not the specific cell chemistry used in the T-150.

4. **Motor-specific data**: Thermal mass, cooling rate, and efficiency curves are based on the T-Motor U15II class rather than specific test data.

---

## 8. References

[1] Office of the Under Secretary of Defense for Research and Engineering, "DoD Instruction 5000.61: DoD Modeling and Simulation Verification, Validation, and Accreditation," September 17, 2024.

[2] Department of Defense, "MIL-STD-3022: Documentation of Verification, Validation, and Accreditation (VV&A) for Models and Simulations," January 28, 2008.

[3] Department of the Army, "Army Directive 2024-02: Software Development," 2024.

[4] Air Accidents Investigation Branch, "AAIB Investigation to Malloy Aeronautics T-150, South Scarle, Nottinghamshire," 2023.

[5] Malloy Aeronautics, "Battery Pack Topology for Unmanned Aerial Vehicle," US Patent US20240262499A1, 2024.

[6] T-Motor, "U15II KV100 Motor Specifications," 2024.

[7] Air Force Research Laboratory, "AFSIM 2.2505 Documentation: Advanced Framework for Simulation, Integration, and Modeling," 2025.

[8] I.C. Cheeseman and W.E. Bennett, "The Effect of Ground on a Helicopter Rotor in Forward Flight," ARC R&M 3021, 1955.

[9] W. Johnson, "Helicopter Theory," Princeton University Press, 1980.

[10] J.G. Leishman, "Principles of Helicopter Aerodynamics," 2nd ed., Cambridge University Press, 2006.

[11] SURVICE Engineering / Malloy Aeronautics, "TRV-150 Heavy-Lift UAS Product Data Sheet," 2024.

---

## 9. Acronyms

| Acronym | Definition |
|---------|------------|
| ADD | Architecture Description Document |
| AFSIM | Advanced Framework for Simulation, Integration, and Modeling |
| AGL | Above Ground Level |
| API | Application Programming Interface |
| AuxData | Auxiliary Data (AFSIM inter-processor communication) |
| BEMT | Blade Element Momentum Theory |
| C-UAS | Counter-Unmanned Aerial System |
| CG | Center of Gravity |
| CONOPS | Concept of Operations |
| DLL | Dynamic Link Library |
| DoD | Department of Defense |
| DoDI | DoD Instruction |
| 6DOF | Six Degrees of Freedom |
| ESC | Electronic Speed Controller |
| GE | Ground Effect |
| ICD | Interface Control Document |
| IGE | In Ground Effect |
| ISA | International Standard Atmosphere |
| KPI | Key Performance Indicator |
| KV | Motor velocity constant (RPM per Volt) |
| M&S | Modeling and Simulation |
| MTOW | Maximum Takeoff Weight |
| NED | North-East-Down (coordinate frame) |
| NGIC | National Ground Intelligence Center |
| OCV | Open-Circuit Voltage |
| OGE | Out of Ground Effect |
| PD | Proportional-Derivative (controller) |
| PID | Proportional-Integral-Derivative (controller) |
| RPM | Revolutions Per Minute |
| SOC | State of Charge |
| TDD | Test Design Document |
| UAM | User and Analyst Manual |
| UAS | Unmanned Aerial System |
| UML | Unified Modeling Language |
| V&V | Verification and Validation |
| VRS | Vortex Ring State |
| VV&A | Verification, Validation, and Accreditation |

[CLASSIFICATION: UNCLASSIFIED]
