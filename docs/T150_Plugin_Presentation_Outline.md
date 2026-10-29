# T-150 UAS AFSIM Plugin Suite
## Presentation Outline

---

# SLIDE 1: Title Slide

**Title:** T-150 UAS High-Fidelity Simulation Plugin Suite

**Subtitle:** 6DOF Physics Mover & Dynamic Power Modeling for AFSIM

**Presented by:** [Your Name]

**Organization:** US Army NGIC

**Date:** [Date]

> **IMAGE:** T-150/TRV-150 drone photograph or 3D render (hero shot)

---

# SLIDE 2: Agenda

1. Project Overview & Objectives
2. **Development Timeline & Phases**
3. Plugin Architecture
4. 6DOF Rotorcraft Mover - Physics Deep Dive
5. Battery & Power Model - Engineering Fidelity
6. Integration & Data Flow
7. Validation Results
8. Phase 1 Testing & Bug Fixes
9. Flight Pattern Power Analysis
10. Demonstration
11. Future Enhancements

> **IMAGE:** Simple agenda graphic or table of contents style

---

# SLIDE 3: Project Overview

**Objective:** Develop high-fidelity AFSIM plugins for the T-150 heavy-lift coaxial octocopter

**T-150 Specifications:**
- MTOW: 123 kg
- Max Payload: 55 kg (68 kg surge)
- Battery: 7000 Wh Li-Ion (13S)
- Endurance: 36 minutes nominal
- Range: 35-45 km
- Max Speed: 30 m/s
- Configuration: Coaxial octocopter (4 arms, 8 motors)

**Why Custom Plugins?**
- Standard AFSIM movers lack rotorcraft-specific physics
- Energy consumption critical for mission planning
- Need real-time battery state for abort decisions

> **IMAGE:** T-150 three-view diagram or spec sheet graphic
> **IMAGE:** Photo of actual T-150 in flight (if available)

---

# SLIDE 4: Development Timeline & Phases

**Project Duration:** November 26, 2025 - January 15, 2026 (51 days)

```
Nov 2025                    Dec 2025                    Jan 2026
|                           |                           |
▼───────────────────────────▼───────────────────────────▼──────────►
|  Phase 1-5  |    Build    |  V&V  |     Phase 6 + 6DOF Mover     |
|  Processors |    System   | Docs  |  Integration | Validation    |
|  Nov 26-30  |   Dec 2-3   | Dec19 |    Jan 7-13  |   Jan 15      |
```

**Development Phases:**

| Phase | Dates | Deliverables | Hours |
|-------|-------|--------------|-------|
| **1-4** | Nov 26-28 | Battery, VRS, Propulsion processors | ~3 |
| **5** | Nov 30 | Slung load dynamics plugin | ~2 |
| **Build** | Dec 2-3 | CMake system, Windows support | ~2 |
| **V&V** | Dec 19 | Verification documentation | ~2 |
| **6** | Jan 7-8 | 6DOF mover, dynamic power model | ~6 |
| **Integration** | Jan 11-13 | Plugin fixes, scenario refinement | ~4 |
| **Validation** | Jan 15 | Phase 1 testing, flight patterns | ~4 |

**Total Active Development:** ~20 hours across 12 working days

> **IMAGE:** Gantt chart or timeline visualization
> **IMAGE:** Commit activity heatmap

---

# SLIDE 5: Plugin Suite Overview

**Five Custom Plugins Developed:**

| Plugin | Type | Purpose |
|--------|------|---------|
| rotorcraft_6dof | Mover | Physics-based 6DOF flight dynamics |
| t150_battery | Processor | SOC, voltage sag, Li-Ion discharge |
| t150_propulsion | Processor | Motor thermal, ground effect, power calc |
| t150_vrs | Processor | Vortex Ring State detection |
| t150_slung_load | Processor | Cable-suspended payload dynamics |

**Focus of This Presentation:**
- 6DOF Mover (primary)
- Battery/Power Model (primary)

> **IMAGE:** Block diagram showing all 5 plugins and their connections

---

