# UAS Adaptation Guide

## Overview

This guide explains how to adapt the T-150 simulation framework for other drone/UAS platforms. The simulation is built using AFSIM's WSF_FULL_ROTORCRAFT_MOVER and can be modified to model any rotorcraft with appropriate specification data.

---

## 1. Required Platform Data

Before adapting for a new UAS, gather the following specifications:

### 1.1 Physical Characteristics
| Parameter | Example (T-150) | Your UAS |
|-----------|-----------------|----------|
| Configuration | Coaxial Octocopter | |
| MTOW | 123 kg | |
| Empty Weight | ~55 kg | |
| Max Payload | 68 kg | |
| Rotor Span | ~1.8 m | |
| Total Thrust | 240 kg | |

### 1.2 Performance Specifications
| Parameter | Example (T-150) | Your UAS |
|-----------|-----------------|----------|
| Max Cruise Speed | 30 m/s | |
| Max Climb Rate | 5-8 m/s | |
| Max Descent Rate | 3-4 m/s | |
| Max Bank Angle | 35-60 deg | |
| Service Ceiling | 3000 m | |
| Max Turn Rate | 25-45 deg/s | |

### 1.3 Power/Energy Specifications
| Parameter | Example (T-150) | Your UAS |
|-----------|-----------------|----------|
| Battery Chemistry | Li-Ion 13S | |
| Battery Capacity | 7000 Wh | |
| Hover Power | ~18 kW | |
| Cruise Power | ~17 kW | |
| Max Power | ~22 kW | |
| Nominal Voltage | 46.8 V | |

---

## 2. Files to Modify

### 2.1 Mover Definition: `platforms/t150_mover.afsim`

This is the primary file defining flight dynamics. Rename and modify for your platform.

#### Step 1: Copy and Rename
```bash
cp platforms/t150_mover.afsim platforms/your_uas_mover.afsim
```

#### Step 2: Update Mover Name
```
mover YOUR_UAS_MOVER WSF_FULL_ROTORCRAFT_MOVER
```

#### Step 3: Update Performance Limits (Operational Envelope)
```
performance_limits
    available_airspeed             YOUR_CRUISE_SPEED m/s
    available_rate_of_ascent       YOUR_CLIMB_RATE m/s
    available_rate_of_descent      YOUR_DESCENT_RATE m/s
    available_ascent_path_angle    90 deg  # Vertical capable
    available_descent_path_angle   90 deg
    available_altitude             YOUR_CEILING m
    available_turn_rate            YOUR_TURN_RATE deg/s
end_performance_limits
```

#### Step 4: Update Global Limits (Physical Limits)
```
global_limits
    maximum_airspeed               YOUR_VNE m/s
    maximum_crab_speed             YOUR_SIDEWAYS_LIMIT m/s
    maximum_hover_speed            10 m/s  # Definition of hover
    maximum_rate_of_ascent         YOUR_MAX_CLIMB m/s
    maximum_rate_of_descent        YOUR_MAX_DESCENT m/s
    maximum_bank_angle             YOUR_MAX_BANK deg
    maximum_altitude               YOUR_CEILING m
    minimum_altitude               0 m
    maximum_hover_acceleration_magnitude  0.2 g
    maximum_axial_acceleration     YOUR_MAX_ACCEL g
    maximum_axial_deceleration     YOUR_MAX_DECEL g
    maximum_radial_acceleration    YOUR_LATERAL_G g
    maximum_turn_rate              YOUR_MAX_TURN_RATE deg/s
    maximum_yaw_rate               YOUR_YAW_RATE deg/s
    maximum_pitch_rate             100 deg/s
    maximum_roll_rate              100 deg/s
    minimum_turn_radius            1 m  # Can pivot
    maximum_load_factor            YOUR_G_LIMIT
end_global_limits
```

#### Step 5: Update Performance Tables

Modify tables for altitude and speed effects. Example for climb rate:
```
table available_rate_of_ascent maximum_rate_of_ascent_table
    regular_table
        independent_variable altitude
            units m
            0 500 1000 1500 2000 2500 3000
        end_independent_variable
        independent_variable airspeed
            units m/s
            0 10 20 30
        end_independent_variable
        dependent_variable
            units m/s
            # Values decrease with altitude (less excess thrust)
            5.0 5.0 4.5 4.0   # 0m
            4.8 4.8 4.3 3.8   # 500m
            4.5 4.5 4.0 3.5   # 1000m
            # ... adjust for your aircraft
        end_dependent_variable
    end_regular_table
end_table
```

### 2.2 Platform Definition: `platforms/t150_platform.afsim`

#### Step 1: Copy and Rename
```bash
cp platforms/t150_platform.afsim platforms/your_uas_platform.afsim
```

#### Step 2: Update Include and Platform Type
```
include your_uas_mover.afsim

platform_type YOUR_UAS WSF_PLATFORM
    side blue
    icon 4_rotor_good  # Or appropriate icon
    category air

    mover YOUR_UAS_MOVER
    end_mover

    radar_signature YOUR_UAS_RADAR_SIG  # Update if needed
end_platform_type
```

#### Step 3: Update Radar Signature (Optional)
Estimate RCS based on airframe size and materials:
- Small plastic drone: -15 to -5 dBsm
- Medium composite: -5 to 5 dBsm
- Large metal: 5 to 15 dBsm

### 2.3 Scenario File: `scenarios/t150_flight_demo.afsim`

#### Step 1: Copy and Rename
```bash
cp scenarios/t150_flight_demo.afsim scenarios/your_uas_demo.afsim
```

