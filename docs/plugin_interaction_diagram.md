# T-150 UAS Plugin Interaction Diagram

## Overview

This document describes how the 5 custom AFSIM plugins interact with each other
through the AuxData communication layer.

## System Block Diagram

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              AFSIM SIMULATION ENGINE                             │
│                                                                                  │
│  ┌────────────────────────────────────────────────────────────────────────────┐ │
│  │                         AFSIM PLATFORM (T150_UAS)                          │ │
│  │                                                                            │ │
│  │   ┌─────────────┐    Route/Waypoints    ┌─────────────────────────────┐   │ │
│  │   │   AFSIM     │ ──────────────────────>│                             │   │ │
│  │   │   Route     │                        │   ROTORCRAFT_6DOF_MOVER     │   │ │
│  │   │   System    │<──────────────────────│   (Physics Engine)          │   │ │
│  │   └─────────────┘   Position/Velocity    │                             │   │ │
│  │                                          │  ┌─────────────────────┐    │   │ │
│  │                                          │  │ • Rigid Body 6DOF   │    │   │ │
│  │                                          │  │ • 8x Rotor Models   │    │   │ │
│  │                                          │  │ • Flight Controllers│    │   │ │
│  │                                          │  │ • Quaternion Att.   │    │   │ │
│  │                                          │  └─────────────────────┘    │   │ │
│  │                                          └──────────────┬──────────────┘   │ │
│  │                                                         │                   │ │
│  │                                                         │ OUTPUTS:          │ │
│  │                                                         │ • 6dof_total_power│ │
│  │                                                         │ • 6dof_total_thrust│ │
│  │                                                         │ • 6dof_vel_*      │ │
│  │                                                         │ • 6dof_pos_*      │ │
│  │                                                         │ • 6dof_*_deg      │ │
│  │                                                         ▼                   │ │
│  │  ┌──────────────────────────────────────────────────────────────────────┐  │ │
│  │  │                     PLATFORM AUXDATA (Key-Value Store)               │  │ │
│  │  │                                                                      │  │ │
│  │  │  6dof_total_power ─────┬──────────────────────────────────────────┐  │  │ │
│  │  │  6dof_vel_d ───────────┼────────────────┬─────────────────────────┤  │  │ │
│  │  │  6dof_vel_n/e ─────────┼────────────────┼──────────┬──────────────┤  │  │ │
│  │  │  Platform Altitude ────┼────────────────┼──────────┼──────────────┤  │  │ │
│  │  │  Platform Accel ───────┼────────────────┼──────────┼──────────┬───┤  │  │ │
│  │  │                        │                │          │          │   │  │  │ │
│  │  └────────────────────────┼────────────────┼──────────┼──────────┼───┘  │ │
│  │                           │                │          │          │      │ │
│  │                           ▼                ▼          ▼          ▼      │ │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────┐  ┌─────────────────┐  │ │
│  │  │   BATTERY    │  │  PROPULSION  │  │   VRS    │  │   SLUNG LOAD    │  │ │
│  │  │  PROCESSOR   │  │  PROCESSOR   │  │PROCESSOR │  │   PROCESSOR     │  │ │
│  │  │              │  │              │  │          │  │                 │  │ │
│  │  │ ┌──────────┐ │  │ ┌──────────┐ │  │┌────────┐│  │ ┌─────────────┐ │  │ │
│  │  │ │Li-Ion    │ │  │ │Motor     │ │  ││Descent ││  │ │ Spherical   │ │  │ │
│  │  │ │Chemistry │ │  │ │Thermal   │ │  ││Rate    ││  │ │ Pendulum    │ │  │ │
│  │  │ │Model     │ │  │ │Model     │ │  ││Monitor ││  │ │ Dynamics    │ │  │ │
│  │  │ ├──────────┤ │  │ ├──────────┤ │  │├────────┤│  │ ├─────────────┤ │  │ │
│  │  │ │Voltage   │ │  │ │Ground    │ │  ││5-State ││  │ │ Cable       │ │  │ │
│  │  │ │Sag       │ │  │ │Effect    │ │  ││Machine ││  │ │ Tension     │ │  │ │
│  │  │ │Quadratic │ │  │ │Model     │ │  ││        ││  │ │ Calculation │ │  │ │
│  │  │ ├──────────┤ │  │ ├──────────┤ │  │├────────┤│  │ ├─────────────┤ │  │ │
│  │  │ │SOC       │ │  │ │Payload   │ │  ││Lift/   ││  │ │ Resonance   │ │  │ │
│  │  │ │Tracking  │ │  │ │Mass      │ │  ││Control ││  │ │ Detection   │ │  │ │
│  │  │ └──────────┘ │  │ └──────────┘ │  ││Degrade ││  │ └─────────────┘ │  │ │
│  │  └──────┬───────┘  └──────┬───────┘  │└────────┘│  └────────┬────────┘  │ │
│  │         │                 │          └────┬─────┘           │           │ │
│  │         │                 │               │                 │           │ │
│  │         ▼                 ▼               ▼                 ▼           │ │
│  │  ┌──────────────────────────────────────────────────────────────────┐   │ │
│  │  │                     PLATFORM AUXDATA (Outputs)                   │   │ │
│  │  │                                                                  │   │ │
│  │  │  BATTERY_SOC         PROP_TOTAL_POWER    VRS_STATE    SLUNG_ANGLE_X│  │ │
│  │  │  BATTERY_VOLTAGE     PROP_TOTAL_THRUST   VRS_MARGIN   SLUNG_ANGLE_Y│  │ │
│  │  │  BATTERY_CURRENT_A   PROP_MAX_TEMP       VRS_LIFT_*   SLUNG_TENSION│  │ │
│  │  │  BATTERY_POWER_W     PROP_GROUND_EFFECT  VRS_CAUTION  SLUNG_FORCE_*│  │ │
│  │  │  BATTERY_LOW         PROP_ALTITUDE_AGL   VRS_CRITICAL SLUNG_RESONANCE│ │ │
│  │  │  BATTERY_VOLTAGE_SAG PROP_OVERLOADED     VRS_CONTROL_*SLUNG_OVERLOAD│  │ │
│  │  └──────────────────────────────────────────────────────────────────┘   │ │
│  │                                      │                                   │ │
│  └──────────────────────────────────────┼───────────────────────────────────┘ │
│                                         │                                     │
│                                         ▼                                     │
│  ┌──────────────────────────────────────────────────────────────────────────┐ │
│  │                        SCENARIO SCRIPTS                                  │ │
│  │                                                                          │ │
│  │  • Read any AuxData value: t150_drone.BATTERY_SOC                       │ │
│  │  • Trigger actions based on values: if (VRS_MARGIN < 0.5) ...           │ │
│  │  • Log telemetry: write_log soc "," voltage "," power                   │ │
│  │  • Mission control: abort if battery critical                            │ │
│  └──────────────────────────────────────────────────────────────────────────┘ │
│                                                                               │
└───────────────────────────────────────────────────────────────────────────────┘
```

## Data Flow Summary

### Primary Data Flows

| Source | Data | Consumer | Purpose |
|--------|------|----------|---------|
| AFSIM Route | Waypoints, Speed | 6DOF Mover | Navigation targets |
| 6DOF Mover | `6dof_total_power` | Battery Processor | Energy consumption |
| 6DOF Mover | Velocity (vel_d) | VRS Processor | Descent rate monitoring |
| 6DOF Mover | Velocity (vel_n/e) | VRS Processor | Forward speed for VRS calc |
| Platform | Altitude | Propulsion Processor | Ground effect calculation |
| Platform | Acceleration | Slung Load Processor | Pendulum forcing function |

### Processor Independence

Each processor is designed to function independently:

| Processor | If 6DOF Unavailable | Fallback Behavior |
|-----------|---------------------|-------------------|
| Battery | Uses `hover_power_w` | Static power consumption |
| Propulsion | Uses platform velocity | Table-driven estimates |
| VRS | Uses platform velocity | Standard kinematic data |
| Slung Load | Uses platform acceleration | Derived from velocity changes |

## Timing and Update Rates

```
┌─────────────────────────────────────────────────────────────┐
│                    UPDATE RATE HIERARCHY                     │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  50 Hz (20ms)  ─────┐                                       │
│                     │  Slung Load Dynamics                  │
│                     │  (requires fast integration for       │
│                     │   pendulum stability)                 │
│                     │                                       │
│  20 Hz (50ms)  ─────┤                                       │
│                     │  6DOF Mover Physics                   │
│                     │  (flight dynamics, rotor models)      │
│                     │                                       │
│  10 Hz (100ms) ─────┤                                       │
│                     │  Battery Processor                    │
│                     │  Propulsion Processor                 │
│                     │  VRS Processor                        │
│                     │  (subsystem monitoring)               │
│                     │                                       │
│  Variable     ─────┘                                        │
│                     Script Execution                        │
│                     (execute at_interval N sec)             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## Cross-Plugin Dependencies

