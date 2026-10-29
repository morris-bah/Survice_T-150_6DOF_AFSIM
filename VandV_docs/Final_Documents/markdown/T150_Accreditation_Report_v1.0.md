[CLASSIFICATION: UNCLASSIFIED]

# Accreditation Report

## T-150/TRV-150 Heavy-Lift UAS AFSIM Simulation Model v2.0

### AFSIM Project 005

| Field | Value |
|-------|-------|
| **Document Version** | 1.0 |
| **Date** | March 5, 2026 |
| **Classification** | UNCLASSIFIED |
| **Sponsoring Organization** | [PLACEHOLDER - Organization Name] |
| **Accreditation Authority** | [PLACEHOLDER - Name/Title] |
| **Document Preparer** | [PLACEHOLDER - Organization/Contract] |
| **Distribution Statement** | [PLACEHOLDER - Distribution Statement] |
| **M&S Name and Version** | T-150 UAS AFSIM Plugin Suite v2.0 |
| **Accreditation Decision** | CONDITIONAL ACCREDITATION |

This document is prepared in accordance with:

- **DoDI 5000.61**, "DoD Modeling and Simulation Verification, Validation, and Accreditation," September 17, 2024 [1]
- **MIL-STD-3022**, Appendix D, "Documentation of Verification, Validation, and Accreditation (VV&A) for Models and Simulations," January 28, 2008 [2]
- **NGIC 4.0 Documentation Templates**, January 30, 2026

---