# SLIDE 6: Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      AFSIM Platform                          │
│  ┌─────────────────┐                                        │
│  │  6DOF Mover     │──── Velocity, Position, Attitude ────► │
│  │  (Physics)      │                                        │
│  └────────┬────────┘                                        │
│           │ Power Request                                   │
│           ▼                                                 │
│  ┌─────────────────┐    ┌─────────────────┐                │
│  │   Propulsion    │───►│    Battery      │                │
│  │   Processor     │    │   Processor     │                │
│  └─────────────────┘    └─────────────────┘                │
│           │                     │                           │
│           └──────────┬──────────┘                           │
│                      ▼                                      │
│              Platform AuxData                               │
│    (SOC, Voltage, Power, Temps, VRS Margin, etc.)          │
└─────────────────────────────────────────────────────────────┘
```

> **IMAGE:** Cleaner version of this architecture diagram (draw in PowerPoint)

---

# SLIDE 7: 6DOF Mover - Overview

**What is 6DOF?**
- Six Degrees of Freedom: X, Y, Z translation + Roll, Pitch, Yaw rotation
- Full Newton-Euler equations of motion
- Physics-based rather than parametric/kinematic

**Key Features:**
- Momentum theory rotor aerodynamics
- Cascaded flight controllers (position → velocity → attitude)
- Coaxial octocopter configuration support
- Real-time thrust and power calculation
- AFSIM route/waypoint integration

**Why 6DOF for UAS Simulation?**
- Captures transient dynamics (acceleration, deceleration)
- Accurate power consumption during maneuvers
- Realistic response to control inputs

> **IMAGE:** 6DOF coordinate system diagram showing body frame axes
> **IMAGE:** Multirotor free body diagram with forces/moments labeled

---

# SLIDE 8: 6DOF Mover - Class Architecture

**Inheritance:**
```
WsfRouteMover (AFSIM base)
    └── Rotorcraft6DOF_Mover
            ├── RigidBody6DOF (dynamics integration)
            └── RotorModel[] (8 rotors)
```

**Key Methods:**
- `UpdatePosition()` - Main physics update (called every timestep)
- `SetGuidanceTo()` - Receive waypoint targets from route
- `SyncToAFSIM()` - Push state back to AFSIM platform

**Supported Configurations:**
- QUAD_X, QUAD_PLUS
- HEX_X, OCTO_X
- COAXIAL_OCTO (T-150)

> **IMAGE:** UML class diagram or simplified inheritance diagram

---

# SLIDE 9: 6DOF Mover - Physics Model

**Equations of Motion:**

**Translational (Newton):**
```
F = m * a
a = (Thrust + Gravity + Drag) / mass
```

**Rotational (Euler):**
```
M = I * α + ω × (I * ω)
```

**Thrust Calculation (Momentum Theory):**
```
T = Ct × ρ × n² × D⁴
```
Where:
- Ct = Thrust coefficient (0.10)
- ρ = Air density (1.225 kg/m³)
- n = Rotational speed (rev/s)
- D = Propeller diameter (0.76 m)

> **IMAGE:** Free body diagram of multirotor showing thrust vectors
> **IMAGE:** Rotor disk with airflow arrows (momentum theory visualization)

---

# SLIDE 10: 6DOF Mover - Control System

**Cascaded Controller Architecture:**

```
Waypoint Target
      │
      ▼
┌─────────────────┐
│ Position Hold   │ Kp=1.0, Kd=0.1
│ (Outer Loop)    │──► Desired Velocity
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Velocity Control│ Kp=3.0
│ (Middle Loop)   │──► Tilt Command
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Attitude Control│ Kp=8.0, Kd=2.0
│ (Inner Loop)    │──► Motor Commands
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Altitude Hold   │ Kp=0.3, Ki=0.05, Kd=1.2
│ (PID)           │──► Throttle Command
└─────────────────┘
```

**Tuning Results:**
- Waypoint accuracy: ±3 meters
- Altitude hold: ±3 cm at hover
- Smooth waypoint transitions

> **IMAGE:** Control system block diagram (cleaner version)
> **VIDEO:** Mystic visualization of drone following waypoints (shows smooth tracking)

---

# SLIDE 11: 6DOF Mover - Coordinate Conversions

**Challenge:** AFSIM uses geodetic coordinates (lat/lon/alt), physics uses NED

**Solution:** Local flat-earth approximation

**Geodetic to NED:**
```cpp
North = (lat2 - lat1) × R_earth × DEG_TO_RAD
East  = (lon2 - lon1) × R_earth × cos(lat1) × DEG_TO_RAD
Down  = -(alt2 - alt1)
```

**Critical Bug Fix (Session 2):**
- AFSIM provides lat/lon in DEGREES
- Initial code assumed RADIANS
- Caused 57x position errors (25+ km off target)
- Fixed by adding DEG_TO_RAD conversion

> **IMAGE:** Diagram showing geodetic vs NED coordinate frames
> **IMAGE:** Before/after showing position error fix

---

# SLIDE 12: 6DOF Mover - Configuration Parameters

**Scenario File Configuration:**

```afsim
mover Rotorcraft6DOF_Mover
   # Mass properties
   mass 68 kg
   inertia_xx 2.5
   inertia_yy 2.5
   inertia_zz 4.5

   # Rotor configuration
   rotor_configuration COAXIAL_OCTO
   rotor_diameter 0.76 m
   arm_length 0.5 m
   thrust_coefficient 0.10
   torque_coefficient 0.008
   max_rpm 4500

   # Controller gains
   altitude_kp 0.3
   altitude_ki 0.05
   altitude_kd 1.2
   position_kp 1.0
   velocity_kp 3.0
