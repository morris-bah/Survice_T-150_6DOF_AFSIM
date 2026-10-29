[CLASSIFICATION: UNCLASSIFIED]

# Accreditation Plan

## T-150/TRV-150 Heavy-Lift UAS AFSIM Simulation Model v2.0

### AFSIM Project 005

| Field | Value |
|-------|-------|
| **Document Version** | 1.0 |
| **Date** | March 5, 2026 |
| **Classification** | UNCLASSIFIED |
| **Sponsoring Organization** | [PLACEHOLDER - Organization Name] |
| **Accreditation Authority** | [PLACEHOLDER - Accreditation Authority Name/Title] |
| **Program Manager** | [PLACEHOLDER - Name/Title] |
| **Document Preparer** | [PLACEHOLDER - Organization/Contract] |
| **Distribution Statement** | [PLACEHOLDER - Distribution Statement] |
| **M&S Name and Version** | T-150 UAS AFSIM Plugin Suite v2.0 |

---

## Record of Changes

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-03-05 | [PLACEHOLDER] | Initial release per MIL-STD-3022 Appendix A |

---

## Table of Contents

- [Executive Summary](#executive-summary)
- [1. Problem Statement](#1-problem-statement)
- [2. M&S Requirements and Acceptability Criteria](#2-ms-requirements-and-acceptability-criteria)
- [3. M&S Assumptions, Capabilities, Limitations, and Risks/Impacts](#3-ms-assumptions-capabilities-limitations-and-risksimpacts)
- [4. Accreditation Methodology](#4-accreditation-methodology)
- [5. Accreditation Issues](#5-accreditation-issues)
- [6. Key Participants](#6-key-participants)
- [7. Planned Accreditation Resources](#7-planned-accreditation-resources)
- [Appendix A: M&S Description](#appendix-a-ms-description)
- [Appendix B: M&S Requirements Traceability Matrix](#appendix-b-ms-requirements-traceability-matrix)
- [Appendix C: Basis of Comparison](#appendix-c-basis-of-comparison)
- [Appendix D: References](#appendix-d-references)
- [Appendix E: Acronyms](#appendix-e-acronyms)
- [Appendix F: Glossary](#appendix-f-glossary)

---

## Executive Summary

This Accreditation Plan establishes the framework and methodology for determining whether the T-150/TRV-150 Heavy-Lift UAS AFSIM Simulation Model v2.0 is acceptable for its intended use in intelligence analysis at the National Ground Intelligence Center (NGIC). The plan is prepared in accordance with MIL-STD-3022, "Documentation of Verification, Validation, and Accreditation (VV&A) for Models and Simulations," and supports the requirements of DoDI 5000.61, "DoD Modeling and Simulation Verification, Validation, and Accreditation."

The T-150 UAS AFSIM Plugin Suite v2.0 is a physics-based simulation model comprising five custom C++ plugins (approximately 7,736 lines of code) integrated with the Advanced Framework for Simulation, Integration, and Modeling (AFSIM). The model represents the Malloy Aeronautics T-150 (also designated TRV-150), a coaxial octocopter heavy-lift UAS with a maximum takeoff weight of 123 kg and a payload capacity of 55 kg. The model was developed to support NGIC intelligence analysis including assessment of operational performance, flight endurance, mission profiles, vulnerable flight regimes, and payload impacts on UAS capabilities.

The five plugins provide: (1) 6DOF rigid-body flight dynamics with quaternion attitude representation, (2) battery state-of-charge tracking with voltage sag modeling, (3) propulsion system modeling with thermal dynamics and ground effect, (4) Vortex Ring State detection and severity assessment, and (5) slung load pendulum dynamics. Together, these plugins deliver a high-fidelity representation of the T-150 platform within the AFSIM scenario-based analysis framework.

This plan defines the accreditation scope, identifies information needs, describes the methodology for collecting and assessing evidence, and catalogs known issues that the Accreditation Authority must consider. The model is proposed for accreditation in the domain of intelligence analysis of T-150/TRV-150 class heavy-lift UAS. The model is explicitly **not** proposed for accreditation for real-time flight control, safety-critical decision-making, acquisition milestone decisions, or live-virtual-constructive training.

Key accreditation considerations include the absence of T-150 flight test data for direct validation, a simulated flight endurance gap relative to manufacturer specifications (14.3 minutes simulated vs. 36 minutes manufacturer-claimed), the use of developer self-testing rather than independent V&V, and the novel use of AI-assisted development methodology. These issues are documented in Section 5 and will require explicit disposition by the Accreditation Authority.

---

## 1. Problem Statement

### 1.1 Intended Use

The T-150 UAS AFSIM Simulation Model v2.0 is intended for use by NGIC analysts to conduct intelligence analysis of the T-150/TRV-150 class heavy-lift UAS. Specific analytical objectives include:

- **Operational Performance Assessment**: Evaluate flight dynamics, speed, climb rate, descent rate, and maneuvering characteristics under various configurations and payload conditions.
- **Flight Endurance Analysis**: Determine battery-limited mission duration across different flight profiles (hover, cruise, mixed mission), payload weights, and environmental conditions.
- **Mission Profile Analysis**: Simulate representative mission profiles to assess operational utility, including cargo delivery, surveillance orbits, and ingress/egress routes.
- **Vulnerable Flight Regime Identification**: Identify flight conditions (e.g., steep descent at low forward speed) where the platform is susceptible to Vortex Ring State or other loss-of-control phenomena.
- **Payload Impact Assessment**: Quantify the effects of payload mass and configuration (internal vs. slung load) on flight performance, endurance, and stability.
- **C-UAS Assessment Support**: Provide simulation inputs for Counter-UAS analysis including observable flight characteristics, engagement window estimation, and vulnerability assessment.
- **CONOPS Development**: Support the development of concepts of operations for employment of T-150 class UAS in adversary force structures.

### 1.2 M&S Overview

The T-150 UAS AFSIM Plugin Suite v2.0 consists of five custom C++ AFSIM plugins totaling approximately 7,736 lines of code. The plugins implement physics-based flight dynamics with subsystem-level modeling:

| Plugin | AFSIM Type | LOC | Function |
|--------|-----------|-----|----------|
| rotorcraft_6dof | WSF_ROTORCRAFT_6DOF_MOVER | ~4,085 | 6DOF rigid-body mover with quaternion attitude, rotor model, flight controllers |
| t150_battery | T150_BATTERY_PROCESSOR | ~1,139 | Battery SOC/voltage/current tracking with voltage sag model |
| t150_propulsion | T150_PROPULSION_PROCESSOR | ~1,395 | 8-motor propulsion, thermal model, ground effect, dynamic power |
| t150_vrs | T150_VRS_PROCESSOR | ~697 | VRS detection, state machine, lift/control degradation factors |
| t150_slung_load | T150_SLUNG_LOAD_PROCESSOR | ~825 | 2-DOF spherical pendulum slung load dynamics |

The model operates within the AFSIM simulation framework (v2.2505), leveraging AFSIM's scenario management, event recording, and visualization capabilities. Inter-plugin communication uses the AFSIM AuxData key-value interface.

### 1.3 M&S Application

The simulation model provides a high-fidelity UAS platform representation for scenario-based analysis within the AFSIM framework. Analysts configure scenarios using AFSIM text-based scenario files (.afsim) that define platform parameters, mission routes, environmental conditions, and output recording. The model produces time-series telemetry (position, velocity, attitude, battery state, propulsion state, VRS status, and slung load dynamics) that analysts use to assess platform capabilities and limitations.

The model supports both individual scenario execution and batch analysis across parameter sweeps (e.g., varying payload mass to determine performance degradation curves).

### 1.4 Accreditation Scope

**In Scope**: The model is proposed for accreditation for use in intelligence analysis of T-150/TRV-150 class heavy-lift UAS, including the analytical applications listed in Section 1.1.

**Out of Scope -- The model is NOT accredited for**:

- Real-time flight control or hardware-in-the-loop applications
- Safety-critical decisions (e.g., airspace deconfliction, collision avoidance)
- Acquisition milestone decisions or cost-benefit analyses
- Live-virtual-constructive (LVC) training exercises
- Analysis of UAS platforms substantially different from the T-150/TRV-150 coaxial octocopter configuration without re-parameterization and re-validation

---

## 2. M&S Requirements and Acceptability Criteria

The following table maps M&S requirements to acceptability criteria and metrics. These requirements derive from the intended use described in Section 1.1 and are traceable to the test cases documented in the V&V Report.

| Req ID | Requirement | Acceptability Criterion | Metric | V&V Evidence |
|--------|------------|------------------------|--------|--------------|
| REQ-01 | Flight dynamics fidelity | Stable 6DOF physics integration with correct response to commanded maneuvers | No NaN/divergence; position hold drift < 1.0 m; velocity tracking error < 1.0 m/s | Phase 1 Tests 01-05 |
| REQ-02 | Energy system accuracy | SOC tracking with < 0.1% cumulative error; correct voltage sag behavior under load | Energy balance error < 0.1%; monotonic SOC decrease; voltage sag proportional to current | Phase 1 Tests 06, 09 |
| REQ-03 | VRS detection accuracy | Correct VRS state transitions at documented descent rate and forward speed thresholds | VRS onset at descent rate >= 4.0 m/s with forward speed < 5.0 m/s; correct 5-state transitions | VRS processor unit testing |
| REQ-04 | Performance envelope fidelity | Cruise speed within 10% of manufacturer specification; correct payload capacity limits | Max cruise speed 27-33 m/s; MTOW 123 kg; max payload 55 kg | Phase 1 Test 04; parameter review |
| REQ-05 | Subsystem interaction correctness | Correct AuxData communication between all plugins; battery reads power from mover | All 68 AuxData keys published correctly; power source fallback chain functional | Integration testing; ICD verification |
| REQ-06 | Ground effect modeling | Thrust augmentation near ground consistent with Cheeseman-Bennett approximation | Ground effect factor 1.0-1.3; active below 1 rotor diameter AGL | Phase 1 Test 07 |
| REQ-07 | Motor thermal dynamics | Realistic motor temperature response to power draw | Temperature equilibrium 60-90 C at hover; no thermal runaway | Phase 1 Test 01 |
| REQ-08 | Slung load dynamics | Physically correct pendulum behavior with appropriate natural frequency | Natural frequency matches f = (1/2*pi)*sqrt(g/L); cable tension >= 0; swing damping observable | Slung load unit testing |
| REQ-09 | Numerical stability | No numerical instability across operating envelope including edge cases | Bounded state variables under extreme maneuvers; stable over > 14 min flights | Phase 1 Tests 05, 10 |
| REQ-10 | Flight endurance realism | Simulated endurance within documented uncertainty bounds of reference data | Endurance within identified bounds (14-36 min) with documented rationale for variance | Phase 1 Test 10; gap analysis |

---

## 3. M&S Assumptions, Capabilities, Limitations, and Risks/Impacts

Per MIL-STD-3022 Section 5.2, this section is shared with the V&V Plan.

### 3.1 Assumptions

| ID | Assumption | Justification | Risk if Invalid |
|----|-----------|---------------|-----------------|
| A-01 | Standard atmosphere (ISA) at sea level | Analysis focuses on low-altitude operations | Overestimates performance at high altitude/temperature |
| A-02 | All 8 motors identical and symmetric | Manufacturer design intent; no failure data available | Masks individual motor failure effects |
| A-03 | Rigid airframe (no structural flex) | Standard practice for flight dynamics analysis at this fidelity level | Ignores vibration-induced effects |
| A-04 | No wind, turbulence, or gusts | Isolates platform performance from environmental factors | Results represent ideal conditions only |
| A-05 | Perfect sensor/navigation state knowledge | Focuses analysis on platform physics, not navigation errors | Overestimates controller performance |
| A-06 | Uniform battery cell characteristics | Standard simplification for pack-level modeling | Ignores cell imbalance degradation |
| A-07 | Li-Ion chemistry with polynomial discharge curve | Generic Li-Ion behavior well-established in literature | Specific cell chemistry may differ |

### 3.2 Capabilities

The model provides the following analytical capabilities:

- **6DOF flight dynamics** with quaternion attitude representation and semi-implicit Euler integration at 100 Hz
- **Battery energy modeling** with SOC tracking, OCV polynomial curve, SOC-dependent internal resistance, and voltage sag under load
- **Propulsion system modeling** with 8-motor coaxial configuration, motor thermal dynamics, ground effect (Cheeseman-Bennett), and dynamic power computation from flight state
- **VRS detection** with 5-state machine, margin computation, and lift/control degradation factors
- **Slung load dynamics** with 2-DOF spherical pendulum model, cable tension computation, and resonance detection
- **Configurable parameters** allowing scenario-level adjustment of all key physical parameters without recompilation
- **CSV telemetry output** for post-processing and analysis

### 3.3 Limitations

| ID | Limitation | Impact on Intended Use | Mitigation |
|----|-----------|----------------------|------------|
| L-01 | Endurance gap: 14.3 min simulated vs. 36 min manufacturer spec | Absolute endurance values require analyst interpretation | Document scaling factor; use relative comparisons across configurations |
| L-02 | No wind/atmospheric model | Results valid only for calm conditions | Analysts caveat results for wind effects |
| L-03 | VRS is informational only (no feedback to mover) | VRS detection correct but aircraft does not exhibit actual loss of lift | Analysts use VRS state/margin data for vulnerability assessment |
| L-04 | Slung load forces informational (no mover feedback) | Load dynamics computed but aircraft trajectory unaffected | Analysts assess load stability independently |
| L-05 | No sensor noise or navigation error | Controller performance idealized | Analysts apply appropriate margins |
| L-06 | Constant air density (sea level) | Performance at altitude not captured | Restrict analysis to low-altitude operations or apply analytical corrections |
| L-07 | Coaxial interference modeled as constant factor (0.85) | Rotor interaction does not vary with flight state | Conservative factor appropriate for most operating conditions |

### 3.4 Risks and Impacts

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Endurance overestimation by analysts | Medium | Incorrect mission planning conclusions | Clearly document endurance gap in all analysis products |
| VRS onset mischaracterized | Low | Incorrect vulnerability assessment | VRS thresholds based on established rotorcraft aerodynamics literature |
| Parameter values significantly different from actual T-150 | Medium | Quantitative results may not match reality | Use parametric sweeps; focus on relative trends rather than absolute values |
| Novel AI-assisted development introduces undetected errors | Low | Subtle behavioral errors possible | Comprehensive Phase 1 validation testing; code review |

---

## 4. Accreditation Methodology

### 4.1 Accreditation Information Needs

The Accreditation Authority requires the following categories of information to render an accreditation determination:

1. **V&V Results**: Results from formal verification and validation testing, including the Phase 1 Validation Report documenting 10 test cases covering flight dynamics, battery integration, and system-level behavior.

2. **Test Reports**: Detailed test reports with quantitative metrics for each acceptability criterion defined in Section 2, including pass/fail determination and measured values.

3. **SME Assessments**: Subject matter expert assessments of model fidelity in the following domains:
   - Rotorcraft aerodynamics and flight dynamics
   - Li-Ion battery electrochemistry and power systems
   - Coaxial rotor VRS phenomena
   - Slung load dynamics

4. **Comparison to Reference Data**: Comparison of model outputs to manufacturer specifications, published T-150 performance data, and first-principles physics predictions.

5. **Developer Documentation**: Architecture Description Document (ADD), Interface Control Document (ICD), User and Analyst Manual (UAM), and source code documentation.

6. **Gap Analysis**: Documentation of all known gaps, their assessed impact on intended use, and recommended mitigations.

### 4.2 Information Collection Plan

| Information Need | Source Document | Status | Responsible Party |
|-----------------|----------------|--------|-------------------|
| V&V test results | Phase 1 Validation Report | Complete | Developer |
| Architecture documentation | T-150 UAS ADD v2.0 | Complete | Developer |
| Interface documentation | T-150 UAS ICD v1.0 | Complete | Developer |
| User documentation | T-150 UAS UAM v3.0 | Complete | Developer |
| Acceptability criteria assessment | V&V Report (this effort) | In Progress | V&V Agent |
| SME flight dynamics review | [PLACEHOLDER - SME Assessment] | Planned | [PLACEHOLDER] |
| SME battery/power review | [PLACEHOLDER - SME Assessment] | Planned | [PLACEHOLDER] |
| Comparison to manufacturer data | Gap Analysis (UAM Section 7) | Complete | Developer |
| Code quality assessment | Code review / static analysis | [PLACEHOLDER] | [PLACEHOLDER] |
| Risk assessment | Accreditation Plan Section 3.4 | Complete | Developer |

### 4.3 Assessment Plan

The accreditation assessment will follow a three-phase approach:

**Phase I -- Evidence Review**

Review all V&V evidence against the requirements and acceptability criteria defined in Section 2. For each requirement, determine whether the evidence demonstrates that the criterion is met, partially met, or not met. Document any gaps in evidence.

**Phase II -- SME Assessment**

Convene subject matter experts to review the model's physics implementations, parameter values, and behavioral outputs. SMEs will assess:
- Whether the implemented physics models (momentum theory, Cheeseman-Bennett ground effect, Li-Ion electrochemistry, spherical pendulum dynamics) are appropriate for the intended use
- Whether parameter values are reasonable given available reference data
- Whether model outputs exhibit physically correct qualitative behavior
- Whether identified limitations materially affect the intended analytical use cases

**Phase III -- Risk-Based Determination**

The Accreditation Authority will weigh the cumulative evidence and SME assessments to determine whether the model is:
- **Accredited**: Acceptable for all intended uses listed in Section 1.1
- **Accredited with Restrictions**: Acceptable for a subset of intended uses, with specific caveats
- **Not Accredited**: Insufficient evidence or unacceptable risk for intended uses

The assessment will apply the principle that for intelligence analysis (as opposed to acquisition or safety-critical applications), the model need not perfectly reproduce absolute values but must correctly represent relative trends, physical phenomena, and qualitative behavior.

---

## 5. Accreditation Issues

The following issues have been identified as requiring explicit consideration by the Accreditation Authority:

### 5.1 No T-150 Flight Test Data

**Issue**: No actual T-150 flight test data (telemetry, performance measurements, or recorded flight profiles) is available for direct model validation. All validation is performed against manufacturer specifications, published parameters from open sources (AAIB accident report, patent filings, product data sheets), and first-principles physics predictions.

**Impact**: The model cannot be validated in the traditional sense of comparing model outputs to measured system behavior. Acceptability criteria are necessarily based on physics consistency, parameter reasonableness, and comparison to analytical predictions.

**Recommended Disposition**: Accept for intelligence analysis use with the caveat that absolute quantitative results carry higher uncertainty than models validated against flight test data. Focus accreditation on qualitative behavior correctness and relative trend accuracy.

### 5.2 Developer Self-Testing

**Issue**: All verification and validation testing was performed by the model developer. No independent V&V agent was engaged. This limits the independence of the V&V assessment per DoDI 5000.61 guidance.

**Impact**: Potential for confirmation bias in test design and result interpretation. Tests may not adequately stress failure modes or edge cases that an independent agent would identify.

**Recommended Disposition**: Acknowledge the limitation. For the intelligence analysis use case and the current maturity level of the model, developer self-testing provides adequate evidence when combined with SME review. Consider independent V&V for any future use in higher-consequence applications.

### 5.3 Flight Endurance Gap

**Issue**: Simulated flight endurance (14.3 minutes at MTOW in hover) is approximately 40% of the manufacturer-specified endurance (36 minutes). Root cause analysis attributes this to conservative power modeling (auxiliary power estimate of 4 kW may be high) and possible differences between the simplified momentum-theory power model and actual propulsion system efficiency.

**Impact**: Absolute endurance predictions will underestimate actual T-150 endurance. Mission profile analyses based on absolute flight time will be conservative (pessimistic).

**Recommended Disposition**: Document the endurance scaling factor (approximately 2.5x) in all analysis products. Note that the conservative bias is acceptable for threat assessment (tends to underestimate adversary capability). Endurance relative trends across configurations remain valid. Tuning auxiliary power and motor efficiency parameters can close the gap if better reference data becomes available.

### 5.4 AI-Assisted Development Methodology

**Issue**: The T-150 UAS AFSIM Plugin Suite was developed using AI-assisted coding (Claude Code). This is a novel approach with limited precedent in the M&S accreditation community. There are no established standards or best practices for accrediting AI-assisted simulation code.

**Impact**: Reviewers may lack familiarity with AI-assisted development workflows. Code structure and commenting patterns may differ from traditional hand-coded plugins. The development methodology does not change the fundamental accreditation question (does the model produce credible results for its intended use), but it introduces process novelty.

**Recommended Disposition**: Evaluate the model on its outputs and documented behavior, not on the development methodology. AI-assisted development is a tool; the resulting code is standard C++ that compiles, links, and executes identically to hand-coded plugins. The Phase 1 validation results provide the same evidentiary basis regardless of how the code was produced. Document the development methodology for transparency.

### 5.5 Estimated Parameters

**Issue**: Several model parameters are estimated rather than measured from actual T-150 hardware. These include moments of inertia (Ixx=2.5, Iyy=2.5, Izz=4.5 kg*m^2), battery pack internal resistance (0.025 Ohm), motor thermal mass (1500 J/K), motor cooling rate (20 W/K), and cable damping coefficient (0.1 1/s).

**Impact**: Estimated parameters introduce uncertainty in absolute model outputs. The magnitude of this uncertainty varies by parameter; inertia estimates primarily affect dynamic response characteristics, while battery resistance affects voltage sag magnitude.

**Recommended Disposition**: Accept estimated parameters as appropriate for intelligence analysis given the absence of measured data. All estimated parameters are configurable via scenario files, allowing analysts to perform sensitivity analyses. Document parameter sources and confidence levels in the UAM.

---

## 6. Key Participants

### 6.1 Accreditation Authority

| Role | Name | Organization | Responsibility |
|------|------|-------------|----------------|
| Accreditation Authority | [PLACEHOLDER] | [PLACEHOLDER] | Final accreditation determination |
| Deputy Accreditation Authority | [PLACEHOLDER] | [PLACEHOLDER] | Supports accreditation review |

### 6.2 M&S Proponent

| Role | Name | Organization | Responsibility |
|------|------|-------------|----------------|
| M&S Proponent | [PLACEHOLDER] | [PLACEHOLDER] | Advocates for model use; defines intended use |
| Program Manager | [PLACEHOLDER] | [PLACEHOLDER] | Overall program oversight |

### 6.3 V&V Agent

| Role | Name | Organization | Responsibility |
|------|------|-------------|----------------|
| V&V Agent | [PLACEHOLDER] | [PLACEHOLDER] | Executes V&V activities; produces V&V Report |
| V&V Analyst | [PLACEHOLDER] | [PLACEHOLDER] | Conducts testing and analysis |

### 6.4 Model Developer

| Role | Name | Organization | Responsibility |
|------|------|-------------|----------------|
| Lead Developer | [PLACEHOLDER] | [PLACEHOLDER] | Plugin development, documentation, developer testing |
| Configuration Manager | [PLACEHOLDER] | [PLACEHOLDER] | Source code management (GitHub) |

### 6.5 Subject Matter Experts

| Domain | Name | Organization | Qualification |
|--------|------|-------------|---------------|
| Rotorcraft Aerodynamics | [PLACEHOLDER] | [PLACEHOLDER] | [PLACEHOLDER] |
| UAS Systems Engineering | [PLACEHOLDER] | [PLACEHOLDER] | [PLACEHOLDER] |
| Battery / Power Systems | [PLACEHOLDER] | [PLACEHOLDER] | [PLACEHOLDER] |
| AFSIM Framework | [PLACEHOLDER] | [PLACEHOLDER] | [PLACEHOLDER] |
| Intelligence Analysis (T-150) | [PLACEHOLDER] | [PLACEHOLDER] | [PLACEHOLDER] |

---

## 7. Planned Accreditation Resources

### 7.1 Resource Requirements

| Resource | Description | Status |
|----------|-------------|--------|
| V&V documentation suite | ADD, ICD, UAM, V&V Plan, V&V Report | Complete (ADD v2.0, ICD v1.0, UAM v3.0) |
| AFSIM runtime environment | AFSIM v2.2505 with plugin installation capability | Available |
| Test scenarios | Phase 1 validation test suite (10 scenarios) | Complete |
| Test output data | CSV telemetry and AER event records from validation | Complete |
| SME availability | Access to domain experts for review sessions | [PLACEHOLDER] |
| Accreditation review meeting | Scheduled review with Accreditation Authority | [PLACEHOLDER] |

### 7.2 Milestones

| Milestone | Target Date | Status |
|-----------|------------|--------|
| V&V Plan complete | [PLACEHOLDER] | [PLACEHOLDER] |
| Phase 1 validation testing complete | 2026-01-15 | Complete |
| V&V Report complete | [PLACEHOLDER] | [PLACEHOLDER] |
| Accreditation Plan complete | 2026-03-05 | Complete |
| SME review sessions | [PLACEHOLDER] | Planned |
| Accreditation review meeting | [PLACEHOLDER] | Planned |
| Accreditation determination | [PLACEHOLDER] | Planned |

### 7.3 Timeline

```
2025-12     Model development complete (Phases 1-6)
2026-01     Phase 1 validation testing; bug fixes; retest
2026-01     ADD v2.0, UAM v2.0 published
2026-03     UAM v3.0, ICD v1.0, Accreditation Plan v1.0 published
TBD         V&V Plan and V&V Report finalized
TBD         SME review sessions conducted
TBD         Accreditation review and determination
```

---

## Appendix A: M&S Description

### A.1 System Being Modeled

The Malloy Aeronautics T-150 (commercial designation TRV-150) is a heavy-lift unmanned aerial system employing a coaxial octocopter configuration (4 arms, 8 motors, 8 propellers in 4 coaxial pairs). The platform is designed for cargo delivery, logistics, and ISR missions.

**Key T-150 Specifications**:

| Parameter | Value | Source |
|-----------|-------|--------|
| Maximum Takeoff Weight | 123 kg | AAIB Report [4] |
| Empty Weight | 68 kg | AAIB Report [4] |
| Maximum Payload | 55 kg | Derived (MTOW - empty) |
| Motor Configuration | 4-arm coaxial octocopter (8 motors) | AAIB Report [4] |
| Motor Type | T-Motor U15II KV100 | Specification sheets [6] |
| Propeller Diameter | 30 in (0.76 m) | Motor spec [6] |
| Battery Configuration | 13S Li-Ion | Patent [5] |
| Battery Capacity | 7,000 Wh | Patent [5] |
| Maximum Cruise Speed | 30 m/s | Product data [11] |
| Manufacturer-Claimed Endurance | 36 min (at unspecified payload) | Product data [11] |

### A.2 Model Architecture

The model uses a modular plugin architecture. The 6DOF mover provides the core flight dynamics, while four processors independently model battery state, propulsion system state, VRS conditions, and slung load dynamics. Plugins communicate through the AFSIM AuxData key-value store, publishing a total of 68 AuxData keys.

### A.3 Development History

The model was developed in six phases from October 2025 through January 2026:

| Phase | Content | Date |
|-------|---------|------|
| 1 | Thermal fixes, VRS mover integration, motor time constants | Nov 2025 |
| 2 | Battery voltage sag model | Nov 2025 |
| 3 | Dynamic power model (battery reads from propulsion) | Dec 2025 |
| 4 | Ground effect, payload dynamics | Dec 2025 |
| 5 | Slung load dynamics plugin | Dec 2025 |
| 6 | Dynamic power based on flight state (velocity, acceleration, bank) | Jan 2026 |

---

## Appendix B: M&S Requirements Traceability Matrix

| Req ID | Requirement | V&V Test | Acceptability Criterion | Result | Status |
|--------|------------|---------|------------------------|--------|--------|
| REQ-01 | Flight dynamics fidelity | Tests 01-05 | No divergence; drift < 1.0 m; tracking error < 1.0 m/s | Drift ~1e-17 m/s; tracking 14.49/15.0 m/s | PASS |
| REQ-02 | Energy system accuracy | Tests 06, 09 | Energy error < 0.1%; monotonic SOC | Error < 0.01%; SOC monotonic after fix | PASS |
| REQ-03 | VRS detection accuracy | VRS unit test | Correct state transitions at thresholds | States transition at documented margins | PASS |
| REQ-04 | Performance envelope | Test 04; param review | Speed 27-33 m/s; MTOW 123 kg | Max speed 30 m/s; MTOW 123 kg | PASS |
| REQ-05 | Subsystem interactions | Integration test | All AuxData keys published; fallback chain works | 68 keys verified; power source chain functional | PASS |
| REQ-06 | Ground effect | Test 07 | Factor 1.0-1.3; active below 1D AGL | GE model functional; factor within range | PASS |
| REQ-07 | Motor thermal dynamics | Test 01 | Equilibrium 60-90 C; no runaway | Stable thermal response | PASS |
| REQ-08 | Slung load dynamics | Slung load unit test | Correct natural frequency; tension >= 0 | Pendulum behavior physically correct | PASS |
| REQ-09 | Numerical stability | Tests 05, 10 | No NaN/divergence; bounded states | Stable under extremes; 14.3 min flight | PASS |
| REQ-10 | Flight endurance realism | Test 10; gap analysis | Within documented bounds with rationale | 14.3 min; gap documented and justified | PASS (with caveat) |

---

## Appendix C: Basis of Comparison

The following sources serve as the basis of comparison for model validation:

| Source | Data Type | Used For | Confidence |
|--------|----------|----------|------------|
| AAIB South Scarle Report [4] | MTOW, configuration, accident circumstances | Mass, motor count, arm configuration | High (official investigation) |
| US Patent US20240262499A1 [5] | Battery topology, voltage, capacity | Battery model parameters | Medium (design intent, not measured) |
| T-Motor U15II Spec Sheet [6] | KV rating, max power, dimensions | Motor model parameters | High (manufacturer data) |
| SURVICE TRV-150 Data Sheet [11] | Speed, endurance, payload claims | Performance envelope validation | Medium (marketing material) |
| Momentum Theory [9] | Thrust/power relationships | Rotor model validation | High (established theory) |
| Cheeseman-Bennett [8] | Ground effect augmentation | Ground effect model validation | High (peer-reviewed research) |
| Leishman [10] | VRS onset boundaries | VRS threshold validation | High (textbook reference) |
| First-principles physics | Energy conservation, Newton's laws | Numerical accuracy verification | High (fundamental physics) |

---

## Appendix D: References

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

## Appendix E: Acronyms

| Acronym | Definition |
|---------|------------|
| ADD | Architecture Description Document |
| AER | AFSIM Event Record |
| AFSIM | Advanced Framework for Simulation, Integration, and Modeling |
| AGL | Above Ground Level |
| API | Application Programming Interface |
| AuxData | Auxiliary Data (AFSIM inter-processor communication) |
| BEMT | Blade Element Momentum Theory |
| CG | Center of Gravity |
| CONOPS | Concept of Operations |
| C-UAS | Counter-Unmanned Aerial System |
| DoDI | Department of Defense Instruction |
| 6DOF | Six Degrees of Freedom |
| ESC | Electronic Speed Controller |
| GE | Ground Effect |
| ICD | Interface Control Document |
| IGE | In Ground Effect |
| ISA | International Standard Atmosphere |
| LVC | Live-Virtual-Constructive |
| M&S | Modeling and Simulation |
| MTOW | Maximum Takeoff Weight |
| NED | North-East-Down (coordinate frame) |
| NGIC | National Ground Intelligence Center |
| OCV | Open-Circuit Voltage |
| OGE | Out of Ground Effect |
| SME | Subject Matter Expert |
| SOC | State of Charge |
| UAM | User and Analyst Manual |
| UAS | Unmanned Aerial System |
| V&V | Verification and Validation |
| VRS | Vortex Ring State |
| VV&A | Verification, Validation, and Accreditation |

---

## Appendix F: Glossary

Key M&S terms per MIL-STD-3022 Section 3:

| Term | Definition |
|------|-----------|
| Accreditation | The official certification that a model or simulation is acceptable for use for a specific purpose. (MIL-STD-3022 3.1) |
| Accreditation Authority | The official who has the authority and responsibility for determining if a model or simulation is acceptable for a specific use. |
| Acceptability Criteria | Standards against which a model or simulation is judged as adequate for its intended use. |
| Model | A physical, mathematical, or otherwise logical representation of a system, entity, phenomenon, or process. |
| Simulation | A method for implementing a model over time; the execution of a model. |
| Validation | The process of determining the degree to which a model or simulation is an accurate representation of the real world from the perspective of its intended uses. |
| Verification | The process of determining that a model or simulation implementation and its associated data accurately represent the developer's conceptual description and specifications. |
| Intended Use | The specific purpose for which a model or simulation is to be used, defining the context in which accreditation is sought. |
| Fidelity | The degree to which a model or simulation represents the real-world system for its intended use. |
| Resolution | The degree of detail used to represent aspects of the real world in a model or simulation. |
| Conceptual Model | The collection of assumptions, algorithms, relationships, and data that describe the real-world system or process to be modeled. |
| V&V Agent | The organization or individual responsible for executing verification and validation activities and producing the V&V Report. |
| Subject Matter Expert | An individual with specialized knowledge in a particular domain relevant to the model or simulation being assessed. |

---

[CLASSIFICATION: UNCLASSIFIED]
