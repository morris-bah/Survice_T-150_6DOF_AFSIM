# T-150 UAS Simulation Architecture

## Overview

This document describes the architecture of the T-150/TRV-150 Heavy-Lift UAS simulation built using AFSIM (Advanced Framework for Simulation, Integration, and Modeling).

---

## 1. Model Architecture

### 1.1 Platform Definition Hierarchy

```
T150_UAS (Platform Type)
├── WSF_PLATFORM (Base class)
├── T150_MOVER (WSF_FULL_ROTORCRAFT_MOVER)
│   ├── rotorcraft_kinematics
│   │   ├── performance_limits (operational envelope)
│   │   ├── global_limits (physical "do not exceed")
│   │   ├── run_time_gains (control system tuning)
│   │   ├── feedback_control_system (PID controllers)
│   │   └── acceleration_table (deceleration profiles)
│   └── performance_tables (altitude/speed variable performance)
└── T150_RADAR_SIG (Radar cross-section signature)
```

### 1.2 File Structure

| File | Purpose |
|------|---------|
| `platforms/t150_mover.afsim` | WSF_FULL_ROTORCRAFT_MOVER definition with flight dynamics |
| `platforms/t150_platform.afsim` | Platform type combining mover + RCS signature |
| `scenarios/t150_flight_demo.afsim` | Mission scenario with routes and energy tracking |

### 1.3 Key Design Decisions

1. **WSF_FULL_ROTORCRAFT_MOVER Selection**: Chosen over WSF_BASIC_ROTORCRAFT_MOVER for:
   - Table-driven performance varying with altitude and airspeed
   - Mode-based performance (DEFAULT, HOVER)
   - More realistic rotorcraft dynamics

2. **Parametric vs Physics-Based**: AFSIM uses parametric motion models rather than full 6-DOF physics. Performance is defined by tables and limits rather than computed from forces/moments.

3. **Energy Tracking Approach**: Time-based energy estimation using execute blocks rather than real-time processor due to script variable scope limitations between platform_type and scenario levels.

---

## 2. Simulation Architecture

### 2.1 Scenario Components

```
t150_flight_demo.afsim
├── Platform Instance (t150_drone)
│   └── Route (67 waypoints)
│       ├── Phase 1: Takeoff/Climb (0m → 500m AGL)
│       ├── Phase 2: Straight Line (2km)
│       ├── Phase 3: Sawtooth Pattern (8 segments)
│       ├── Phase 4: Square Wave Pattern (6 cycles)
│       ├── Phase 5: Sine Wave Pattern (2 cycles)
│       ├── Phase 6: Cycloid Loops (4 loops)
│       └── Phase 7: Return & Land
├── Script Variables (energy tracking)
├── Execute Blocks (phase completion logging)
└── Event Pipe (output recording)
```

### 2.2 Route Construction

Routes in AFSIM define waypoint sequences. For rotorcraft:
- First waypoint = platform spawn location
- Subsequent waypoints = destinations to fly toward
- Waypoint attributes: position, altitude, speed, end-of-route action

**Critical Learning**: Platform `position` and `altitude` commands are ignored when a mover is defined. The route's first waypoint determines starting location.

### 2.3 Energy Tracking Implementation

```
Energy Model (Time-Based Estimation):
├── Climb Power: 22 kW
├── Cruise Power: 16-17 kW (speed dependent)
├── Descent Power: 12 kW
├── Hover Power: 18 kW
└── Battery Capacity: 7000 Wh

Phase Energy = Power × Duration
Duration = Distance / Speed (or Altitude / Climb Rate)
```

---

## 3. AFSIM Calculation Methods

### 3.1 Mover Kinematics

WSF_FULL_ROTORCRAFT_MOVER uses a kinematic approach:

1. **Desired State**: Route waypoints define target position, altitude, speed
2. **Error Computation**: Difference between current and desired state
3. **Gain Application**: run_time_gains convert errors to rate commands
4. **PID Control**: feedback_control_system smooths acceleration commands
5. **Limit Enforcement**: global_limits and performance_limits constrain motion
6. **Attitude Coupling**: Bank angle couples to turn rate; pitch couples to climb/acceleration