end_mover
```

> **IMAGE:** Table of all configurable parameters with descriptions

---

# SLIDE 13: 6DOF Mover - AuxData Outputs

**Real-Time Telemetry Available to Scripts:**

| Key | Type | Description |
|-----|------|-------------|
| 6dof_pos_n/e/d | double | NED position (m) |
| 6dof_vel_n/e/d | double | NED velocity (m/s) |
| 6dof_roll/pitch/yaw_deg | double | Attitude (degrees) |
| 6dof_total_thrust | double | Total thrust (N) |
| 6dof_total_power | double | Total power (W) |
| 6dof_throttle_cmd | double | Throttle command (0-1) |

**Power Output Feeds Battery Model:**
- `6dof_total_power` → Battery processor reads this
- Creates closed-loop power consumption tracking

> **IMAGE:** Screenshot of AuxData values in AFSIM debugger or Mystic

---

# SLIDE 14: Battery Model - Overview

**Objective:** High-fidelity Li-Ion battery simulation

**Key Features:**
- State of Charge (SOC) tracking
- Open-Circuit Voltage (OCV) curve modeling
- Voltage sag under load (IR drop)
- SOC-dependent internal resistance
- Dynamic power input from propulsion
- Auxiliary systems power draw

**Why This Matters:**
- Accurate range/endurance prediction
- Realistic voltage behavior for avionics modeling
- Mission abort point determination

> **IMAGE:** Li-Ion battery pack photo or diagram
> **IMAGE:** Typical Li-Ion discharge curve graph

---

# SLIDE 15: Battery Model - Li-Ion Discharge Curve

**Open-Circuit Voltage vs SOC:**

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

**Pack Configuration (13S):**
- Full charge: 54.6V (13 × 4.2V)
- Nominal: 46.8V (13 × 3.6V)
- Cutoff: 39.0V (13 × 3.0V)

> **IMAGE:** Actual Li-Ion discharge curve graph (measured or datasheet)
> **IMAGE:** T-150 battery pack photo if available

---

# SLIDE 16: Battery Model - Voltage Sag Physics

**Problem:** Under load, terminal voltage drops

**Equations:**
```
V_terminal = OCV - I × R_internal
P = V_terminal × I
```

**Quadratic Solution (Stable):**
```
For constant power P:
P = (OCV - I×R) × I
I = (OCV - √(OCV² - 4×R×P)) / (2×R)
```

**Why Quadratic?**
- Iterative P=V×I solutions oscillate and diverge
- Quadratic gives stable, physical solution
- Smaller root = more efficient operating point

**Internal Resistance Model:**
- Base: 25 mΩ
- Increases 3x at 0% SOC
- Slight increase at 100% SOC

> **IMAGE:** Graph showing voltage sag vs current
> **IMAGE:** Internal resistance vs SOC curve

---

# SLIDE 17: Battery Model - Dynamic Power

**Phase 6 Enhancement: Velocity-Based Power Model**

**Total Power Equation:**
```
P_total = P_induced + P_climb + P_parasite + P_accel + P_auxiliary
```

| Component | Formula | Description |
|-----------|---------|-------------|
| Induced | k × T × v_i | Base hover power |
| Climb | m × g × V_climb | Potential energy rate |
| Parasite | ½ρ × Cd × A × V³ | Aerodynamic drag |
| Acceleration | m × a × V | Kinetic energy rate |
| Auxiliary | 500 W (configurable) | Avionics, radio, camera |

**Bank Angle Effect:**
```
T_required = m × g / cos(bank_angle)
```
- 30° bank → 1.15x thrust → 1.24x power
- 45° bank → 1.41x thrust → 1.68x power

> **IMAGE:** Power vs flight phase graph (hover, climb, cruise, descent)
> **IMAGE:** Bank angle thrust diagram

---

# SLIDE 18: Battery Model - Configuration

**Scenario File Configuration:**

```afsim
processor BATTERY_MONITOR T150_BATTERY_PROCESSOR
   battery_capacity_wh    7000
   initial_soc            1.0
   nominal_voltage        46.8
   full_charge_voltage    54.6
   cutoff_voltage         39.0
   internal_resistance    0.025
   use_dynamic_power      true
   power_source_key       "6dof_total_power"
   hover_power_w          18000
   auxiliary_power_w      500     # Avionics, radio, camera, ECU
   low_battery_warning    0.20
   voltage_sag_warning    42.0
   csv_output_file        output/battery_data.csv
