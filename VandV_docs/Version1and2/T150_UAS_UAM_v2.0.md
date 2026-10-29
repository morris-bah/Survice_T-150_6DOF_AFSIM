# User Acceptance Manual (UAM)

## T-150/TRV-150 Heavy-Lift UAS Simulation

### AFSIM Project 005

**Version:** 2.0
**Date:** January 15, 2026
**Classification:** UNCLASSIFIED // FOR OFFICIAL USE ONLY

---

## Table of Contents

1. [Overview](#1-overview)
2. [Quick Start Guide](#2-quick-start-guide)
3. [Platform Configuration](#3-platform-configuration)
4. [Scenario Development](#4-scenario-development)
5. [AuxData Reference](#5-auxdata-reference)
6. [Script Examples](#6-script-examples)
7. [Troubleshooting](#7-troubleshooting)
8. [Adapting for Other Aircraft](#8-adapting-for-other-aircraft)

---

## 1. Overview

### 1.1 What This Package Provides

The T-150 UAS AFSIM Plugin Suite provides high-fidelity simulation of heavy-lift multirotor aircraft with:

| Feature | Description |
|---------|-------------|
| **6DOF Physics** | Full rigid body dynamics with quaternion attitude |
| **Battery Modeling** | Li-Ion voltage sag, SOC tracking, energy consumption |
| **Power Model** | Dynamic power based on actual flight maneuvers |
| **VRS Detection** | Vortex Ring State safety warnings |
| **Slung Load** | Cable-suspended payload dynamics |
| **Ground Effect** | Thrust augmentation near ground |

### 1.2 Plugin Files

After installation, these plugins are available:

| Plugin File | AFSIM Type Name | Purpose |
|-------------|-----------------|---------|
| `rotorcraft_6dof.so` | `Rotorcraft6DOF_Mover` | Physics-based flight dynamics |
| `t150_battery_processor.so` | `T150_BATTERY_PROCESSOR` | Battery simulation |
| `t150_propulsion.so` | `T150_PROPULSION_PROCESSOR` | Motor/propulsion modeling |
| `t150_vrs.so` | `T150_VRS_PROCESSOR` | VRS detection |
| `t150_slung_load.so` | `T150_SLUNG_LOAD_PROCESSOR` | Slung load dynamics |

### 1.3 Tested Configurations

| Configuration | Value |
|---------------|-------|
| AFSIM Version | 2.2505.0 |
| Platform | Linux (AlmaLinux 9) |
| Compiler | GCC 13.3 |
| Test Results | 15/15 tests PASS |

---

## 2. Quick Start Guide

### 2.1 Installation

```bash
# 1. Clone the repository
git clone https://github.com/DSGarrett/project005_t150_uas.git
cd project005_t150_uas

# 2. Build plugins
./ngic_afsim_build.sh

# 3. Install to AFSIM (requires sudo)
sudo ./ngic_afsim_install.sh

# 4. Verify installation
ls /opt/afsim2.2505/lib/wsf_plugins/lib*.so
```

### 2.2 Run a Test Scenario

```bash
# Run the basic endurance test
/opt/afsim2.2505/bin/mission scenarios/phase1_tests/test10_endurance.afsim

# View in Mystic
/opt/afsim2.2505/bin/mystic output/phase1_testing/test10_endurance.aer
```

### 2.3 Minimal Platform Definition

```afsim
// Minimal T-150 platform with 6DOF mover and battery
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

---

## 3. Platform Configuration

### 3.1 6DOF Mover Configuration

The `Rotorcraft6DOF_Mover` is the core physics engine. Configure it with:

```afsim
mover Rotorcraft6DOF_Mover
    // === Mass Properties ===
    mass 123 kg                    // Total mass (MTOW)
    inertia_xx 2.5                 // Roll inertia [kg·m²]
    inertia_yy 2.5                 // Pitch inertia [kg·m²]
    inertia_zz 4.5                 // Yaw inertia [kg·m²]

    // === Rotor Configuration ===
    rotor_configuration COAXIAL_OCTO   // Options: QUAD_X, QUAD_PLUS, HEX_X, OCTO_X, COAXIAL_OCTO
    rotor_diameter 0.76 m          // Propeller diameter
    arm_length 0.5 m               // Motor arm length from CG
    thrust_coefficient 0.10        // Ct for momentum theory
    torque_coefficient 0.008       // Cq for reaction torque
    max_rpm 4500                   // Maximum motor RPM

    // === Speed Limits ===
    max_speed 30 m/s               // Maximum horizontal speed
    max_climb_rate 8 m/s           // Maximum climb rate
    max_descent_rate 4 m/s         // Maximum descent rate

    // === Controller Gains ===
    // Altitude PID
    altitude_kp 0.3
    altitude_ki 0.05
    altitude_kd 1.2

    // Position PD
    position_kp 1.0
    position_kd 0.1

    // Velocity P
    velocity_kp 3.0

    // Attitude PD
    attitude_kp 8.0
    attitude_kd 2.0

    // === Limits ===
    max_tilt_angle 25 deg          // Maximum bank/pitch angle
end_mover
```

### 3.2 Battery Processor Configuration

```afsim
processor BATTERY T150_BATTERY_PROCESSOR
    // === Capacity ===
    battery_capacity_wh 7000       // Total energy capacity [Wh]
    initial_soc 1.0                // Starting SOC (0-1, 1.0 = 100%)

    // === Voltage ===
    nominal_voltage 46.8           // 13S x 3.6V
    full_charge_voltage 54.6       // 13S x 4.2V
    cutoff_voltage 39.0            // 13S x 3.0V
    internal_resistance 0.025      // Pack resistance [Ohms]

    // === Power ===
    use_dynamic_power true         // Read power from 6DOF mover
    power_source_key "6dof_total_power"  // AuxData key for power input
    hover_power_w 18000            // Fallback if dynamic power unavailable
    motor_efficiency 0.85          // Motor/ESC efficiency (0-1)
    auxiliary_power_w 4000         // Avionics, cooling, etc.

    // === Warnings ===
    low_battery_warning 0.20       // SOC threshold for warning
    voltage_sag_warning 42.0       // Voltage threshold for warning

    // === Output ===
    csv_output_file output/battery.csv  // Optional CSV logging
end_processor
```

### 3.3 Propulsion Processor Configuration

```afsim
processor PROPULSION T150_PROPULSION_PROCESSOR
    // === Motors ===
    motor_count 8
    kv_rating 100.0                // RPM per Volt
    max_power_per_motor 3500.0     // Watts
    motor_time_constant 0.15       // Response lag [seconds]

    // === Propellers ===
    propeller_diameter 0.76        // meters
    coaxial_efficiency 0.85        // Lower rotor efficiency

    // === Thermal ===
    motor_thermal_limit 120.0      // Max temperature [°C]
    motor_thermal_mass 1500.0      // J/K
    motor_cooling_rate 20.0        // W/K

    // === Mass ===
    empty_mass 68.0                // Aircraft empty mass [kg]
    payload_mass 0.0               // Current payload [kg]
    max_payload_mass 55.0          // Maximum payload [kg]

    // === Ground Effect ===
    ground_effect_enabled true
end_processor
```

### 3.4 VRS Processor Configuration

```afsim
processor VRS T150_VRS_PROCESSOR
    vrs_descent_threshold 4.0      // Descent rate for VRS [m/s]
    vrs_speed_threshold 5.0        // Forward speed to exit VRS [m/s]
    caution_margin 0.8             // Margin for caution warning
    induced_velocity 8.0           // Hover induced velocity [m/s]
    coaxial_factor 0.85            // Coaxial VRS resistance
    vrs_development_time 2.0       // Time to develop full VRS [s]
    vrs_recovery_time 3.0          // Time to recover [s]
    max_lift_degradation 0.6       // Maximum lift loss (0.6 = 60%)
end_processor
```

### 3.5 Slung Load Processor Configuration

```afsim
processor SLUNG_LOAD T150_SLUNG_LOAD_PROCESSOR
    load_mass 20.0                 // Suspended load mass [kg]
    cable_length 5.0               // Cable length [m]
    cable_damping 0.1              // Damping coefficient [1/s]
    attach_point_z 0.5             // Attach point below CG [m]
    max_cable_tension 5000.0       // Maximum tension [N]
    resonance_threshold 20.0       // Swing angle for warning [deg]
    update_interval 0.02           // 50 Hz update rate [s]
end_processor
```

---

## 4. Scenario Development

### 4.1 Basic Scenario Structure

```afsim
// scenario_example.afsim

// Time settings
time 0 sec
end_time 600 sec

// Include platform definition
include platforms/t150_platform.txt

// Create aircraft instance
platform t150_test T150_UAS
    side blue
    position 35.0n 120.0w altitude 0 m agl

    route test_route
        position 35.0n 120.0w altitude 0 m agl speed 0 m/s
        position 35.001n 120.0w altitude 100 m agl speed 5 m/s
        position 35.01n 120.0w altitude 100 m agl speed 30 m/s
        position 35.02n 120.0w altitude 100 m agl speed 30 m/s
        position 35.021n 120.0w altitude 0 m agl speed 2 m/s stop
    end_route
end_platform

// Simulation output
execute at_time 0 sec
    create_event_file output/test.aer
end_execute
```

### 4.2 Route Waypoints

Waypoints support the following parameters:

```afsim
position <lat> <lon> altitude <alt> <units> <reference> speed <speed> <units> [stop]
```

| Parameter | Options | Description |
|-----------|---------|-------------|
| lat/lon | Decimal degrees or DMS | Position |
| altitude | number | Target altitude |
| alt units | m, ft, km | Altitude units |
| reference | agl, msl | Above ground or sea level |
| speed | number | Target speed at/after waypoint |
| speed units | m/s, kts, km/h | Speed units |
| stop | (optional) | Hover at waypoint before proceeding |

**Example waypoints:**

```afsim
// Takeoff
position 35.0n 120.0w altitude 0 m agl speed 0 m/s

// Climb
position 35.001n 120.0w altitude 100 m agl speed 5 m/s

// Cruise
position 35.05n 120.0w altitude 100 m agl speed 30 m/s

// Hover at waypoint
position 35.06n 120.0w altitude 100 m agl speed 0 m/s stop

// Land
position 35.06n 120.001w altitude 0 m agl speed 2 m/s stop
```

### 4.3 Script Events

Use AFSIM script blocks to monitor and react to aircraft state:

```afsim
// Monitor battery and take action
script_block battery_monitor
    on entry
        set soc = owner.AuxDataValue("BATTERY_SOC")
        if soc < 0.2 then
            writeln("Low battery warning! SOC = " + (string)(soc * 100) + "%")
            // Could trigger RTB here
        end_if
    end_entry
end_script_block

// Attach to platform
platform t150_test T150_UAS
    script battery_monitor repeat 10 sec
end_platform
```

### 4.4 CSV Data Logging

Enable CSV output for analysis:

```afsim
processor BATTERY T150_BATTERY_PROCESSOR
    csv_output_file output/battery_data.csv
end_processor

// Or use AFSIM built-in logging
execute at_time 0 sec
    open_data_file output/telemetry.csv
    data_file_format csv
    data_file_time_format seconds

    add_data_column "Time" sim_time
    add_data_column "SOC" t150_test.AuxDataValue("BATTERY_SOC")
    add_data_column "Power" t150_test.AuxDataValue("BATTERY_POWER_W")
    add_data_column "Altitude" t150_test.Altitude()
end_execute
```

---

## 5. AuxData Reference

All AuxData keys can be accessed from scripts using:
```afsim
platform.AuxDataValue("KEY_NAME")
```

### 5.1 6DOF Mover Keys

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `6dof_pos_n` | double | m | North position |
| `6dof_pos_e` | double | m | East position |
| `6dof_pos_d` | double | m | Down position |
| `6dof_vel_n` | double | m/s | North velocity |
| `6dof_vel_e` | double | m/s | East velocity |
| `6dof_vel_d` | double | m/s | Down velocity |
| `6dof_roll_deg` | double | deg | Roll angle |
| `6dof_pitch_deg` | double | deg | Pitch angle |
| `6dof_yaw_deg` | double | deg | Yaw angle |
| `6dof_total_thrust` | double | N | Total thrust |
| `6dof_total_power` | double | W | Mechanical power |
| `6dof_throttle_cmd` | double | 0-1 | Throttle command |

### 5.2 Battery Keys

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `BATTERY_SOC` | double | 0-1 | State of charge |
| `BATTERY_POWER_W` | double | W | Power consumption |
| `BATTERY_ENERGY_WH` | double | Wh | Remaining energy |
| `BATTERY_VOLTAGE` | double | V | Terminal voltage |
| `BATTERY_OCV` | double | V | Open-circuit voltage |
| `BATTERY_VOLTAGE_SAG` | double | V | IR drop |
| `BATTERY_CURRENT_A` | double | A | Current draw |
| `BATTERY_LOW` | bool | - | Low battery warning |
| `BATTERY_VOLTAGE_LOW` | bool | - | Voltage sag warning |

### 5.3 Propulsion Keys

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `PROP_TOTAL_POWER` | double | W | Electrical power |
| `PROP_TOTAL_THRUST` | double | N | Total thrust |
| `PROP_MAX_TEMP` | double | °C | Hottest motor |
| `PROP_AVG_TEMP` | double | °C | Average temp |
| `PROP_GROUND_EFFECT` | double | 1.0+ | GE factor |
| `PROP_ALTITUDE_AGL` | double | m | Altitude AGL |
| `PROP_TOTAL_MASS` | double | kg | Total mass |
| `PROP_OVERLOADED` | bool | - | Overload flag |

### 5.4 VRS Keys

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `VRS_STATE` | int | 0-4 | VRS state enum |
| `VRS_MARGIN` | double | 0-1 | Safety margin |
| `VRS_DESCENT_RATE` | double | m/s | Descent rate |
| `VRS_LIFT_FACTOR` | double | 0-1 | Lift degradation |
| `VRS_CONTROL_FACTOR` | double | 0-1 | Control degradation |
| `VRS_CAUTION` | bool | - | Caution warning |
| `VRS_CRITICAL` | bool | - | Critical warning |

### 5.5 Slung Load Keys

| Key | Type | Units | Description |
|-----|------|-------|-------------|
| `SLUNG_ANGLE_X` | double | deg | X-axis swing |
| `SLUNG_ANGLE_Y` | double | deg | Y-axis swing |
| `SLUNG_TENSION` | double | N | Cable tension |
| `SLUNG_LOAD_MASS` | double | kg | Load mass |
| `SLUNG_RESONANCE` | bool | - | Resonance warning |
| `SLUNG_OVERLOAD` | bool | - | Overload warning |

---

## 6. Script Examples

### 6.1 Battery Depletion Auto-Land

```afsim
script_block auto_land_on_low_battery
    on entry
        set soc = owner.AuxDataValue("BATTERY_SOC")

        if soc < 0.15 then
            writeln("CRITICAL: Battery at " + (string)(soc * 100) + "% - initiating emergency landing")

            // Command immediate descent
            owner.SetRoute(emergency_landing_route)
        end_if
    end_entry
end_script_block

// Emergency landing route
route emergency_landing_route
    position current altitude 0 m agl speed 3 m/s stop
end_route
```

### 6.2 VRS Avoidance

```afsim
script_block vrs_monitor
    on entry
        set margin = owner.AuxDataValue("VRS_MARGIN")
        set state = owner.AuxDataValue("VRS_STATE")

        if margin < 0.5 then
            writeln("VRS WARNING: Margin = " + (string)margin)

            // Could command speed increase or reduced descent
        end_if

        if state >= 3 then  // DEVELOPED
            writeln("VRS CRITICAL: Aircraft in developed VRS!")
        end_if
    end_entry
end_script_block
```

### 6.3 Power Consumption Logging

```afsim
script_block power_logger
    on entry
        set power = owner.AuxDataValue("6dof_total_power")
        set soc = owner.AuxDataValue("BATTERY_SOC")
        set alt = owner.Altitude()
        set vel = owner.Speed()

        writeln("T=" + (string)sim_time + "s | Power=" + (string)(power/1000) + "kW | SOC=" + (string)(soc*100) + "% | Alt=" + (string)alt + "m | Vel=" + (string)vel + "m/s")
    end_entry
end_script_block

platform t150 T150_UAS
    script power_logger repeat 30 sec
end_platform
```

### 6.4 Dynamic Payload Drop

```afsim
// Configure slung load
processor SLUNG_LOAD T150_SLUNG_LOAD_PROCESSOR
    load_mass 30.0
    cable_length 8.0
end_processor

// Drop payload at waypoint
script_block drop_payload
    on entry
        writeln("Releasing payload...")
        owner.AuxData().Set("SLUNG_LOAD_MASS", 0.0)
    end_entry
end_script_block

execute at waypoint drop_point
    run_script drop_payload
end_execute
```

---

## 7. Troubleshooting

### 7.1 Plugin Not Loading

**Symptom:** `Unknown type: Rotorcraft6DOF_Mover`

**Causes and Solutions:**

| Cause | Solution |
|-------|----------|
| Plugin not installed | Run `./ngic_afsim_install.sh` |
| Wrong AFSIM version | Rebuild with matching AFSIM |
| Permission issue | Check file permissions: `ls -la /opt/afsim*/lib/wsf_plugins/` |

**Verify plugin is loadable:**
```bash
ldd /opt/afsim2.2505/lib/wsf_plugins/librotorcraft_6dof.so
```

### 7.2 Aircraft Doesn't Move

**Symptom:** Aircraft stays at start position

**Checklist:**
1. Route has valid waypoints with different positions
2. Speed is specified: `speed 5 m/s`
3. First waypoint altitude matches starting altitude
4. Mover is correctly configured

### 7.3 Battery Drains Too Fast

**Symptom:** Flight time much shorter than expected

**Check these parameters:**
```afsim
motor_efficiency 0.85      // Should be 0.80-0.90
auxiliary_power_w 4000     // Reduce if too high
hover_power_w 18000        // Fallback power
```

**Also verify:**
- `use_dynamic_power true` is set
- 6DOF mover is publishing `6dof_total_power`

### 7.4 Altitude Oscillations

**Symptom:** Aircraft bounces up and down

**Solution:** Tune altitude PID gains:
```afsim
altitude_kp 0.3    // Reduce if oscillating
altitude_kd 1.2    // Increase for damping
altitude_ki 0.05   // Reduce to limit overshoot
```

### 7.5 Poor Waypoint Tracking

**Symptom:** Aircraft drifts or overshoots waypoints

**Solution:** Adjust position and velocity gains:
```afsim
position_kp 1.0    // Increase for tighter tracking
velocity_kp 3.0    // Reduce if overshooting
max_tilt_angle 25  // Increase for faster response
```

### 7.6 VRS False Positives

**Symptom:** VRS warnings during normal descent

**Solution:** Adjust VRS thresholds:
```afsim
vrs_descent_threshold 5.0  // Increase from 4.0
caution_margin 0.7         // Reduce from 0.8
```

---

## 8. Adapting for Other Aircraft

### 8.1 Quadcopter Example

```afsim
platform_type MY_QUAD WSF_PLATFORM

    mover Rotorcraft6DOF_Mover
        mass 2.5 kg
        inertia_xx 0.05
        inertia_yy 0.05
        inertia_zz 0.08

        rotor_configuration QUAD_X
        rotor_diameter 0.25 m
        arm_length 0.2 m
        max_rpm 12000

        max_speed 20 m/s
        max_climb_rate 5 m/s
        max_tilt_angle 35 deg

        altitude_kp 0.5
        altitude_ki 0.1
        altitude_kd 0.8
    end_mover

    processor BATTERY T150_BATTERY_PROCESSOR
        battery_capacity_wh 100
        nominal_voltage 14.8
        motor_efficiency 0.85
        auxiliary_power_w 20
        use_dynamic_power true
    end_processor

end_platform_type
```

### 8.2 Hexacopter Example

```afsim
platform_type MY_HEX WSF_PLATFORM

    mover Rotorcraft6DOF_Mover
        mass 15 kg
        inertia_xx 0.5
        inertia_yy 0.5
        inertia_zz 0.8

        rotor_configuration HEX_X
        rotor_diameter 0.4 m
        arm_length 0.35 m
        max_rpm 8000

        max_speed 25 m/s
        max_climb_rate 6 m/s
    end_mover

    processor BATTERY T150_BATTERY_PROCESSOR
        battery_capacity_wh 500
        nominal_voltage 22.2
        use_dynamic_power true
    end_processor

end_platform_type
```

### 8.3 Tuning Process

1. **Start with conservative gains** (lower than examples)
2. **Test hover stability** - adjust altitude gains
3. **Test position hold** - adjust position/velocity gains
4. **Test waypoint tracking** - fine-tune for your mission profile
5. **Validate battery life** - compare to real aircraft data

### 8.4 Physical Parameters

To adapt for your aircraft, you need:

| Parameter | How to Obtain |
|-----------|---------------|
| Mass | Weigh aircraft with battery |
| Inertia | Estimate from shape or measure with bifilar pendulum |
| Rotor diameter | Measure propeller |
| Arm length | Measure motor to CG distance |
| Max RPM | From motor/ESC specifications |
| Battery capacity | From battery specifications |
| Motor efficiency | Typically 0.80-0.90 for brushless |

---

## Appendix: Test Scenario Files

The following test scenarios are included for validation:

| Scenario | Purpose | Duration |
|----------|---------|----------|
| `test01_motor_step_response.afsim` | Throttle dynamics | 30s |
| `test02_attitude_recovery.afsim` | Turn stability | 60s |
| `test03_position_hold.afsim` | Hover drift | 120s |
| `test04_velocity_step.afsim` | Speed tracking | 60s |
| `test05_extreme_maneuvers.afsim` | Numerical stability | 120s |
| `test06_voltage_sag.afsim` | Low SOC behavior | 300s |
| `test07_ground_effect.afsim` | GE validation | 120s |
| `test08_coaxial_efficiency.afsim` | Power model | 60s |
| `test09_energy_conservation.afsim` | Energy balance | 300s |
| `test10_endurance.afsim` | Full battery test | 900s |
| `battery_test_straight.afsim` | Baseline power | 600s |
| `battery_test_sine.afsim` | Maneuver power | 700s |
| `battery_test_cycloid.afsim` | Loop pattern | 900s |

---

*Document generated with assistance from Claude Code*
*January 15, 2026*