```
                    ┌─────────────────┐
                    │  6DOF MOVER     │
                    │  (Primary)      │
                    └────────┬────────┘
                             │
            ┌────────────────┼────────────────┐
            │                │                │
            ▼                ▼                ▼
    ┌───────────────┐ ┌───────────┐ ┌─────────────────┐
    │   BATTERY     │ │    VRS    │ │   PROPULSION    │
    │   (Energy)    │ │  (Safety) │ │   (Thermal)     │
    └───────────────┘ └───────────┘ └────────┬────────┘
                                             │
                                             ▼
                                    ┌─────────────────┐
                                    │   SLUNG LOAD    │
                                    │   (Payload)     │
                                    └─────────────────┘

Legend:
─────▶  Data dependency (reads AuxData from)
```

## AuxData Key Naming Convention

| Prefix | Plugin | Example |
|--------|--------|---------|
| `6dof_` | Rotorcraft 6DOF Mover | `6dof_total_power` |
| `BATTERY_` | Battery Processor | `BATTERY_SOC` |
| `PROP_` | Propulsion Processor | `PROP_TOTAL_POWER` |
| `VRS_` | VRS Processor | `VRS_MARGIN` |
| `SLUNG_` | Slung Load Processor | `SLUNG_TENSION` |

---

*Diagram created for T-150 UAS AFSIM Project 005*
*January 2026*