#### Step 2: Update Platform Include
```
include ../platforms/your_uas_platform.afsim
```

#### Step 3: Update Platform Instance
```
platform your_drone YOUR_UAS
    # Route definition
end_platform
```

#### Step 4: Update Energy Tracking

Modify execute blocks with your power consumption values:
```
# Phase 1 Complete: Takeoff/Climb
execute at_time YOUR_CLIMB_TIME sec absolute
    double phase_energy = YOUR_HOVER_POWER * YOUR_CLIMB_TIME / 3600 * 1000;
    # ... rest of logging
end_execute
```

---

## 3. Tuning Parameters

### 3.1 Control System Gains

If the platform oscillates, overshoots, or tracks poorly, adjust `run_time_gains`:

| Gain | Effect | Increase If | Decrease If |
|------|--------|-------------|-------------|
| waypoint_proportional_navigation_gain | Path following | Sluggish tracking | Oscillation |
| waypoint_velocity_pursuit_gain | Lead pursuit | Late turns | Overshoot |
| deceleration_table_adjustment_gain | Stop approach | Overshoot stops | Stops too early |
| hover_damping_deceleration_gain | Final positioning | Drifts past point | Too slow approach |
| hover_position_nulling_gain | Position hold | Wanders | Oscillates |

### 3.2 PID Controllers

For finer control, adjust PID gains:
```
feedback_control_system
    axial_acceleration_pid
        proportional_gain 1.0  # Speed control
        integral_gain     0.05
        derivative_gain   0.1
    end_axial_acceleration_pid
    # Similar for lateral and vertical
end_feedback_control_system
```

### 3.3 Acceleration Table

Defines deceleration profile from cruise to hover:
```
acceleration_table
    delta_time_for_auto_generation  0.5 s
    maximum_speed_for_auto_generation  YOUR_MAX_SPEED m/s
    point time 0 s acceleration 0.3 g end_point   # Initial strong decel
    point time 8 s acceleration 0.2 g end_point   # Moderate
    point time 20 s acceleration 0.15 g end_point # Gentle
    point time 50 s acceleration 0.1 g end_point  # Final approach
end_acceleration_table
```

---

## 4. Route Adaptation

### 4.1 Starting Position

The first waypoint in the route IS the starting position:
```
route
    # Platform spawns here
    position YOUR_START_LAT YOUR_START_LON altitude 0 m agl speed 0 m/s

    # First destination
    position YOUR_CLIMB_LAT YOUR_CLIMB_LON altitude YOUR_CRUISE_ALT m agl speed 5 m/s

    # Continue with mission
end_route
```

### 4.2 Waypoint Spacing

Consider your platform's turn radius and speed:
- Minimum spacing: 2 × turn_radius at that speed
- Sharp turns: Reduce approach speed
- High-speed segments: Increase spacing

### 4.3 Landing Sequence

Use staged descent for smooth landing:
```
# Approach at altitude
position LAND_LAT+offset LAND_LON altitude 500 m agl speed 15 m/s

# Begin descent
position LAND_LAT+small_offset LAND_LON altitude 100 m agl speed 5 m/s

# Final touchdown
position LAND_LAT LAND_LON altitude 0 m agl speed 3 m/s stop
```

---

## 5. Validation Checklist

After adapting, verify:

- [ ] Simulation runs without errors
- [ ] Platform starts at correct position (ground level)
- [ ] Climb rate matches specification
- [ ] Cruise speed achieved and maintained
- [ ] Turns executed without excessive overshoot
- [ ] Altitude maintained during cruise
- [ ] Descent rate controlled (no VRS indication)
- [ ] Landing at designated point
- [ ] Energy consumption reasonable for battery capacity

---

## 6. Common Issues and Solutions

### Issue: Platform starts at wrong altitude
**Solution**: Ensure first route waypoint is at ground level (0 m agl)

### Issue: Platform can't complete route
**Solution**: Check simulation end_time is sufficient; reduce route complexity

### Issue: Excessive turn overshoot
**Solution**: Reduce waypoint speeds; increase deceleration_table_adjustment_gain

### Issue: Sluggish response
**Solution**: Increase waypoint_proportional_navigation_gain

### Issue: Platform oscillates
**Solution**: Decrease gains; check PID derivative terms

### Issue: Energy calculations incorrect
**Solution**: Verify power consumption values; check phase timing in execute blocks

---

## 7. Quick Reference: Parameter Scaling

For rough adaptation between platforms:

| Parameter | Scaling Factor |
|-----------|----------------|
| Max Speed | Direct from spec |
| Climb Rate | Proportional to thrust/weight |
| Turn Rate | Inversely proportional to wingspan |
| Deceleration | Similar to climb rate |
| Hover Power | Proportional to weight |
| Cruise Power | Weight × speed² relationship |

---

## 8. Example: Adapting for DJI Matrice 300

```
# Quick parameter mapping
MTOW: 9 kg (vs 123 kg T-150)
Max Speed: 23 m/s (vs 30 m/s)
Climb Rate: 6 m/s (vs 5 m/s) - better power/weight
Battery: 274 Wh (vs 7000 Wh)
Hover Power: ~500 W (vs 18 kW)

# Key changes:
- Reduce all speed values ~25%
- Increase climb/descent rates slightly
- Much lower power values
- Shorter flight times
- May need higher gains (more agile)
```

---

## Conclusion

This framework provides a solid foundation for rotorcraft simulation. The key to successful adaptation is:

1. Accurate specification data
2. Systematic parameter updating
3. Incremental testing and tuning
4. Validation against known performance

Start with conservative gains and gradually increase until desired performance is achieved.