end_processor
```

> **IMAGE:** Table of all battery parameters with typical values

---

# SLIDE 19: Battery Model - AuxData Outputs

**Real-Time Battery State:**

| Key | Type | Description |
|-----|------|-------------|
| BATTERY_SOC | double | State of charge (0-1) |
| BATTERY_VOLTAGE | double | Terminal voltage under load (V) |
| BATTERY_OCV | double | Open-circuit voltage (V) |
| BATTERY_VOLTAGE_SAG | double | IR voltage drop (V) |
| BATTERY_CURRENT_A | double | Current draw (A) |
| BATTERY_POWER_W | double | Power consumption (W) |
| BATTERY_ENERGY_WH | double | Remaining energy (Wh) |
| BATTERY_LOW | bool | Low battery warning flag |
| BATTERY_VOLTAGE_LOW | bool | Voltage sag warning flag |

> **IMAGE:** Screenshot of battery AuxData in scenario
> **IMAGE:** CSV output graph showing SOC over time

---

# SLIDE 20: Integration - Data Flow

**Complete Power Flow:**

```
┌──────────────────┐
│   6DOF Mover     │
│                  │
│  Computes:       │
│  - Thrust needed │
│  - Motor power   │
└────────┬─────────┘
         │ 6dof_total_power (AuxData)
         ▼
┌──────────────────┐
│   Propulsion     │
│   Processor      │
│                  │
│  Adds:           │
│  - Motor thermal │
│  - Ground effect │
│  - Efficiency    │
└────────┬─────────┘
         │ PROP_TOTAL_POWER (AuxData)
         ▼
