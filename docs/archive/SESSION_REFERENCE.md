# Session Reference Document

> **ARCHIVED**: This document contains historical session context from November 2025.
> For current project state, see `README.md` and `ENHANCEMENT_ROADMAP.md`.
> For physics documentation, see `T150_PHYSICS_REFERENCE.md`.

## Purpose
This document provides context for future Claude sessions working on this project. It captures key decisions, lessons learned, and the current state of the simulation.

---

## Project Overview

**Project**: T-150/TRV-150 Heavy-Lift UAS Simulation
**AFSIM Version**: 2.2505.1
**Location**: `/home/shepard/afsim_projects/project005_t150_uas`
**Repository**: `git@github.com:DSGarrett/project005_t150_uas.git`

---

## Current State (as of 2025-11-26)

### Working Features
- WSF_FULL_ROTORCRAFT_MOVER with tuned flight dynamics
- Ground-level takeoff with climb to altitude
- Multiple flight patterns (sawtooth, square wave, sine, cycloid)
- Staged landing approach with touchdown
- Time-based energy tracking per phase
- RCS signature for radar modeling
- Comprehensive documentation

### Known Behaviors
- Some waypoint overshoot on tight turns (expected with switch_on_passing)
- Landing requires slight overshoot correction (normal for momentum)
- Mission exceeds battery capacity by 7.5% (intentional for testing)

---

## Key Technical Decisions

### 1. Mover Selection
**Decision**: Use WSF_FULL_ROTORCRAFT_MOVER instead of WSF_BASIC_ROTORCRAFT_MOVER
**Reason**: Provides table-driven performance that varies with altitude/speed

### 2. Energy Tracking Approach
**Decision**: Time-based estimates in execute blocks instead of real-time processor
**Reason**: Script variables in platform_type are not accessible from scenario-level processors. Real-time tracking would require C++ plugin.

### 3. Ground Start Implementation
**Decision**: First route waypoint at 0m AGL serves as spawn point
**Reason**: Platform `position` and `altitude` commands are ignored when mover is defined. Route's first waypoint determines starting location.

### 4. Landing Approach
**Decision**: Three-waypoint staged descent (cruise alt → 100m → ground)
**Reason**: Single waypoint caused deceleration issues. Staged approach provides smoother transition.

---

## Critical Lessons Learned

### AFSIM Behavior
1. **Platform position vs Route**: When a mover is defined, platform-level position/altitude commands are IGNORED. Route controls position.

2. **Coincident Waypoints**: AFSIM rejects waypoints at same lat/lon even with different altitudes. Must offset waypoints slightly.

3. **Script Variable Scope**: Script variables defined at platform_type level are not accessible from scenario-level execute blocks or processors.

4. **initial_conditions Block**: Documented for WSF_BASIC_ROTORCRAFT_MOVER but not available for editing in WSF_FULL_ROTORCRAFT_MOVER at runtime.

### Tuning Insights
1. **Navigation Gains**: waypoint_proportional_navigation_gain of 50.0 and velocity_pursuit_gain of 10.0 work well for this platform

2. **Deceleration**: deceleration_table_adjustment_gain of 1.8 provides adequate approach deceleration

3. **Pattern Speed**: Square wave requires slower speed (10 m/s) for 90-degree turns; sawtooth can handle 18 m/s

---

## File Structure

```
project005_t150_uas/
├── README.md                      # Project overview
├── platforms/
│   ├── t150_mover.afsim           # Flight dynamics definition
│   └── t150_platform.afsim        # Platform type with RCS
├── scenarios/
│   └── t150_flight_demo.afsim     # Main demonstration scenario
├── output/                        # Simulation outputs (.log, .aer)
├── plugins/                       # Custom C++ AFSIM plugins
│   ├── README.md                  # Plugin documentation
│   ├── build.sh                   # Build script
│   ├── t150_battery/              # Battery SOC processor
│   ├── t150_vrs/                  # VRS detection processor
│   └── t150_propulsion/           # Electric propulsion processor
└── docs/
    ├── architecture.md            # Technical architecture
    ├── analysis_summary.md        # Battery/route analysis results
    ├── future_enhancements.md     # Roadmap for improvements
    ├── uas_adaptation_guide.md    # How to adapt for other UAS
    ├── plugin_enhancement_ideas.md # C++ plugin concepts
    └── SESSION_REFERENCE.md       # This file
```