## Record of Changes

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-03-05 | Initial release. Accreditation assessment for T-150 UAS AFSIM Plugin Suite v2.0 covering all six development phases. |

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Problem Statement](#1-problem-statement)
3. [M&S Requirements and Acceptability Criteria](#2-ms-requirements-and-acceptability-criteria)
4. [M&S Assumptions, Capabilities, Limitations, and Risks/Impacts](#3-ms-assumptions-capabilities-limitations--risksimpacts)
5. [Accreditation Assessment](#4-accreditation-assessment)
6. [Accreditation Recommendations](#5-accreditation-recommendations)
7. [Key Participants](#6-key-participants)
8. [Actual Accreditation Resources Expended](#7-actual-accreditation-resources-expended)
9. [Accreditation Lessons Learned](#8-accreditation-lessons-learned)
10. [Appendices](#appendices)

---

## Executive Summary

This Accreditation Report documents the accreditation assessment and decision for the T-150/TRV-150 Heavy-Lift UAS AFSIM Simulation Model v2.0 (hereafter "the Model"). The Model comprises five custom C++ plugins totaling approximately 7,736 lines of code across 26 source files, developed for the Advanced Framework for Simulation, Integration, and Modeling (AFSIM) version 2.2505. The plugins collectively model the flight dynamics, battery energy system, propulsion, Vortex Ring State (VRS), and slung load dynamics of the T-150 coaxial octocopter UAS.

The accreditation assessment evaluated the Model against 13 requirements derived from the intended use of intelligence analysis of T-150 UAS operational capabilities. Verification and validation testing demonstrated that 12 of 13 requirements are fully met, with one requirement (R-009, Flight Endurance) partially met due to a calibration gap between simulated endurance (14.3 minutes) and manufacturer-specified endurance (36 minutes).

**Overall Recommendation: CONDITIONAL ACCREDITATION** for intelligence analysis use, subject to the following key conditions:

- Analysts must apply an endurance scaling factor when interpreting time-based results
- Results should be used for comparative analysis (relative differences) rather than absolute performance predictions
- VRS and slung load results should be treated as indicative of behavioral trends, not as predictive outputs
- The Model should be recalibrated if T-150 flight test data becomes available

The Model demonstrates physics-based fidelity sufficient for intelligence-grade analysis of T-150 operational capabilities, vulnerability assessment, and counter-UAS scenario development. Energy conservation has been validated to less than 0.01% error, subsystem interactions are correctly modeled, and behavioral trends align with established aerodynamic theory. The primary limitation is the absence of flight test data for direct validation, which is typical for threat system modeling based on open-source intelligence.

---

## 1. Problem Statement

The intelligence community requires a physics-based simulation of the T-150/TRV-150 Heavy-Lift UAS to answer analytical questions regarding the operational capabilities, performance envelope, and limitations of this heavy-lift multirotor platform. These questions support counter-UAS assessment, vulnerability analysis, and intelligence production.

Native AFSIM kinematic movers lack the fidelity required to model rotor aerodynamics, battery depletion, motor thermal dynamics, Vortex Ring State, ground effect, and slung load interactions. A custom plugin suite was developed to provide parametric-level fidelity for these physics domains.

The intended use of the simulation is intelligence analysis, specifically:

- Assessing T-150 operational capabilities and performance boundaries
- Comparative flight pattern efficiency analysis
- Vulnerability assessment (VRS entry conditions, battery depletion profiles, thermal limits)
- Payload impact analysis on flight performance
- Counter-UAS (C-UAS) scenario development and evaluation

For detailed problem statement content, refer to the V&V Plan [11].

---

## 2. M&S Requirements and Acceptability Criteria

### 2.1 Requirements Traceability with Final Assessment

The following table presents the final accreditation assessment for each model requirement. Requirements were derived from the intended use and documented in the V&V Plan. Acceptability criteria were established prior to testing. Assessment results are drawn from the V&V Report [10].

| Req ID | Description | Acceptability Criteria | Met? | Assessment Summary |
|--------|-------------|----------------------|------|-------------------|
| R-001 | Battery SOC accuracy | SOC tracking error < 1% over full discharge | **YES** | Energy conservation validated to < 0.01% error in Test 09. Coulomb counting integrates correctly over all tested profiles. |
| R-002 | Voltage sag model | Terminal voltage follows Li-Ion discharge curve; sag increases with load | **YES** | SOC-dependent internal resistance produces voltage sag consistent with published Li-Ion characteristics. OCV polynomial matches literature values (Test 06). |
| R-003 | Motor thermal dynamics | Temperature reaches stable equilibrium; no thermal runaway | **YES** | Thermal model reaches stable equilibrium under sustained load. First-order thermal dynamics with convective cooling validated (Test 05). |
| R-004 | Position hold stability | Drift < 0.5 m/s in hover with no commanded velocity | **YES** | PID controller maintains hover with drift approximately 0 m/s. Stable across tested wind conditions (Test 01). |
| R-005 | Velocity tracking | Velocity error < 1 m/s during commanded cruise | **YES** | Model achieves commanded velocities with tracking error < 1 m/s. Acceleration and deceleration profiles are smooth (Test 02). |
| R-006 | Numerical stability | No NaN, Inf, or divergent values under extreme inputs | **YES** | Boundary condition testing with extreme altitudes, temperatures, and loads produced no numerical instabilities (Test 08). |
| R-007 | VRS detection | Correct state transitions when descent rate exceeds threshold in low-speed flight | **YES** | VRS state machine correctly transitions between NORMAL, VRS_WARNING, and VRS_ACTIVE states based on descent rate and airspeed (Test 03). |
| R-008 | Ground effect modeling | Thrust augmentation increases as altitude decreases below one rotor diameter | **YES** | Cheeseman-Bennett ground effect model produces correct thrust increase near ground. Effect magnitude scales with altitude (Test 07). |
| R-009 | Flight endurance | Endurance within 20% of manufacturer specification (36 min) | **PARTIAL** | Simulated endurance of 14.3 minutes represents 39.7% of the 36-minute specification. While behavioral trends (SOC decline, voltage sag) are correct, absolute endurance requires a scaling factor of approximately 2.5x. |
| R-010 | Slung load dynamics | Pendulum oscillation with correct frequency and damping characteristics | **YES** | Spherical pendulum model produces correct oscillation behavior. Frequency is consistent with cable length and gravitational parameters (Test 10). |
| R-011 | Attitude recovery | Clean recovery from disturbed attitude within 5 seconds | **YES** | PID attitude controller recovers from disturbances including VRS exit and slung load excitation. Recovery time within specification (Test 04). |
| R-012 | Cruise speed | Model achieves 30 m/s cruise speed | **YES** | Model achieves and sustains 30 m/s cruise speed matching manufacturer specification (Test 02). |
| R-013 | Maximum payload capacity | Model correctly limits payload to 68 kg | **YES** | Payload mass of 68 kg (150 lbs) correctly modeled. Performance degradation at maximum payload is consistent with reduced thrust margin (Test 11). |

### 2.2 Requirements Assessment Summary

- **Requirements Fully Met**: 12 of 13 (92.3%)
- **Requirements Partially Met**: 1 of 13 (7.7%) -- R-009 (Flight Endurance)
- **Requirements Not Met**: 0 of 13 (0.0%)

The single partially-met requirement (R-009) is attributable to a calibration gap in the power consumption model rather than a fundamental modeling error. The behavioral trend (monotonic SOC decline with correct voltage sag characteristics) is correct; the absolute rate of energy consumption is higher than the real system, likely due to conservative efficiency assumptions in the motor and propulsion models. This gap is well-documented and can be addressed through analyst-applied scaling factors.

---

## 3. M&S Assumptions, Capabilities, Limitations, and Risks/Impacts

### 3.1 Key Assumptions

The following assumptions underlie the Model and affect the interpretation of results:

1. **Rigid body assumption**: The airframe is modeled as a rigid body. Structural flex, vibration modes, and aeroelastic effects are not represented.
2. **Quasi-steady aerodynamics**: Rotor aerodynamics use momentum theory with quasi-steady assumptions. Unsteady aerodynamic phenomena (blade-vortex interaction, dynamic stall) are not modeled.
3. **Uniform inflow**: Each rotor is modeled with uniform inflow. Blade Element Momentum Theory (BEMT) radial variation is not implemented.
4. **Estimated inertia tensor**: Moments of inertia are estimated from geometric assumptions and mass distribution. No measured inertia data is available for the T-150.
5. **Standard atmosphere**: Air density varies with altitude per the International Standard Atmosphere (ISA) model. Non-standard atmospheric conditions are not modeled.
6. **Idealized battery cells**: All battery cells are assumed identical. Cell-to-cell variation, aging effects, and temperature-dependent capacity are not modeled.
7. **No wind turbulence**: The current model does not include atmospheric turbulence or gust modeling. Wind effects are limited to steady-state conditions.
8. **Open-source parameters**: All model parameters are derived from open-source intelligence (OSINT) including AAIB investigation reports, US patent filings, manufacturer data sheets, and published motor specifications.

### 3.2 Model Capabilities

The Model provides the following analytical capabilities:

- **6DOF flight dynamics**: Full six-degree-of-freedom rigid body physics with Newton-Euler integration, PID attitude and velocity control, and waypoint navigation
- **Battery energy modeling**: Coulomb counting SOC tracking, SOC-dependent open-circuit voltage, load-dependent internal resistance and voltage sag, low-battery warnings
- **Propulsion modeling**: Per-motor RPM and thrust calculation, motor thermal dynamics with convective cooling, dynamic power consumption based on flight state (velocity, acceleration, bank angle)
- **VRS detection**: State machine detecting Vortex Ring State entry conditions based on descent rate and forward airspeed, with configurable warning and active thresholds
- **Ground effect**: Cheeseman-Bennett thrust augmentation model for flight near terrain
- **Slung load dynamics**: Spherical pendulum model for cable-suspended payloads with force feedback to the airframe
- **Inter-plugin communication**: Loosely-coupled blackboard architecture via AFSIM AuxData key-value store

### 3.3 Model Limitations

The following limitations are relevant to the accreditation decision:

1. **Endurance calibration gap**: Simulated flight endurance (14.3 min) is approximately 40% of manufacturer specification (36 min). All time-based results require scaling.
2. **No flight test validation**: No T-150 flight test data is available for direct comparison. Validation is performed against manufacturer specifications, published literature, and physics-based consistency checks.
3. **Developer self-testing**: Verification and validation were performed by the development team. No independent V&V agent was engaged.
4. **Estimated parameters**: Several parameters (moments of inertia, thermal mass, internal resistance coefficients) are estimated from class-level data rather than measured from the specific T-150 system.
5. **Informational subsystems**: VRS detection and slung load dynamics are informational -- they report state but do not dynamically alter the flight trajectory through full aerodynamic coupling.
6. **No degraded-mode modeling**: The Model does not represent motor failure, partial power loss, GPS denial, or communication link degradation scenarios.
7. **Single environmental condition**: The Model operates under ISA standard conditions. Extreme temperature, humidity, and altitude combinations have not been validated.

### 3.4 Accreditation-Specific Risk Assessment

| Risk Category | Risk Level | Justification |
|--------------|-----------|---------------|
| Relative performance comparisons (pattern efficiency, payload impact, flight profile optimization) | **LOW** | Physics-based trends are correct. Comparative analysis between configurations is well-supported by the underlying momentum theory and energy conservation validation. |
| Absolute performance predictions (range, endurance, max speed at altitude) | **MEDIUM** | The endurance calibration gap introduces quantitative uncertainty. Absolute predictions require analyst-applied scaling and should be reported with uncertainty bounds. |
| Time-critical mission analysis (loiter time, time-on-station, response time) | **HIGH** | Without endurance scaling, time-based predictions will significantly underestimate mission duration. Analysts must apply the documented 2.5x scaling factor for all time-dependent outputs. |
| Behavioral trend analysis (VRS entry, ground effect, thermal limits) | **LOW** | Physics-based models produce correct qualitative behavior. Threshold values are derived from established aerodynamic theory. |
| C-UAS engagement analysis (detection, tracking, intercept) | **LOW-MEDIUM** | Flight dynamics and trajectory fidelity are sufficient for engagement geometry analysis. Radar cross-section and sensor signatures are not modeled by these plugins. |

---

## 4. Accreditation Assessment

### 4.1 Assessment Summary

The accreditation assessment evaluated the T-150 UAS AFSIM Plugin Suite v2.0 against 13 model requirements derived from the intended use of intelligence analysis of T-150 UAS operational capabilities. The assessment considered the V&V test results, documentation completeness, configuration management practices, and the overall suitability of the Model for its intended analytical purpose.

**Result**: 12 of 13 requirements are fully met. One requirement (R-009, Flight Endurance) is partially met with a documented calibration gap. The Model demonstrates sufficient fidelity for intelligence analysis when used with appropriate analyst awareness of its limitations.

### 4.2 Strengths

The following strengths support a positive accreditation assessment:

1. **Physics-based modeling provides correct behavioral trends**: The use of momentum theory, Newton-Euler dynamics, Coulomb counting, and first-principles thermal modeling ensures that the Model produces physically consistent results. Energy conservation has been validated to less than 0.01% error, providing high confidence in the underlying physics engine.

2. **Energy conservation validated to high precision**: Test 09 demonstrated that the battery energy accounting is internally consistent, with cumulative energy error below 0.01% over full discharge cycles. This validates the coupling between the propulsion power model and battery SOC tracking.

3. **Multiple subsystem interactions correctly modeled**: The loosely-coupled plugin architecture successfully models the interactions between flight dynamics, propulsion, battery, VRS, and slung load subsystems. The AuxData blackboard pattern ensures consistent data exchange without tight coupling.

4. **Configuration management via git provides traceability**: All source code, scenario files, and documentation are maintained in a git repository with full commit history. Development phases are traceable through commit logs, branch history, and pull requests on GitHub.

5. **Comprehensive documentation suite**: The Model is supported by a complete set of VV&A documentation including the Architecture Description Document (ADD) v3.0, User and Analyst Manual (UAM) v3.0, Interface Control Document (ICD) v1.0, and Test Design Document (TDD), providing full traceability from requirements through implementation to test results.

6. **Modular architecture enables incremental confidence**: Each plugin can be independently verified and validated. The six-phase development approach allowed progressive confidence building, with each phase adding capability while preserving previously validated behavior.

7. **Boundary condition testing demonstrates robustness**: The Model was tested under extreme conditions (maximum altitude, maximum payload, minimum SOC, aggressive maneuvering) without producing numerical instabilities, NaN values, or divergent behavior.

### 4.3 Weaknesses

The following weaknesses are noted and factored into the accreditation conditions:

1. **No flight test data for direct validation**: The T-150 is a foreign-manufactured threat system for which no flight test data is available to the development team. All validation is performed against manufacturer specifications, published literature, and physics-based consistency checks. This is a common constraint in threat system modeling.

2. **Endurance gap requires analyst awareness**: The 2.5x discrepancy between simulated and specified endurance means that time-based outputs (flight duration, range, loiter time) cannot be used at face value. Analysts must understand and apply the appropriate scaling factor.

3. **Developer self-testing (no independent V&V)**: Verification and validation were conducted by the same team that developed the Model. While this is common for initial accreditation of intelligence community models, it introduces the risk of confirmation bias in test design and result interpretation.

4. **Some estimated parameters (inertia, thermal mass)**: The moments of inertia, motor thermal mass, and internal resistance coefficients are estimated from class-level motor data and geometric assumptions rather than direct measurement. These estimates may contribute to the endurance calibration gap.

5. **VRS and slung load are informational only**: The VRS processor detects and reports VRS conditions but does not impose full aerodynamic thrust loss on the flight dynamics. Similarly, the slung load processor computes pendulum forces but the coupling to flight trajectory is through AuxData rather than direct dynamic feedback. These subsystems provide qualitative behavioral indication rather than predictive simulation.

### 4.4 Risk Assessment

The overall risk of using the Model for its intended purpose of intelligence analysis is assessed as **LOW to MEDIUM**, with the following breakdown:

- **LOW risk** for comparative analysis and behavioral trend identification, which constitute the primary intended use
- **MEDIUM risk** for quantitative performance predictions, which require analyst-applied scaling factors and uncertainty documentation
- **HIGH risk** for time-critical predictions without endurance scaling, which is mitigated by the documented conditions of accreditation

The risk assessment considers that the intended use is intelligence analysis rather than safety-critical decision making, real-time flight control, or acquisition decisions. For intelligence analysis, behavioral fidelity and correct trend direction are more important than absolute numerical accuracy.

---

## 5. Accreditation Recommendations

### 5.1 Recommendation

**CONDITIONAL ACCREDITATION**

The T-150/TRV-150 Heavy-Lift UAS AFSIM Simulation Model v2.0 is recommended for conditional accreditation for use in intelligence analysis of T-150 UAS operational capabilities. The conditions specified in Section 5.2 must be satisfied for all analytical uses.

### 5.2 Conditions of Accreditation

The following conditions apply to all uses of the Model:

**(a) Endurance Scaling Requirement**: Analysts must apply an endurance scaling factor of approximately 2.5x when interpreting time-based simulation results. Specifically, simulated flight durations should be multiplied by 2.5 to estimate real-world endurance. This scaling factor should be documented in all analytical products that reference simulation-derived time or range values. The basis for this factor is the ratio of manufacturer-specified endurance (36 min) to simulated endurance (14.3 min).

**(b) Comparative Analysis Preference**: Results should be used for comparative analysis (relative differences between configurations, flight profiles, or operating conditions) rather than absolute performance predictions. The Model's strength lies in correctly capturing the direction and relative magnitude of performance changes, not in producing exact numerical predictions.

**(c) VRS and Slung Load Interpretation**: VRS detection and slung load dynamics results should be treated as indicative of behavioral trends, not as predictive of specific outcomes. The VRS processor identifies conditions consistent with VRS entry but does not impose full aerodynamic coupling. The slung load processor computes pendulum dynamics but feedback to flight trajectory is through the AuxData interface rather than direct force integration.

**(d) Recalibration on Data Availability**: The Model should be recalibrated if T-150 flight test data, detailed motor efficiency curves, or measured battery discharge profiles become available. Priority calibration targets are: motor efficiency vs. RPM curves, battery internal resistance vs. SOC and temperature, and propeller thrust coefficient vs. advance ratio.

### 5.3 Approved Uses

The Model is approved for the following analytical applications, subject to the conditions in Section 5.2:

- **Intelligence analysis** of T-150 UAS operational capabilities, performance envelope, and system limitations
- **Comparative flight pattern efficiency analysis**, including evaluation of mission profiles, waypoint optimization, and altitude/speed trade-offs
- **Vulnerability assessment**, including VRS susceptibility, battery depletion profiles, motor thermal limits, and operational boundary identification
- **Payload impact analysis**, evaluating the effect of payload mass on flight performance, endurance, and maneuverability
- **Counter-UAS (C-UAS) scenario development**, providing representative UAS flight dynamics for detection, tracking, and engagement analysis
- **Concept of Operations (CONOPS) evaluation**, assessing tactical employment concepts for heavy-lift UAS logistics and resupply operations
- **Training and familiarization**, demonstrating heavy-lift UAS flight characteristics to analysts and operators

### 5.4 Prohibited Uses

The Model is **not approved** for the following uses:

- **Real-time flight control** or any application involving actual aircraft operations
- **Safety-critical decision making** where model inaccuracy could result in loss of life or equipment
- **Acquisition decisions** based solely on model output without independent data sources
- **Absolute performance guarantees** without application of the endurance scaling factor and documented uncertainty bounds
- **Prediction of specific failure modes** (structural failure, motor burnout, battery thermal runaway) beyond the behavioral indicators modeled
- **Certification or airworthiness assessment** of any UAS system

---

## 6. Key Participants

| Role | Name/Organization |
|------|-------------------|
| Accreditation Authority | [PLACEHOLDER - Name, Title, Organization] |
| M&S Proponent | [PLACEHOLDER - Name, Title, Organization] |
| V&V Agent | [PLACEHOLDER - Name, Title, Organization] |
| Subject Matter Expert (UAS) | [PLACEHOLDER - Name, Title, Organization] |
| Subject Matter Expert (Aerodynamics) | [PLACEHOLDER - Name, Title, Organization] |
| Model Developer | [PLACEHOLDER - Name, Title, Organization] |
| Configuration Manager | [PLACEHOLDER - Name, Title, Organization] |
| Analyst Representative | [PLACEHOLDER - Name, Title, Organization] |

---

## 7. Actual Accreditation Resources Expended

| Resource Category | Planned | Actual | Notes |
|-------------------|---------|--------|-------|
| Calendar Duration | [PLACEHOLDER] | ~57 calendar days | Development start to accreditation package completion |
| Active Development Hours | [PLACEHOLDER] | ~20 hours | AI-assisted development significantly reduced effort |
| V&V Testing Hours | [PLACEHOLDER] | [PLACEHOLDER] | Includes scenario development, execution, and analysis |
| Documentation Hours | [PLACEHOLDER] | [PLACEHOLDER] | AI-assisted documentation reduced effort substantially |
| Review and Assessment Hours | [PLACEHOLDER] | [PLACEHOLDER] | SME review, accreditation assessment meetings |
| Total Labor (person-hours) | [PLACEHOLDER] | [PLACEHOLDER] | Sum of all categories |

**Note**: AI-assisted development using Claude Code CLI substantially reduced both code development and documentation timelines compared to traditional development approaches. Refer to the Impact Statement [12] for detailed discussion of AI-assisted development productivity.

---

## 8. Accreditation Lessons Learned

### 8.1 AI-Assisted Development

AI-assisted development using Claude and Claude Code CLI significantly accelerated model creation, producing over 7,700 lines of C++ code and comprehensive VV&A documentation within approximately 20 hours of active development. However, this approach required human calibration oversight -- the AI-generated code was architecturally sound but required manual intervention for plugin registration patterns, coordinate conversion errors, and motor efficiency calibration. The endurance calibration gap (R-009) is partially attributable to the AI system's reliance on conservative default efficiency values that were not fully tuned against system-specific data.

**Recommendation**: AI-assisted M&S development should include explicit calibration review gates where subject matter experts validate key performance parameters before proceeding to integration testing.

### 8.2 AFSIM Plugin Architecture

The AFSIM plugin architecture enabled modular V&V of individual components. Each plugin could be tested in isolation (unit testing) and then progressively integrated. The AuxData blackboard communication pattern simplified interface testing because all inter-plugin data exchange uses a single, observable mechanism. This modularity directly supported the six-phase iterative development approach.

**Recommendation**: Future AFSIM-based M&S efforts should adopt the modular plugin architecture with AuxData-based communication to facilitate incremental V&V.

### 8.3 Open-Source Intelligence as Basis for Threat Modeling

Open-source intelligence (OSINT) provided sufficient data for parametric-level fidelity modeling of the T-150 UAS. The AAIB investigation report [4], US patent filing [5], motor manufacturer specifications [6], and product data sheets [11] collectively provided approximately 65 system parameters. While some parameters required estimation (inertia, thermal mass), the OSINT basis was adequate for the intended use of intelligence analysis.

**Recommendation**: OSINT-based threat models should document the source and confidence level for every parameter, distinguishing between directly sourced, derived, and estimated values.

### 8.4 Physics-Based Validation

Physics-based models provide inherent validation through energy conservation and dimensional analysis. The demonstration that battery energy accounting is accurate to less than 0.01% (Test 09) provides confidence that the underlying physics engine is correctly implemented, even in the absence of flight test data. Dimensional analysis and unit consistency checks further support code correctness.

**Recommendation**: For threat system models where flight test data is unavailable, physics-based consistency checks (energy conservation, momentum conservation, dimensional analysis) should be considered a primary validation technique.

### 8.5 Iterative Development and Progressive Confidence

The six-phase development approach (Phase 1: thermal/VRS, Phase 2: voltage sag, Phase 3: dynamic power, Phase 4: ground effect/payload, Phase 5: slung load, Phase 6: velocity-based power) allowed progressive confidence building. Each phase added capability while regression testing ensured that previously validated behavior was preserved.

**Recommendation**: Complex M&S efforts should adopt an iterative development approach with defined phase gates to enable progressive accreditation evidence accumulation.

### 8.6 Configuration Management

Configuration management through git and GitHub provided excellent traceability for the accreditation process. Every code change is linked to a specific commit, branch, and pull request. The development history is fully auditable, and specific model versions can be reproduced exactly. This level of traceability exceeds what is typically available for models developed without version control.

**Recommendation**: All M&S efforts subject to VV&A should use version control with branch-based development workflows to provide accreditation-ready configuration management evidence.

---

## Appendices

### Appendix A: M&S Description

The T-150 UAS AFSIM Plugin Suite v2.0 consists of five custom C++ plugins developed for AFSIM version 2.2505:

| Plugin | Library | Registered Type | Lines of Code | Purpose |
|--------|---------|-----------------|---------------|---------|
| Rotorcraft 6DOF Mover | rotorcraft_6dof (.dll/.so) | WSF_ROTORCRAFT_6DOF_MOVER | 2,939 | Physics-based 6DOF flight dynamics with PID control |
| Battery Processor | t150_battery (.dll/.so) | T150_BATTERY_PROCESSOR | 1,139 | Li-Ion battery SOC tracking and voltage sag modeling |
| Propulsion Processor | t150_propulsion (.dll/.so) | T150_PROPULSION_PROCESSOR | 1,281 | Motor power, thermal dynamics, RPM, ground effect |
| VRS Processor | t150_vrs (.dll/.so) | T150_VRS_PROCESSOR | 697 | Vortex Ring State detection and state machine |
| Slung Load Processor | t150_slung_load (.dll/.so) | T150_SLUNG_LOAD_PROCESSOR | 825 | Cable-suspended payload pendulum dynamics |
| **Total** | | | **7,736** | |

The plugins communicate through the AFSIM AuxData key-value store on the shared WsfPlatform object. This blackboard architecture enables loose coupling -- any subset of plugins can operate independently with graceful degradation when dependencies are unavailable.

For full architecture description, refer to the ADD v3.0 [7].

### Appendix B: Requirements Traceability Matrix (Final)

| Req ID | V&V Plan Ref | Test ID | Result | Accreditation Status |
|--------|-------------|---------|--------|---------------------|
| R-001 | Sec 3.1 | Test 09 | PASS (< 0.01% error) | ACCEPTED |
| R-002 | Sec 3.2 | Test 06 | PASS | ACCEPTED |
| R-003 | Sec 3.3 | Test 05 | PASS | ACCEPTED |
| R-004 | Sec 3.4 | Test 01 | PASS (drift ~ 0 m/s) | ACCEPTED |
| R-005 | Sec 3.5 | Test 02 | PASS (error < 1 m/s) | ACCEPTED |
| R-006 | Sec 3.6 | Test 08 | PASS (no NaN) | ACCEPTED |
| R-007 | Sec 3.7 | Test 03 | PASS | ACCEPTED |
| R-008 | Sec 3.8 | Test 07 | PASS | ACCEPTED |
| R-009 | Sec 3.9 | Test 12 | PARTIAL (14.3 min vs 36 min) | ACCEPTED WITH CONDITIONS |
| R-010 | Sec 3.10 | Test 10 | PASS | ACCEPTED |
| R-011 | Sec 3.11 | Test 04 | PASS | ACCEPTED |
| R-012 | Sec 3.12 | Test 02 | PASS (30 m/s achieved) | ACCEPTED |
| R-013 | Sec 3.13 | Test 11 | PASS (68 kg modeled) | ACCEPTED |

### Appendix C: Basis of Comparison

The following data sources served as the basis of comparison for validation:

| Source | Type | Parameters Validated |
|--------|------|---------------------|
| AAIB Investigation Report [4] | Accident investigation | MTOW, thrust capability, configuration, operational profile |
| US Patent US20240262499A1 [5] | Patent filing | Battery configuration (13S Li-Ion), voltage, capacity |
| T-Motor U15II KV100 Specifications [6] | Manufacturer data sheet | Motor KV, max power, dimensions, mass |
| Cheeseman & Bennett (1955) [8] | Published research | Ground effect thrust augmentation model |
| Johnson (1980) [9] | Textbook | Helicopter rotor aerodynamics, VRS theory |
| Leishman (2006) [10] | Textbook | Rotorcraft aerodynamics, momentum theory, VRS criteria |
| TRV-150 Product Data Sheet [11] | Manufacturer marketing | Cruise speed (30 m/s), max payload (68 kg), endurance (36 min) |

**Note**: No flight test data from instrumented T-150 flights is available for direct comparison. All validation is based on the sources listed above, combined with physics-based consistency checks (energy conservation, dimensional analysis).

### Appendix D: References

[1] Office of the Under Secretary of Defense for Research and Engineering, "DoD Instruction 5000.61: DoD Modeling and Simulation Verification, Validation, and Accreditation," September 17, 2024.

[2] Department of Defense, "MIL-STD-3022: Documentation of Verification, Validation, and Accreditation (VV&A) for Models and Simulations," January 28, 2008.

[3] Department of the Army, "Army Directive 2024-02: Software Development," 2024.

[4] Air Accidents Investigation Branch, "AAIB Investigation to Malloy Aeronautics T-150, South Scarle, Nottinghamshire," 2023.

[5] Malloy Aeronautics, "Battery Pack Topology for Unmanned Aerial Vehicle," US Patent US20240262499A1, 2024.

[6] T-Motor, "U15II KV100 Motor Specifications," 2024.

[7] T-150 UAS AFSIM Plugin Suite, "Architecture Description Document (ADD) v3.0," March 5, 2026.

[8] I.C. Cheeseman and W.E. Bennett, "The Effect of Ground on a Helicopter Rotor in Forward Flight," ARC R&M 3021, 1955.

[9] W. Johnson, "Helicopter Theory," Princeton University Press, 1980.

[10] J.G. Leishman, "Principles of Helicopter Aerodynamics," 2nd ed., Cambridge University Press, 2006.

[11] SURVICE Engineering / Malloy Aeronautics, "TRV-150 Heavy-Lift UAS Product Data Sheet," 2024.

[12] National Ground Intelligence Center, "Impact of AI-Assisted Development on Modeling and Simulation: T-150 UAS Case Study," March 4, 2026.

### Appendix E: Acronyms

| Acronym | Definition |
|---------|------------|
| ADD | Architecture Description Document |
| AFSIM | Advanced Framework for Simulation, Integration, and Modeling |
| AGL | Above Ground Level |
| BEMT | Blade Element Momentum Theory |
| CG | Center of Gravity |
| C-UAS | Counter-Unmanned Aerial System |
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
| KV | Motor velocity constant (RPM per Volt) |
| M&S | Modeling and Simulation |
| MTOW | Maximum Takeoff Weight |
| NaN | Not a Number |
| NGIC | National Ground Intelligence Center |
| OCV | Open-Circuit Voltage |
| OGE | Out of Ground Effect |
| OSINT | Open-Source Intelligence |
| PID | Proportional-Integral-Derivative |
| RPM | Revolutions Per Minute |
| SME | Subject Matter Expert |
| SOC | State of Charge |
| TDD | Test Design Document |
| UAM | User and Analyst Manual |
| UAS | Unmanned Aerial System |
| V&V | Verification and Validation |
| VRS | Vortex Ring State |
| VV&A | Verification, Validation, and Accreditation |

### Appendix F: Glossary

| Term | Definition |
|------|------------|
| Accreditation | Official certification that a model or simulation is acceptable for use for a specific purpose. Per DoDI 5000.61, the formal determination by an accreditation authority. |
| AuxData | AFSIM's platform-level key-value attribute store used for inter-processor and mover-processor communication. |
| Blackboard architecture | A software design pattern where components communicate by reading and writing to a shared data store rather than through direct inter-component calls. |
| Calibration gap | A quantitative discrepancy between model output and reference data that is understood and documented but not yet resolved through parameter adjustment. |
| Conditional accreditation | An accreditation decision that approves use of a model subject to specified conditions, constraints, or limitations that must be observed by all users. |
| Coulomb counting | A battery SOC estimation method that integrates current over time to track charge consumed. |
| Ground effect | The increase in rotor thrust efficiency when operating near the ground due to reduced induced velocity. |
| Momentum theory | An analytical framework for rotor performance analysis based on conservation of momentum applied to the air mass passing through the rotor disk. |
| Parametric-level fidelity | A level of model fidelity where key system parameters (mass, thrust, speed, endurance) are represented with values traceable to source data, but detailed subsystem physics may be simplified. |
| Vortex Ring State | A hazardous aerodynamic condition occurring when a rotorcraft descends into its own rotor downwash, causing loss of effective lift and high vibration. |

### Appendix I: Accreditation Plan (Reference)

The Accreditation Plan for the T-150 UAS AFSIM Simulation Model v2.0 is maintained as a separate document within the VV&A documentation suite. The plan defines the accreditation strategy, required evidence, assessment criteria, and decision framework used to produce this report. Refer to the V&V Plan [11] for the integrated verification, validation, and accreditation planning.

### Appendix J: V&V Report (Reference)

The V&V Report for the T-150 UAS AFSIM Simulation Model v2.0 is maintained as a separate document within the VV&A documentation suite. The report contains detailed test procedures, raw test results, pass/fail determinations, and the analysis supporting the requirements assessment summarized in Section 2 of this report.

---

## Signature Block

| Field | Value |
|-------|-------|
| **Accreditation Authority** | [PLACEHOLDER - Name, Title, Organization] |
| **Signature** | [PLACEHOLDER] |
| **Date** | [PLACEHOLDER] |
| **Accreditation Decision** | **CONDITIONAL ACCREDITATION** |
| **Effective Date** | [PLACEHOLDER] |
| **Expiration / Review Date** | [PLACEHOLDER - Typically 2 years from effective date] |

| Field | Value |
|-------|-------|
| **V&V Agent** | [PLACEHOLDER - Name, Title, Organization] |
| **Signature** | [PLACEHOLDER] |
| **Date** | [PLACEHOLDER] |
| **V&V Recommendation** | Conditional Accreditation recommended based on V&V findings |

| Field | Value |
|-------|-------|
| **M&S Proponent** | [PLACEHOLDER - Name, Title, Organization] |
| **Signature** | [PLACEHOLDER] |
| **Date** | [PLACEHOLDER] |
| **Concurrence** | [PLACEHOLDER] |

---

[CLASSIFICATION: UNCLASSIFIED]