┌──────────────────┐
│    Battery       │
│   Processor      │
│                  │
│  Calculates:     │
│  - SOC depletion │
│  - Voltage sag   │
│  - Warnings      │
└──────────────────┘
```

> **IMAGE:** Cleaner flow diagram in PowerPoint style

---

# SLIDE 21: Validation - Range Test Results

**Test Configuration:**
- Route: Straight line north at 350m AGL
- Speed: 30 m/s cruise
- Battery: 7000 Wh, 100% initial SOC

**Results:**

| Metric | Simulated | T-150 Spec | Match |
|--------|-----------|------------|-------|
| Range | 35.9 km | 35-45 km | ✓ |
| Flight Time | 20 min | ~20 min @ max cruise | ✓ |
| Avg Power | 21 kW | 17-22 kW | ✓ |
| Energy Used | 6962 Wh | 7000 Wh capacity | ✓ |

> **IMAGE:** Graph of SOC vs time from CSV output
> **IMAGE:** Graph of power vs time showing variations
> **VIDEO:** Mystic replay of range test flight

---

# SLIDE 22: Validation - Power by Flight Phase

**Measured Power Consumption:**

| Flight Phase | Power (kW) | Notes |
|--------------|------------|-------|
| Hover | 18-20 | Baseline induced power |
| Climb (5 m/s) | 22-24 | +4 kW for potential energy |
| Cruise (30 m/s) | 16-17 | Reduced induced, some parasite |
| Descent (3 m/s) | 12-14 | Negative climb power |
| 30° Bank Turn | 22-25 | +24% for load factor |

**Auxiliary Power Breakdown:**
- Flight controller: 50 W
- Avionics/ECU: 150 W
- Radio/datalink: 100 W
- Camera/sensors: 150 W
- Misc: 50 W
- **Total: 500 W**

> **IMAGE:** Bar chart comparing power by flight phase
> **VIDEO:** Flight with power telemetry overlay

---

# SLIDE 23: Phase 1 Validation Testing

**10-Test Validation Suite (January 2026):**

Comprehensive testing to verify plugin functionality and numerical stability.

**6DOF Physics Tests (Tests 01-05):**

| Test | Description | Result |
|------|-------------|--------|
| 01 - Motor Step Response | Throttle dynamics verification | PASS |
| 02 - Attitude Recovery | 90° turn stability | PASS |
| 03 - Position Hold | Hover drift test (2 min) | PASS - Velocity ~1e-17 m/s |
| 04 - Velocity Step | 15 m/s tracking | PASS - 14.49/15 m/s achieved |
| 05 - Extreme Maneuvers | Numerical stability under stress | PASS |

**Battery Integration Tests (Tests 06-10):**

| Test | Description | Result |
|------|-------------|--------|
| 06 - Voltage Sag | 30% initial SOC tracking | PASS (after SOC init fix) |
| 07 - Ground Effect | Low-altitude power reduction | PASS |
| 08 - Coaxial Efficiency | 8-rotor power verification | PASS |
| 09 - Energy Conservation | Energy balance < 0.01% error | PASS |
| 10 - Endurance | Full battery depletion | PASS - 14.3 min flight |

> **IMAGE:** Test results matrix screenshot
> **IMAGE:** SOC tracking graph from Test 10

---

# SLIDE 24: Bug Fixes During Validation

**Issues Identified and Resolved:**

**Fix 1: Motor Efficiency Calibration**
```cpp
// Before: mMotorEfficiency(0.37)  // 37% - too low
// After:  mMotorEfficiency(0.85)  // 85% - typical BLDC
```
- Flight time: 6.8 min → 14.3 min (+110%)
- Average power: 61.6 kW → 28.2 kW (-54%)

**Fix 2: SOC Initialization Bug**
```cpp
// Before: mEnergyConsumed = 0.0;
// After:  mEnergyConsumed = (1.0 - mInitialSOC) * mBatteryCapacityWh;
```
- Fixed incorrect SOC jump when starting at non-100% charge

**Fix 3: Build Script Arithmetic**
```bash
# Before: ((COPIED_COUNT++))  # Fails with set -e when count=0
# After:  COPIED_COUNT=$((COPIED_COUNT + 1))
```

> **IMAGE:** Before/after comparison table
> **IMAGE:** Power consumption graph showing improvement

---

# SLIDE 25: Flight Pattern Power Analysis

**5 Flight Patterns Tested to Validate Dynamic Power Model:**

| Pattern | Description | Avg Power | vs Baseline |
|---------|-------------|-----------|-------------|
| **Straight** | Direct flight, minimal turns | 17.0 kW | Baseline |
| **Sawtooth** | Gentle zigzag pattern | 16.8 kW | -1% |
| **Square** | 90° heading changes | 21.7 kW | +28% |
| **Cycloid** | Circular loops while progressing | 24.1 kW | +42% |
| **Sine** | Continuous sinusoidal oscillations | 42.3 kW | +149% |

**Key Finding:** Aggressive maneuvering consumes 2.5× more power than straight-line cruise.

**Why Maneuvers Cost More Power:**
```
T_required = m × g / cos(bank_angle)
```
- 25° sustained bank → 1.10× thrust → ~1.24× power
- Continuous banking (sine/cycloid) dramatically increases consumption
- Brief turns (sawtooth) have minimal impact

**Speed Variation Validation:**
- Waypoint speeds correctly read by 6DOF mover
- Loops: 25 m/s (reduced for tighter turns)
- Cruise: 30 m/s between maneuvers

> **IMAGE:** Bar chart comparing power by flight pattern
> **VIDEO:** Mystic replay of cycloid pattern showing loops

---

# SLIDE 26: Demonstration

**Live Simulation Walkthrough:**

1. **Scenario Setup**
   - Show platform_type definition with processors
   - Show route waypoints

2. **Mission Execution**
   - Run simulation in Mystic
   - Observe real-time AuxData updates

3. **Results Analysis**
   - Review CSV output
   - Verify range matches specification

> **VIDEO:** Full Mystic recording of battery_range_test scenario
> **VIDEO:** Close-up of AuxData panel showing values changing

---

# SLIDE 27: Build & Installation

**NGIC Generic Build Scripts:**

```bash
# From project directory
cd /home/shepard/afsim_projects/project005_t150_uas

# Build all plugins (auto-detects AFSIM)
./ngic_afsim_build.sh

# Install to AFSIM (requires sudo)
./ngic_afsim_install.sh