### 3.2 Performance Table Lookups

Tables provide variable performance based on:
- **Independent Variables**: altitude, airspeed, (weight/mass)
- **Dependent Variables**: available acceleration, climb rate, bank angle, etc.

Example: At 2000m altitude, maximum climb rate degrades from 5 m/s to 3.5 m/s.

### 3.3 Waypoint Switching

Two modes available:
- **switch_on_passing**: Platform passes waypoint, then switches (may overshoot)
- **switch_on_approach**: Platform anticipates turn, switches early

Current implementation uses `switch_on_passing` with tuned deceleration gains.

### 3.4 Deceleration to Hover

The `acceleration_table` defines deceleration profile from cruise to hover:
- Works in reverse: high decel initially, tapering as speed decreases
- `deceleration_table_adjustment_gain` scales the table values
- `hover_damping_deceleration_gain` provides final positioning damping

---

## 4. Data Flow

```
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│  Scenario File  │────▶│  AFSIM Mission   │────▶│  Event Record   │
│  (.afsim)       │     │  Executable      │     │  (.aer file)    │
└─────────────────┘     └──────────────────┘     └─────────────────┘
                               │
                               ▼
                        ┌──────────────────┐
                        │  Console Output  │
                        │  (Log messages)  │
                        └──────────────────┘
                               │
                               ▼
                        ┌──────────────────┐
                        │  Warlock/Mystic  │
                        │  (Visualization) │
                        └──────────────────┘
```

---

## 5. Configuration Parameters

### 5.1 T-150 Specific Values

| Parameter | Value | Source |
|-----------|-------|--------|
| Max Cruise Speed | 30 m/s | Manufacturer spec |
| Max Climb Rate | 5 m/s (operational) | Battery C-rating limit |
| Max Descent Rate | 3 m/s (operational) | VRS avoidance |
| Max Bank Angle | 35-45 deg (operational) | Stability margin |
| Service Ceiling | 3000 m | Estimated |
| MTOW | 123 kg | AAIB report |

### 5.2 Tuning Parameters

| Parameter | Current Value | Effect |
|-----------|---------------|--------|
| waypoint_proportional_navigation_gain | 50.0 | Path following aggressiveness |
| waypoint_velocity_pursuit_gain | 10.0 | Lead pursuit weighting |
| deceleration_table_adjustment_gain | 1.8 | Deceleration strength |
| hover_damping_deceleration_gain | 0.90 | Final approach damping |
| hover_position_nulling_gain | 0.08 | Position hold tightness |

---

## 6. Extensibility Points

### 6.1 Adding New Flight Patterns
Add waypoints to the route section following the pattern format (see existing sawtooth, square wave examples).

### 6.2 Modifying Performance
Edit `t150_mover.afsim`:
- `performance_limits` for operational envelope
- `global_limits` for physical constraints
- Performance tables for altitude/speed effects

### 6.3 Adding Sensors/Weapons
Add to platform_type definition:
```
sensor <name> <sensor_type>
   ...
end_sensor
```

### 6.4 Real-Time Energy Tracking
Requires custom C++ plugin (see `docs/plugin_enhancement_ideas.md`) to access mover state and compute power draw in real-time.

---

## 7. Known Limitations

1. **Energy Model**: Time-based estimates, not real-time physics
2. **Terrain**: No terrain model; assumes flat surface
3. **Wind**: No atmospheric wind effects
4. **Thermal**: No motor/battery temperature modeling
5. **Waypoint Overshoot**: Rotorcraft may overshoot tight waypoints due to momentum

---

## References

- AFSIM Documentation: `/opt/afsim2.2505/documentation/`
- WSF_FULL_ROTORCRAFT_MOVER: `docs/wsf_full_rotorcraft_mover.rst`
- Route Commands: `docs/route.rst`
- Platform Commands: `docs/platform.rst`
