[CLASSIFICATION: UNCLASSIFIED]

# Verification and Validation Plan

## T-150/TRV-150 Heavy-Lift UAS AFSIM Simulation Model v2.0

### AFSIM Project 005

| Field | Value |
|-------|-------|
| **Document Version** | 1.0 |
| **Date** | [PLACEHOLDER - Date] |
| **Classification** | UNCLASSIFIED |
| **Sponsoring Organization** | [PLACEHOLDER - Organization Name] |
| **Program Manager** | [PLACEHOLDER - Name/Title] |
| **Document Preparer** | [PLACEHOLDER - Organization/Contract] |
| **Distribution Statement** | [PLACEHOLDER - Distribution Statement] |
| **M&S Name and Version** | T-150 UAS AFSIM Plugin Suite v2.0 |
| **V&V Agent** | [PLACEHOLDER - V&V Agent Organization] |
| **Accreditation Authority** | [PLACEHOLDER - Accreditation Authority] |

---

## Record of Changes

| Version | Date | Author | Description of Change |
|---------|------|--------|-----------------------|
| 1.0 | [PLACEHOLDER] | [PLACEHOLDER] | Initial V&V Plan release per MIL-STD-3022 Appendix B. Covers all five plugins and four test families. |

---

## Table of Contents

- [Executive Summary](#executive-summary)
- [1. Problem Statement](#1-problem-statement)
- [2. M&S Requirements and Acceptability Criteria](#2-ms-requirements-and-acceptability-criteria)
- [3. M&S Assumptions, Capabilities, Limitations, and Risks/Impacts](#3-ms-assumptions-capabilities-limitations-and-risksimpacts)
- [4. V&V Methodology](#4-vv-methodology)
- [5. V&V Issues](#5-vv-issues)
- [6. Key Participants](#6-key-participants)
- [7. Planned V&V Resources](#7-planned-vv-resources)
- [Appendix A. M&S Description](#appendix-a-ms-description)
- [Appendix B. M&S Requirements Traceability Matrix](#appendix-b-ms-requirements-traceability-matrix)
- [Appendix C. Basis of Comparison](#appendix-c-basis-of-comparison)
- [Appendix D. References](#appendix-d-references)
- [Appendix E. Acronyms](#appendix-e-acronyms)

---

## Executive Summary

This Verification and Validation (V&V) Plan establishes the framework, requirements, methodology, and acceptance criteria for the formal V&V of the T-150/TRV-150 Heavy-Lift UAS AFSIM Simulation Model, version 2.0. The plan is prepared in accordance with MIL-STD-3022, "Documentation of Verification, Validation, and Accreditation (VV&A) for Models and Simulations" [2], and supports the accreditation process described in DoDI 5000.61, "DoD Modeling and Simulation Verification, Validation, and Accreditation" [1].

The T-150 UAS AFSIM Plugin Suite v2.0 is a high-fidelity simulation of the T-150/TRV-150 heavy-lift coaxial octocopter, implemented as five custom C++ plugins within the AFSIM 2.2505 framework. The model encompasses 7,736 lines of code distributed across 26 source files, organized into five functional plugins: a physics-based 6DOF mover, a Li-Ion battery model with voltage sag, an electric propulsion system with thermal dynamics, a Vortex Ring State detection and degradation model, and a cable-suspended slung load dynamics model.

The intended use of this M&S is to support intelligence analysis of T-150 UAS capabilities, mission planning assessments, Counter-UAS (C-UAS) evaluation, and vulnerability analysis. The model is developed for the intelligence community to answer questions regarding heavy-lift UAS operational performance, flight endurance, payload capacity, and vulnerability conditions.

This V&V Plan defines thirteen (13) quantitative requirements with measurable acceptability criteria spanning all five plugin subsystems. Requirements address battery energy conservation, voltage sag fidelity, motor thermal stability, 6DOF position hold and velocity tracking, numerical robustness, VRS state transition accuracy, ground effect modeling, flight endurance, slung load pendulum dynamics, attitude recovery, cruise speed, and maximum payload capacity. Each requirement is traceable to a specific validation test or set of tests.

The V&V approach employs a combination of verification techniques (code review, unit testing, integration testing, numerical stability testing) and validation techniques (comparison to manufacturer specifications, comparison to analytical first-principles predictions, energy conservation analysis, and pattern efficiency analysis). Testing is organized into four test families: TF01 (6DOF Physics), TF02 (Battery/Power System), TF03 (VRS Detection), and TF04 (Slung Load Dynamics), comprising a total of sixteen (16) individual tests.

A key challenge for this V&V effort is the absence of T-150 flight test data. All validation comparisons are made against manufacturer specifications from open-source product data, analytical predictions derived from first-principles physics (momentum theory, Li-Ion electrochemistry, pendulum dynamics), and published aerodynamic literature. A known gap exists between simulated flight endurance (14.3 minutes) and the manufacturer-stated specification (36 minutes), which has been documented and bounded but not fully resolved.

This plan identifies the V&V tasks, test configurations, acceptance thresholds, participating organizations, and resource requirements necessary to support a formal accreditation recommendation for the T-150 UAS AFSIM Plugin Suite v2.0.

---

## 1. Problem Statement

### 1.1 Intended Use

The T-150 UAS AFSIM Plugin Suite v2.0 is intended for the following analytical applications:

1. **Intelligence Analysis**: Assessing the operational capabilities, performance envelope, and limitations of the T-150/TRV-150 heavy-lift UAS to support intelligence community questions regarding UAS threats. This includes evaluating flight dynamics, payload capacity, energy consumption profiles, and degraded-mode behavior under various operational conditions.

2. **Mission Planning**: Evaluating flight endurance, range, energy consumption, and payload capacity under various mission profiles. The simulation enables analysts to construct mission timelines, estimate fuel (battery) consumption for specific flight profiles, and identify operational constraints for route planning.

3. **Counter-UAS (C-UAS) Assessment**: Providing high-fidelity UAS flight dynamics to support evaluation of detection, tracking, and engagement scenarios against heavy-lift UAS threats. The 6DOF mover produces realistic flight trajectories, acceleration profiles, and maneuvering behavior for use in integrated C-UAS simulations.

4. **Vulnerability Analysis**: Analyzing system degradation behavior including Vortex Ring State onset and development, battery depletion effects on voltage and available power, motor thermal limits, slung load instability, and payload-induced performance degradation. This supports identification of vulnerable flight regimes and engagement windows.

### 1.2 M&S Overview

The T-150 UAS AFSIM Plugin Suite v2.0 consists of five custom C++ plugins developed for the AFSIM 2.2505 framework:

| Plugin | AFSIM Type Name | Purpose | Source Files | LOC (approx.) |
|--------|-----------------|---------|-------------|---------------|
| `rotorcraft_6dof` | `Rotorcraft6DOF_Mover` | Physics-based 6DOF flight dynamics with rigid body, rotor model, and flight controller | 8 | ~3,200 |
| `t150_battery` | `T150_BATTERY_PROCESSOR` | Li-Ion battery with SOC tracking, OCV polynomial, voltage sag, internal resistance | 4 | ~1,100 |
| `t150_propulsion` | `T150_PROPULSION_PROCESSOR` | Motor power, thermal dynamics, RPM, ground effect, payload, dynamic power | 4 | ~1,500 |
| `t150_vrs` | `T150_VRS_PROCESSOR` | VRS detection via state machine, margin calculation, lift/control degradation | 4 | ~900 |
| `t150_slung_load` | `T150_SLUNG_LOAD_PROCESSOR` | Spherical pendulum slung load, cable tension, resonance detection | 4 | ~1,000 |
| **Total** | | | **26** | **~7,736** |

The plugins communicate through AFSIM's AuxData key-value store, enabling inter-processor data flow (e.g., propulsion power feeds into battery discharge calculations). The 6DOF mover publishes total power consumption, which the battery processor reads for SOC depletion. The VRS processor reads velocity states from the mover to compute safety margins. The slung load processor reads platform acceleration to drive pendulum dynamics.

**Framework**: AFSIM 2.2505 (Advanced Framework for Simulation, Integration, and Modeling) [7]

### 1.3 M&S Application

The T-150 UAS AFSIM Plugin Suite supports intelligence assessments conducted by [PLACEHOLDER - Organization] regarding the operational capabilities of heavy-lift UAS platforms. Specific analytical products supported include:

- Operational performance assessments for the T-150/TRV-150 platform class
- Flight envelope characterization (speed, altitude, endurance, payload trade-offs)
- Energy consumption analysis by mission profile and flight pattern
- Vulnerability assessments identifying degraded-mode flight conditions
- C-UAS scenario development requiring realistic heavy-lift UAS behavior

### 1.4 Accreditation Scope

This V&V Plan supports full model accreditation of the T-150 UAS AFSIM Plugin Suite v2.0 for intelligence analysis use as described in Section 1.1. The accreditation determination will be made by the designated Accreditation Authority based on the V&V results documented in the V&V Report, in accordance with DoDI 5000.61 [1].

The accreditation scope encompasses all five plugins and their integrated behavior within the AFSIM framework. The accreditation applies to the specific version (v2.0) of the plugin suite operating within AFSIM 2.2505.

### 1.5 V&V Scope

The V&V effort encompasses:

- **Verification**: Confirmation that the model correctly implements the intended physics, algorithms, and data flows as specified in the Architecture Description Document (ADD) and User and Analyst Manual (UAM). This includes code review of all 26 source files, unit-level testing of mathematical functions, integration testing of inter-plugin communication, and numerical stability testing under extreme inputs.

- **Validation**: Determination that the model produces results consistent with the intended use by comparison against three categories of reference data:
  1. Manufacturer specifications (cruise speed, maximum payload, battery capacity, flight time)
  2. Analytical predictions from first-principles physics (hover power from momentum theory, pendulum natural frequency, VRS onset boundaries)
  3. Published literature on rotorcraft aerodynamics, Li-Ion battery electrochemistry, and ground effect

---

## 2. M&S Requirements and Acceptability Criteria

The following table defines the quantitative requirements for the T-150 UAS AFSIM Plugin Suite v2.0. Each requirement includes a measurable acceptability criterion, the validation method, and the associated test family.

| Req ID | Requirement Description | Acceptability Criterion | Validation Method | Test Family | Priority |
|--------|------------------------|------------------------|-------------------|-------------|----------|
| R-001 | Battery State of Charge (SOC) shall track energy consumption with high accuracy | Energy conservation error < 0.1% over full discharge cycle | Compare total energy consumed to battery capacity; verify energy balance closure | TF02 | Critical |
| R-002 | Battery voltage sag under load shall match Li-Ion discharge characteristics | Terminal voltage follows OCV minus IR drop; voltage sag magnitude consistent with Li-Ion cell behavior at given SOC and current | Compare OCV curve to published Li-Ion discharge profiles; verify quadratic current solution stability | TF02 | High |
| R-003 | Motor thermal model shall reach stable equilibrium temperature during sustained operation | Motor temperature converges to steady-state within 300 seconds; no thermal runaway or unbounded growth | Execute sustained hover and verify temperature stabilization; check equilibrium against Q_in = Q_out analytical prediction | TF02 | High |
| R-004 | 6DOF mover shall maintain position hold stability during hover | Position drift < 1 m from commanded hover position during steady-state hover (after initial transients) | Command hover at fixed position; measure lateral/vertical drift after 10 seconds of stabilization | TF01 | Critical |
| R-005 | 6DOF mover shall track commanded velocity with acceptable accuracy | Steady-state velocity error < 1 m/s during cruise flight at commanded airspeed | Command cruise at 30 m/s; measure velocity error after transient decay | TF01 | High |
| R-006 | All plugins shall maintain numerical stability under extreme inputs | No NaN values, no numerical divergence, and no simulation crashes under boundary and stress conditions | Execute scenarios with zero payload, maximum payload, extreme descent rates, zero-SOC battery, and near-zero altitude | TF01, TF02, TF03, TF04 | Critical |
| R-007 | VRS detection shall trigger correct state transitions at known aerodynamic thresholds | VRS CAUTION triggers at margin < 0.8; VRS ONSET triggers at margin < 0.5; transitions occur at correct descent rate and forward speed combinations | Execute controlled descent profiles crossing VRS thresholds; verify state machine transitions against predicted margins | TF03 | High |
| R-008 | Ground effect model shall produce thrust augmentation below one rotor diameter altitude | Ground effect factor > 1.0 below 0.76 m AGL; factor approaches 1.0 above 1.5 rotor diameters; maximum factor approximately 1.3 at ground level | Execute low-altitude hover at varying heights; compare thrust augmentation to Cheeseman-Bennett analytical predictions | TF01 | Medium |
| R-009 | Simulated flight endurance shall fall within a calibrated range of the manufacturer specification | Flight endurance is documented with known gap (14.3 min simulated vs. 36 min specification); gap is bounded and attributable to identified modeling assumptions | Execute full discharge flight; document endurance and compare to manufacturer specification; identify contributing factors | TF02 | High |
| R-010 | Slung load pendulum dynamics shall produce oscillation frequency matching analytical prediction | Measured pendulum frequency within 5% of analytical natural frequency f_n = (1/2pi) * sqrt(g/L) for a given cable length | Excite slung load via acceleration impulse; measure oscillation period; compare to analytical prediction for 5 m cable (predicted f_n = 0.223 Hz) | TF04 | High |
| R-011 | 6DOF mover shall recover to stable hover following an attitude disturbance | Platform returns to within 5 degrees of level attitude and 2 m of commanded position within 10 seconds of disturbance removal | Apply transient disturbance (e.g., asymmetric load); verify recovery to stable hover | TF01 | Medium |
| R-012 | Simulated cruise speed shall match the manufacturer specification | Steady-state cruise speed within 1 m/s of the 30 m/s manufacturer specification | Command cruise at maximum speed; measure achieved velocity at steady state | TF01 | High |
| R-013 | Maximum payload capacity shall be correctly modeled | Model correctly represents 68 kg maximum payload; overload condition detected and flagged when exceeded | Configure payload at 68 kg and verify stable operation; configure payload above 68 kg and verify overload flag | TF02 | High |

---

## 3. M&S Assumptions, Capabilities, Limitations, and Risks/Impacts

### 3.1 Assumptions

The following assumptions are embedded in the T-150 UAS simulation model. These assumptions bound the conditions under which the model produces valid results.

| ID | Assumption | Impact on Results |
|----|-----------|-------------------|
| A-001 | **Rigid body dynamics**: The airframe is modeled as a rigid body with no structural flexibility, vibration modes, or aeroelastic effects. | Eliminates high-frequency structural dynamics; acceptable for mission-level trajectory analysis. |
| A-002 | **Standard atmosphere**: Air density is modeled at standard sea-level conditions (rho = 1.225 kg/m^3) and does not vary with altitude, temperature, or humidity within a simulation run. | Overestimates performance at high altitude; underestimates performance at high temperature. Acceptable for low-altitude intelligence scenarios. |
| A-003 | **Symmetric inertia tensor**: The moments of inertia tensor is assumed diagonal (products of inertia are zero), implying the platform has geometric symmetry about its principal axes. | Eliminates cross-coupling terms in rotational dynamics. Valid for symmetric octocopter configuration. |
| A-004 | **No wind model**: No atmospheric wind, gusts, or turbulence are modeled. The platform operates in still air. | Optimistic endurance and stability results. Wind effects would increase power consumption and reduce position hold accuracy. |
| A-005 | **Perfect sensors**: All sensor measurements (position, velocity, attitude) available to the flight controller are assumed noise-free and lag-free. | Overestimates control precision; real systems have sensor noise and latency. |
| A-006 | **Uniform battery cells**: All cells in the 13S battery pack are identical in capacity, resistance, and aging characteristics. No cell imbalance is modeled. | Eliminates cell-level failure modes; realistic packs exhibit cell variation, particularly with age. |
| A-007 | **Flat terrain**: The simulation assumes flat terrain at the platform's initial altitude datum. No terrain elevation model is used. | Valid for open-area operations; not suitable for complex terrain analysis. |
| A-008 | **Estimated inertia values**: Moments of inertia are estimated from platform geometry and mass distribution rather than measured from the actual aircraft. | Affects transient dynamic response (oscillation frequency, damping); steady-state behavior less sensitive. |

### 3.2 Capabilities

The T-150 UAS AFSIM Plugin Suite v2.0 provides the following modeling capabilities:

1. **Full 6DOF flight dynamics**: Newton-Euler rigid body physics with quaternion attitude representation, eight individually modeled rotors with thrust and torque, PID-based flight controller for altitude, position, and attitude control, and waypoint navigation with spline interpolation.

2. **Battery voltage sag modeling**: SOC-dependent open-circuit voltage via polynomial curve, SOC-dependent internal resistance with U-shaped characteristic, quadratic current solution for stable voltage sag calculation under load, and energy conservation tracking.

3. **VRS detection and modeling**: Five-state machine (CLEAR, CAUTION, ONSET, DEVELOPED, RECOVERY) driven by descent rate and forward speed margins, lift degradation factors (down to 0.4x in fully developed VRS), and control authority degradation factors.

4. **Slung load dynamics**: Two-DOF spherical pendulum model with gravity and aircraft acceleration excitation, cable tension calculation with slack and overload detection, resonance detection based on frequency matching, and symplectic (semi-implicit Euler) integration for energy preservation.

5. **Dynamic power modeling**: Flight-state-dependent power calculation incorporating induced power, climb power, parasite drag power, and acceleration power. Bank angle estimation from horizontal acceleration for thrust loading during turns.

6. **Ground effect**: Cheeseman-Bennett approximation for thrust augmentation near ground, active below one rotor diameter (0.76 m) AGL with up to 30% thrust increase at ground level.

7. **Motor thermal dynamics**: First-order thermal model with heat generation from electrical losses, forced convection cooling, thermal mass, and over-temperature warning.

### 3.3 Limitations

| ID | Limitation | Impact | Mitigation |
|----|-----------|--------|------------|
| L-001 | No wind model | Cannot assess wind effects on endurance, stability, or trajectory | Document as out-of-scope; recommend future wind integration |
| L-002 | VRS is informational only | VRS lift and control degradation factors are computed and published to AuxData but do not feed back to modify the 6DOF mover's flight dynamics | Analysts must interpret VRS outputs as advisory; mover trajectory does not reflect VRS-induced performance loss |
| L-003 | Slung load forces do not feed back to mover | Pendulum reaction forces (F_x, F_y, F_z) are calculated and published but do not alter the 6DOF mover's trajectory | Load-induced trim changes and stability effects are not captured in flight trajectory |
| L-004 | Flight endurance gap | Simulated endurance (14.3 min) vs. manufacturer specification (36 min) | Gap is documented and bounded; direct time-based analysis requires calibration factor application |
| L-005 | Constant air density | Performance does not degrade with altitude within a run | Conservative for high-altitude analysis; acceptable for low-altitude scenarios |
| L-006 | No structural failure modes | No modeling of motor failure, ESC failure, arm fracture, or battery cell failure | Single-point failures not assessable; all motors assumed operational |

### 3.4 Risks/Impacts

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| **Endurance gap** limits direct time-based analysis | High | Analysts may draw incorrect conclusions about operational range if simulated endurance is used without calibration | Document calibration factor (approximately 2.5x) in UAM and all analytical products; recommend range analysis use speed-based metrics |
| **No flight test data** available for direct validation | High | Cannot perform traditional model-vs-test correlation; validation relies entirely on specification comparison and first-principles physics | Use multiple independent validation approaches; clearly state validation basis in all products; classify confidence levels per capability |
| **Estimated parameters** (inertia, thermal mass, cooling rate) may not match actual T-150 | Medium | Transient dynamics (oscillation frequency, damping, thermal time constants) may differ from reality | Sensitivity analysis on key estimated parameters; bound uncertainty ranges |
| **Self-testing** (developer performs V&V) reduces independence | Medium | Potential for confirmation bias in test design and result interpretation | Document methodology transparently; use objective quantitative criteria; recommend independent review |

---

## 4. V&V Methodology

### 4.1 V&V Approach

The V&V approach for the T-150 UAS AFSIM Plugin Suite v2.0 combines verification and validation activities to establish confidence that the model is correctly implemented (verification) and produces results appropriate for its intended use (validation).

**Verification** establishes that the code correctly implements the intended algorithms and physics. Verification activities include:

- Peer code review of algorithms against design documentation
- Unit-level numerical checks of mathematical functions
- Integration testing of inter-plugin data flow via AuxData
- Boundary and stress testing for numerical robustness

**Validation** establishes that the model's outputs are consistent with real-world behavior within acceptable tolerances. Given the absence of flight test data, validation employs three complementary approaches:

1. **Comparison to manufacturer specifications**: Cruise speed, maximum payload, battery capacity, and stated flight time from open-source product data [11]
2. **Comparison to analytical predictions**: Hover power from momentum theory, pendulum natural frequency from classical mechanics, VRS onset boundaries from established aerodynamic theory [9, 10]
3. **Energy conservation and internal consistency**: Verifying that energy in equals energy out, that state transitions occur at predicted thresholds, and that subsystem outputs are mutually consistent

### 4.2 Verification Tasks

#### 4.2.1 Code Review

All 26 source files across the five plugins shall be reviewed for:

- Correctness of physics equations against the Physics Reference Guide and UAM documentation
- Proper unit conversions (degrees/radians, RPM/rev-per-second, watts/kilowatts)
- Numerical guard conditions (division by zero, square root of negative, NaN propagation)
- Proper initialization of state variables
- Correct use of AFSIM API (AuxData reads/writes, mover state access, time step handling)

| Plugin | Source Files | Key Algorithms to Review |
|--------|-------------|-------------------------|
| rotorcraft_6dof | RigidBody6DOF.cpp/.hpp, RotorModel.cpp/.hpp, Rotorcraft6DOF_Mover.cpp/.hpp, MathUtils.hpp, Registration files | Quaternion integration, Newton-Euler equations, rotor thrust/torque, PID controllers, waypoint navigation |
| t150_battery | BatteryProcessor.cpp/.hpp, Registration files | OCV polynomial, quadratic current solver, SOC integration, internal resistance variation |
| t150_propulsion | PropulsionProcessor.cpp/.hpp, Registration files | Momentum theory thrust, efficiency curves, thermal dynamics, ground effect, dynamic power model |
| t150_vrs | VRSProcessor.cpp/.hpp, Registration files | VRS margin calculation, state machine transitions, lift/control degradation factors |
| t150_slung_load | SlungLoadProcessor.cpp/.hpp, Registration files | Pendulum equations of motion, symplectic integration, cable tension, resonance detection |

#### 4.2.2 Unit-Level Testing

Unit-level tests verify individual mathematical functions and algorithms in isolation:

- OCV polynomial evaluation at known SOC values
- Quadratic current solver with known power and resistance inputs
- Thrust coefficient calculation at known RPM and air density
- Pendulum angular acceleration at known angle and cable length
- VRS margin calculation at known descent rate and forward speed combinations
- Quaternion normalization and rotation operations

#### 4.2.3 Integration Testing

Integration tests verify inter-plugin communication and data consistency:

- Propulsion processor publishes `PROP_TOTAL_POWER`; battery processor reads and correctly computes SOC depletion
- 6DOF mover publishes velocity states; VRS processor reads and correctly computes margin
- 6DOF mover publishes acceleration; slung load processor reads and correctly excites pendulum dynamics
- Battery SOC depletion rate is consistent with propulsion power output

#### 4.2.4 Numerical Stability Testing

Stress tests verify robustness under boundary and extreme conditions:

- Zero payload mass (empty aircraft)
- Maximum payload mass (68 kg) and overload (>68 kg)
- Battery SOC at 0% (fully depleted)
- Battery SOC at 100% (fully charged)
- Extreme descent rate (>8 m/s, well beyond VRS threshold)
- Near-zero altitude (ground effect boundary)
- Very small time steps (dt < 0.001 s) and large time steps (dt > 0.1 s)

### 4.3 Validation Tasks

#### 4.3.1 Manufacturer Specification Comparison

The following manufacturer-stated values from the TRV-150 product data sheet [11] are used as primary validation references:

| Parameter | Manufacturer Value | Acceptance Criterion |
|-----------|--------------------|---------------------|
| Cruise speed | 30 m/s | Within 1 m/s |
| Maximum payload | 68 kg (150 lbs) | Exact match (model parameter) |
| Battery capacity | ~7 kWh | Exact match (model parameter) |
| Battery configuration | 13S Li-Ion | Exact match (model parameter) |
| Flight time | 36 minutes | Document gap; provide calibration |
| MTOW | 123 kg | Within 5 kg |

#### 4.3.2 Analytical Prediction Comparison

First-principles physics calculations provide independent validation references:

| Prediction | Analytical Formula | Expected Value | Acceptance Criterion |
|------------|-------------------|----------------|---------------------|
| Hover power | P = T * v_i, where v_i = sqrt(T / (2 * rho * A)) | ~17-20 kW for 123 kg MTOW | Within 15% |
| Pendulum natural frequency | f_n = (1/2pi) * sqrt(g/L) | 0.223 Hz for L = 5 m | Within 5% |
| VRS onset descent rate | v_desc approaching v_i | ~4 m/s at low forward speed | Correct threshold detection |
| Ground effect factor | 1 / (1 - (R/4h)^2) | ~1.3 at h = R/4 | Within 10% |
| Bank angle thrust loading | T = mg / cos(phi) | 1.15x at 30 deg bank | Within 5% |

#### 4.3.3 Energy Conservation Validation

Energy conservation testing verifies that total energy consumed equals the integral of power over time, and that no energy is created or destroyed by the numerical integration:

- Total energy consumed (Wh) = integral of instantaneous power over simulation time
- Battery capacity decrease matches cumulative energy draw
- Acceptance criterion: energy balance error < 0.1%

#### 4.3.4 Pattern Efficiency Analysis

Flight pattern efficiency analysis validates that the relative energy cost of different maneuver patterns follows physically expected trends:

- Straight-line cruise is most efficient (baseline)
- Maneuvering patterns (sawtooth, sine wave, square wave, cycloid) consume progressively more energy
- Energy cost ordering is consistent with maneuver intensity (acceleration magnitude and frequency)

### 4.4 Test Families

Testing is organized into four test families. Each test family targets a specific plugin subsystem or cross-cutting concern.

#### TF01: 6DOF Physics (5 Tests)

| Test ID | Test Name | Requirement | Description |
|---------|-----------|-------------|-------------|
| TF01-01 | Hover Stability | R-004 | Command hover at fixed position for 60 seconds. Measure position drift after initial 10-second transient. Verify drift < 1 m. |
| TF01-02 | Cruise Velocity Tracking | R-005, R-012 | Command cruise at 30 m/s. Measure achieved speed at steady state. Verify error < 1 m/s and speed matches manufacturer specification. |
| TF01-03 | Attitude Recovery | R-011 | Apply asymmetric loading or disturbance. Verify return to level hover within 10 seconds and 5 degrees of level attitude. |
| TF01-04 | Ground Effect Validation | R-008 | Execute hover at 0.2 m, 0.5 m, 1.0 m, and 2.0 m AGL. Record ground effect factor. Compare to Cheeseman-Bennett prediction. |
| TF01-05 | Numerical Robustness (6DOF) | R-006 | Execute extreme maneuver sequences (rapid climb/descent, maximum bank, rapid payload changes). Verify no NaN values or simulation divergence. |

#### TF02: Battery/Power System (5 Tests)

| Test ID | Test Name | Requirement | Description |
|---------|-----------|-------------|-------------|
| TF02-01 | Energy Conservation | R-001 | Execute full battery discharge from 100% to 0% SOC. Verify total energy consumed matches battery capacity within 0.1%. |
| TF02-02 | Voltage Sag Characteristics | R-002 | Execute hover at multiple SOC levels (100%, 80%, 50%, 20%, 5%). Record OCV, terminal voltage, and current. Verify voltage sag follows V = OCV - IR relationship. |
| TF02-03 | Motor Thermal Equilibrium | R-003 | Execute sustained hover for 300 seconds. Verify motor temperature converges to stable equilibrium. Check equilibrium temperature against analytical Q_in = Q_out prediction. |
| TF02-04 | Flight Endurance | R-009 | Execute hover flight until battery depletion. Record endurance time. Document comparison to 36-minute manufacturer specification. Identify contributing factors to gap. |
| TF02-05 | Payload Capacity | R-013 | Configure payload at 0 kg, 34 kg, 68 kg, and 75 kg. Verify stable operation at and below 68 kg. Verify overload flag at 75 kg. Verify thrust margin decreases with payload. |

#### TF03: VRS Detection (3 Tests)

| Test ID | Test Name | Requirement | Description |
|---------|-----------|-------------|-------------|
| TF03-01 | VRS State Transitions | R-007 | Execute controlled descent at increasing rates (1, 2, 3, 4, 5, 6 m/s) with zero forward speed. Record VRS margin and state at each rate. Verify CAUTION at margin < 0.8, ONSET at margin < 0.5. |
| TF03-02 | Forward Speed Immunity | R-007 | Execute descent at 5 m/s with forward speeds of 0, 2, 5, and 10 m/s. Verify VRS is suppressed at forward speeds above 5 m/s threshold. |
| TF03-03 | Numerical Robustness (VRS) | R-006 | Execute extreme descent rates (10 m/s, 15 m/s) and rapid descent-to-climb transitions. Verify no NaN values, no stuck states, and correct state recovery. |

#### TF04: Slung Load Dynamics (3 Tests)

| Test ID | Test Name | Requirement | Description |
|---------|-----------|-------------|-------------|
| TF04-01 | Pendulum Frequency | R-010 | Configure 5 m cable with 25 kg load. Apply acceleration impulse to excite pendulum. Measure oscillation period. Verify frequency within 5% of analytical prediction (0.223 Hz). |
| TF04-02 | Load State Transitions | R-010 | Verify state transitions: DETACHED to ATTACHED on load configuration, ATTACHED to SWINGING on excitation, SWINGING to RESONANCE on sustained excitation at natural frequency. |
| TF04-03 | Numerical Robustness (Slung Load) | R-006 | Execute with extreme cable lengths (0.5 m, 20 m), extreme load masses (1 kg, 55 kg), and rapid platform accelerations. Verify no NaN values or divergent oscillations. |

---

## 5. V&V Issues

The following issues have been identified that may affect the V&V effort, its conclusions, or the resulting accreditation recommendation.

### 5.1 Absence of Flight Test Data

No T-150 flight test data (telemetry, instrumented flight records, wind tunnel data) is available for direct model-versus-test validation. All validation is performed against manufacturer specifications from open-source product data sheets, analytical predictions from first-principles physics, and published aerodynamic literature. This limits the achievable confidence level for certain capabilities, particularly transient dynamic responses and flight-state-dependent power consumption, where real-world data would provide the most definitive validation reference.

**Impact**: Validation confidence is bounded by the quality of analytical predictions and the specificity of manufacturer specifications. Some requirements may receive "Medium" rather than "High" confidence ratings.

**Mitigation**: Multiple independent validation approaches are used (specification comparison, analytical prediction, energy conservation, internal consistency). Confidence levels are explicitly stated per capability in the UAM.

### 5.2 Flight Endurance Gap

The simulated flight endurance of 14.3 minutes at hover significantly differs from the manufacturer-stated specification of 36 minutes. Contributing factors include differences in hover power assumptions, potential differences between the simulated battery capacity utilization and the manufacturer's test conditions, and the absence of manufacturer-specific motor efficiency data.

**Impact**: Direct use of simulated endurance for absolute time-based analysis would produce non-conservative results (shorter mission times than reality). However, relative comparisons (e.g., pattern A uses 30% more energy than pattern B) remain valid.

**Mitigation**: The endurance gap is documented in the UAM, ADD, and V&V Report. A calibration factor is provided. Analysts are instructed to use relative metrics or apply calibration for absolute time-based analysis.

### 5.3 Estimated Parameters

Several model parameters are estimated from the motor class, platform geometry, or published literature rather than measured from the actual T-150 aircraft:

- Moments of inertia (estimated from geometry and mass distribution)
- Motor thermal mass and cooling rate (estimated from T-Motor U15II class data)
- Battery internal resistance (generic Li-Ion values)
- Drag coefficient and frontal area (back-calculated from cruise power)

**Impact**: Transient dynamics (oscillation frequencies, damping rates, thermal time constants) may differ from actual T-150 behavior. Steady-state values are less sensitive to these parameters.

**Mitigation**: Sensitivity analysis on key estimated parameters is recommended. Parameter values are documented with their sources and estimation methodology in the UAM.

### 5.4 No Independent V&V Agent

The V&V is performed by the model developer (self-testing) rather than by an independent V&V agent. This introduces potential confirmation bias in test design, execution, and result interpretation.

**Impact**: Reduced independence may limit the accreditation authority's confidence in the objectivity of V&V conclusions.

**Mitigation**: All V&V activities use objective, quantitative acceptability criteria defined in advance (Section 2). Test procedures and results are fully documented for independent review. The accreditation authority may direct supplemental independent review if warranted.

---

## 6. Key Participants

| Role | Organization | Individual | Responsibility |
|------|-------------|------------|----------------|
| **V&V Agent** | [PLACEHOLDER] | [PLACEHOLDER] | Plan, execute, and document V&V activities. Produce V&V Plan and V&V Report. Provide accreditation recommendation. |
| **M&S Developer** | [PLACEHOLDER] | [PLACEHOLDER] | Develop and maintain the T-150 UAS AFSIM Plugin Suite. Provide source code, documentation, and technical support for V&V activities. |
| **Accreditation Authority** | [PLACEHOLDER] | [PLACEHOLDER] | Review V&V results and make accreditation determination per DoDI 5000.61. |
| **M&S Proponent** | [PLACEHOLDER] | [PLACEHOLDER] | Define intended use and analytical requirements. Validate that M&S capabilities meet analytical needs. |
| **Subject Matter Expert (Aerodynamics)** | [PLACEHOLDER] | [PLACEHOLDER] | Review 6DOF physics, VRS modeling, and ground effect implementation for technical accuracy. |
| **Subject Matter Expert (Power Systems)** | [PLACEHOLDER] | [PLACEHOLDER] | Review battery, propulsion, and thermal modeling for technical accuracy against state-of-the-art in electric UAS power systems. |
| **Subject Matter Expert (UAS Operations)** | [PLACEHOLDER] | [PLACEHOLDER] | Review operational fidelity of flight profiles, mission parameters, and CONOPS representation. |
| **Configuration Manager** | [PLACEHOLDER] | [PLACEHOLDER] | Maintain version control of M&S software, documentation, and V&V artifacts. |

---

## 7. Planned V&V Resources

### 7.1 Schedule

| Milestone | Planned Date | Description |
|-----------|-------------|-------------|
| V&V Plan Approval | [PLACEHOLDER] | V&V Plan reviewed and approved by Accreditation Authority |
| Code Review Complete | [PLACEHOLDER] | All 26 source files reviewed; findings documented |
| Unit Testing Complete | [PLACEHOLDER] | All unit-level verification tests executed and documented |
| Integration Testing Complete | [PLACEHOLDER] | Inter-plugin communication tests executed and documented |
| Validation Testing Complete | [PLACEHOLDER] | All 16 tests across 4 test families executed and documented |
| V&V Report Draft | [PLACEHOLDER] | Draft V&V Report submitted for review |
| V&V Report Final | [PLACEHOLDER] | Final V&V Report delivered to Accreditation Authority |
| Accreditation Decision | [PLACEHOLDER] | Accreditation Authority renders accreditation determination |

### 7.2 Computing Resources

| Resource | Description |
|----------|-------------|
| AFSIM 2.2505 | Licensed simulation framework (Linux or Windows installation) |
| C++ Compiler | GCC 11+ (Linux) or MSVC 2022 (Windows) with C++17 support |
| CMake 3.16+ | Build system for plugin compilation |
| Python 3.12 | Post-processing scripts for output analysis and visualization |
| Workstation | Standard analyst workstation; no HPC required (single-platform simulations) |

### 7.3 Documentation Deliverables

| Document | Standard Reference | Status |
|----------|-------------------|--------|
| V&V Plan (this document) | MIL-STD-3022, Appendix B | This document |
| V&V Report | MIL-STD-3022, Appendix C | To be produced following test execution |
| User and Analyst Manual (UAM) | MIL-STD-3022, Appendix A | Complete (v3.0) |
| Architecture Description Document (ADD) | MIL-STD-3022 | Complete (v2.0) |
| Interface Control Document (ICD) | MIL-STD-3022 | Complete (v1.0) |
| Accreditation Report | DoDI 5000.61 | To be produced by Accreditation Authority |

### 7.4 Level of Effort Estimate

| Activity | Estimated Effort |
|----------|-----------------|
| Code review (26 files, ~7,736 LOC) | [PLACEHOLDER] |
| Unit test development and execution | [PLACEHOLDER] |
| Integration test development and execution | [PLACEHOLDER] |
| Validation test execution (16 tests) | [PLACEHOLDER] |
| V&V Report preparation | [PLACEHOLDER] |
| SME review and coordination | [PLACEHOLDER] |
| **Total estimated effort** | **[PLACEHOLDER]** |

---

## Appendix A. M&S Description

The T-150 UAS AFSIM Plugin Suite v2.0 models the T-150/TRV-150 heavy-lift coaxial octocopter UAS within the AFSIM 2.2505 simulation framework. The model consists of five C++ plugins that implement physics-based subsystem models for flight dynamics, battery power, electric propulsion, Vortex Ring State aerodynamics, and slung load dynamics.

For a complete description of the model architecture, algorithms, parameters, and data flows, refer to:

- **User and Analyst Manual (UAM) v3.0**: Comprehensive model description, parameter tables, confidence levels, assumptions, and limitations.
- **Architecture Description Document (ADD) v2.0**: Software architecture, class hierarchy, data flow diagrams, and plugin interaction model.
- **Interface Control Document (ICD) v1.0**: AuxData key definitions, inter-plugin data exchange specifications, and AFSIM API usage.

---

## Appendix B. M&S Requirements Traceability Matrix

The following matrix traces each requirement to its source, validation test(s), and associated V&V Report section.

| Req ID | Requirement | Source | Test ID(s) | V&V Report Section |
|--------|------------|--------|------------|-------------------|
| R-001 | Battery SOC accuracy (< 0.1% error) | First-principles energy conservation | TF02-01 | [TBD] |
| R-002 | Voltage sag matches Li-Ion characteristics | Published Li-Ion literature [9] | TF02-02 | [TBD] |
| R-003 | Motor thermal equilibrium | Thermodynamics first principles | TF02-03 | [TBD] |
| R-004 | Position hold stability (< 1 m drift) | Flight dynamics requirements | TF01-01 | [TBD] |
| R-005 | Velocity tracking (< 1 m/s error) | Flight dynamics requirements | TF01-02 | [TBD] |
| R-006 | Numerical stability (no NaN/divergence) | Software quality requirements | TF01-05, TF03-03, TF04-03 | [TBD] |
| R-007 | VRS detection at correct thresholds | Aerodynamic theory [9, 10] | TF03-01, TF03-02 | [TBD] |
| R-008 | Ground effect below 1 rotor diameter | Cheeseman-Bennett [8] | TF01-04 | [TBD] |
| R-009 | Flight endurance within calibrated range | Manufacturer specification [11] | TF02-04 | [TBD] |
| R-010 | Slung load pendulum frequency (< 5% error) | Classical mechanics | TF04-01, TF04-02 | [TBD] |
| R-011 | Attitude recovery after disturbance | Flight dynamics requirements | TF01-03 | [TBD] |
| R-012 | Cruise speed matches 30 m/s specification | Manufacturer specification [11] | TF01-02 | [TBD] |
| R-013 | Maximum payload 68 kg correctly modeled | Manufacturer specification [11] | TF02-05 | [TBD] |

---

## Appendix C. Basis of Comparison

This appendix identifies the reference data sources used for validation comparisons in this V&V effort.

### C.1 Manufacturer Specifications

Primary source: TRV-150 Heavy-Lift UAS Product Data Sheet [11], supplemented by the AAIB accident investigation report [4] and US Patent application [5].

| Parameter | Value | Source | Usage in V&V |
|-----------|-------|--------|-------------|
| Cruise speed | 30 m/s | [11] | R-012 validation |
| Maximum payload | 68 kg | [11] | R-013 validation |
| MTOW | 123 kg | [4] | Thrust/power calculations |
| Battery capacity | ~7 kWh | [5] | R-001, R-009 validation |
| Battery configuration | 13S Li-Ion | [5] | Voltage model validation |
| Flight time | 36 minutes | [11] | R-009 comparison |
| Motors | 8x T-Motor U15II KV100 | [4, 6] | Propulsion model parameters |
| Propeller diameter | 30 inches (0.76 m) | Technical docs | Ground effect, thrust calculations |

### C.2 Analytical Predictions

| Prediction | Formula | Parameters | Expected Result |
|------------|---------|------------|-----------------|
| Hover induced velocity | v_i = sqrt(mg / (2 * rho * A_disk)) | m=123 kg, rho=1.225, A=8*pi*0.38^2 | ~8 m/s |
| Hover power | P = mg * v_i * k | k=1.15 (non-ideal factor) | ~17-20 kW |
| Pendulum frequency | f = (1/2pi) * sqrt(g/L) | g=9.81, L=5 m | 0.223 Hz |
| Ground effect (h=R/2) | GE = 1/(1-(R/4h)^2) | R=0.38, h=0.19 | ~1.33 |
| VRS onset | v_desc approaching v_i at low V_fwd | v_i~8, threshold=4 m/s | 50% of v_i |
| Turn thrust loading | T = mg/cos(phi) | phi=30 deg | 1.155x |

### C.3 Published Literature

| Reference | Application in V&V |
|-----------|--------------------|
| Cheeseman & Bennett (1955) [8] | Ground effect thrust augmentation model validation |
| Johnson (1980) [9] | Momentum theory, VRS boundaries, helicopter performance |
| Leishman (2006) [10] | VRS state characterization, rotor aerodynamics |
| Li-Ion discharge curves (general literature) | OCV polynomial shape, internal resistance SOC dependence |

---

## Appendix D. References

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

## Appendix E. Acronyms

| Acronym | Definition |
|---------|------------|
| ADD | Architecture Description Document |
| AFSIM | Advanced Framework for Simulation, Integration, and Modeling |
| AGL | Above Ground Level |
| AuxData | Auxiliary Data (AFSIM inter-processor communication) |
| C-UAS | Counter-Unmanned Aerial System |
| CG | Center of Gravity |
| CONOPS | Concept of Operations |
| DLL | Dynamic Link Library |
| DoD | Department of Defense |
| DoDI | DoD Instruction |
| ESC | Electronic Speed Controller |
| GE | Ground Effect |
| ICD | Interface Control Document |
| IGE | In Ground Effect |
| ISA | International Standard Atmosphere |
| KV | Motor velocity constant (RPM per Volt) |
| LOC | Lines of Code |
| M&S | Modeling and Simulation |
| MTOW | Maximum Takeoff Weight |
| NED | North-East-Down (coordinate frame) |
| OCV | Open-Circuit Voltage |
| OGE | Out of Ground Effect |
| PD | Proportional-Derivative (controller) |
| PID | Proportional-Integral-Derivative (controller) |
| RPM | Revolutions Per Minute |
| SME | Subject Matter Expert |
| SOC | State of Charge |
| TDD | Test Design Document |
| UAM | User and Analyst Manual |
| UAS | Unmanned Aerial System |
| V&V | Verification and Validation |
| VRS | Vortex Ring State |
| VV&A | Verification, Validation, and Accreditation |
| 6DOF | Six Degrees of Freedom |

---

[CLASSIFICATION: UNCLASSIFIED]