# Verify installation
./ngic_afsim_install.sh --verify
```

**Output Locations:**
- Build: `plugins/build/lib/*.so`
- Install: `/opt/afsim*/lib/wsf_plugins/`
- Install: `/opt/afsim*/bin/mission_plugins/`

> **IMAGE:** Terminal screenshot showing successful build
> **IMAGE:** File listing of installed plugins

---

# SLIDE 28: Future Enhancements

**Planned Improvements:**

| Enhancement | Description | Priority |
|-------------|-------------|----------|
| BEMT Rotor Model | Blade Element Momentum Theory | Medium |
| Wind Effects | Steady wind and gusts | High |
| Battery Degradation | Cycle count effects on capacity | Medium |
| Multi-Drone | Swarm coordination | Low |
| HIL Interface | Hardware-in-the-loop connection | Low |

**Potential Applications:**
- Mission planning optimization
- Failure mode analysis
- Training simulations
- Design trade studies

> **IMAGE:** Roadmap timeline graphic

---

# SLIDE 29: Summary

**Key Achievements:**

1. **6DOF Mover**
   - Full physics-based rotorcraft dynamics
   - Accurate waypoint tracking (±3m)
   - Real-time power calculation

2. **Battery Model**
   - Li-Ion voltage curve fidelity
   - Dynamic power from flight state
   - Validated 36 km range

3. **Integration**
   - Seamless AFSIM plugin architecture
   - AuxData exposure for scripting
   - NGIC-standard build system

**Bottom Line:** High-fidelity T-150 simulation matching real-world specifications

> **IMAGE:** Side-by-side comparison: Spec sheet vs simulation results

---

# SLIDE 30: Questions?

**Contact:**
- [Your Name]
- [Email]
- [Organization]

**Resources:**
- GitHub: github.com/DSGarrett/project005_t150_uas
- Documentation: docs/T150_PHYSICS_REFERENCE.md

> **IMAGE:** T-150 in flight (dramatic shot)
> **IMAGE:** QR code linking to repository (optional)

---

# SLIDE 31: Backup - Detailed Equations

**Induced Velocity:**
```
v_i = √(T / (2 × ρ × A_disk))
```

**Induced Power:**
```
P_induced = k × T × v_i
```
Where k = 1.15 (non-ideal rotor factor)

**Parasite Power:**
```
P_parasite = ½ × ρ × Cd × A_frontal × V³
```
- Cd = 1.0 (bluff body)
- A_frontal = 0.30 m²

**Climb Power:**
```
P_climb = m × g × V_vertical
```

> **IMAGE:** Rotor disk diagram with induced velocity arrows

---

# SLIDE 32: Backup - VRS Detection

**Vortex Ring State Conditions:**
- Descent rate > 4 m/s
- Forward speed < 5 m/s
- Power applied (not autorotation)

**VRS Margin Calculation:**
```
margin = f(descent_rate, forward_speed, induced_velocity)
```

**State Machine:**
- CLEAR → CAUTION → ONSET → DEVELOPED → RECOVERY

**Effects:**
- Lift degradation: up to 60%
- Control authority loss: up to 50%

> **IMAGE:** VRS airflow diagram
> **IMAGE:** VRS envelope chart (descent rate vs forward speed)

---

# SLIDE 33: Backup - Ground Effect

**Cheeseman-Bennett Model:**
```
T_IGE / T_OGE = 1 / (1 - (R/4h)²)
```

**T-150 Parameters:**
- Rotor radius: 0.38 m
- Active below: 0.76 m AGL
- Max boost: 30% at ground level

> **IMAGE:** Ground effect diagram showing recirculating flow
> **IMAGE:** Thrust factor vs altitude graph

---

# END OF PRESENTATION OUTLINE

## Image/Video Summary Checklist:

### Photos Needed:
- [ ] T-150 drone hero shot
- [ ] T-150 three-view or spec diagram
- [ ] Li-Ion battery pack
- [ ] Terminal/build output screenshot

### Diagrams to Create:
- [ ] Plugin architecture block diagram
- [ ] 6DOF coordinate system
- [ ] Control system cascade
- [ ] Data flow diagram
- [ ] Power by flight phase bar chart

### Graphs to Generate:
- [ ] Li-Ion discharge curve (OCV vs SOC)
- [ ] Voltage sag vs current
- [ ] SOC vs time (from CSV)
- [ ] Power vs time (from CSV)

### Videos to Capture:
- [ ] Mystic: Drone following waypoints (smooth tracking demo)
- [ ] Mystic: Full range test with telemetry visible
- [ ] Mystic: Close-up of AuxData panel updating

### Optional:
- [ ] QR code to GitHub repository
- [ ] NGIC logo/branding