---

## Key Parameters

### T-150 Specifications Used
| Parameter | Value |
|-----------|-------|
| MTOW | 123 kg |
| Max Payload | 68 kg |
| Max Cruise Speed | 30 m/s |
| Max Climb Rate | 5 m/s (operational) |
| Max Descent Rate | 3 m/s (VRS avoidance) |
| Battery Capacity | 7000 Wh |
| Hover Power | 18 kW |
| Cruise Power | 17 kW |

### Tuned Control Gains
| Gain | Value |
|------|-------|
| waypoint_proportional_navigation_gain | 50.0 |
| waypoint_velocity_pursuit_gain | 10.0 |
| deceleration_table_adjustment_gain | 1.8 |
| hover_damping_deceleration_gain | 0.90 |
| hover_position_nulling_gain | 0.08 |

---

## Running the Simulation

### Command Line
```bash
cd /home/shepard/afsim_projects/project005_t150_uas
/opt/afsim2.2505/bin/mission scenarios/t150_flight_demo.afsim
```

### With Visualization
```bash
DISPLAY=:0 /opt/afsim2.2505/bin/warlock scenarios/t150_flight_demo.afsim
```

### Post-Processing
```bash
DISPLAY=:0 /opt/afsim2.2505/bin/mystic output/t150_flight_demo.aer
```

---

## Common Tasks for Future Sessions

### 1. Add New Flight Pattern
- Edit `scenarios/t150_flight_demo.afsim`
- Add waypoints in route section between existing phases
- Update execute blocks for energy tracking if needed
- Adjust end_time if mission duration changes

### 2. Change Platform Performance
- Edit `platforms/t150_mover.afsim`
- Modify `performance_limits` for operational values
- Modify `global_limits` for physical constraints
- Update performance tables for altitude/speed effects

### 3. Implement Real-Time Energy
- Requires C++ plugin development
- See `docs/plugin_enhancement_ideas.md` for design
- Plugin would be placed in AFSIM plugins directory

### 4. Adapt for Different UAS
- Follow `docs/uas_adaptation_guide.md`
- Copy and rename mover/platform files
- Update performance parameters
- Tune control gains as needed

---

## Reference Documentation

### AFSIM Docs (on system)
- `/opt/afsim2.2505/documentation/`
- Key files:
  - `docs/wsf_full_rotorcraft_mover.rst`
  - `docs/wsf_basic_rotorcraft_mover.rst`
  - `docs/route.rst`
  - `docs/platform.rst`

### T-150 Source Data (referenced)
- AAIB Accident Investigation Report (South Scarle, 2023)
- US Patent US20240262499A1 (Battery topology)
- T-Motor U15II Specifications
- FAA Aircraft Registry (TRV-150)

---

## Contact/Attribution

Project maintained as part of AFSIM Projects collection.
GitHub: `git@github.com:DSGarrett/project005_t150_uas.git`

---

## Version History

| Date | Changes |
|------|---------|
| 2025-11-25 | Initial creation with flight dynamics and energy tracking |
| 2025-11-26 | Fixed ground-start takeoff; fixed landing sequence; added documentation suite |
| 2025-11-27 | Implemented C++ plugins: Battery SOC, VRS detection, Electric propulsion |

---

## Custom Plugins (New)

Three custom C++ plugins have been implemented in the `plugins/` directory:

### T150_BATTERY_PROCESSOR
Real-time battery state-of-charge modeling with:
- Peukert effect (capacity reduction at high discharge)
- Temperature effects on capacity
- Regenerative energy recovery
- Voltage curve modeling

### T150_VRS_PROCESSOR
Vortex Ring State detection with:
- State machine (CLEAR/CAUTION/ONSET/DEVELOPED/RECOVERY)
- Lift and control authority degradation
- Recovery action recommendations

### T150_PROPULSION_PROCESSOR
Electric propulsion system modeling with:
- Per-motor RPM, current, temperature tracking
- Motor efficiency maps
- Propeller thrust coefficients
- Coaxial interference effects

See `plugins/README.md` for full documentation and usage examples.
