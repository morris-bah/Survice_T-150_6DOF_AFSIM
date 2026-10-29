# Interface Control Document (ICD)

# T-150 UAS AFSIM Plugin Suite

**Version 1.0**

| Field | Value |
|---|---|
| Document Title | T-150 UAS AFSIM Plugin Suite Interface Control Document |
| Version | 1.0 |
| Date | 5 March 2026 |
| Classification | UNCLASSIFIED |
| Prepared By | [PLACEHOLDER - Author Name] |
| Organization | [PLACEHOLDER - Organization] |
| Contract Number | [PLACEHOLDER] |
| Distribution | [PLACEHOLDER - Distribution Statement] |

---

## Revision History

| Version | Date | Author | Description |
|---|---|---|---|
| 1.0 | 5 March 2026 | [PLACEHOLDER] | Initial release |

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Plugin Interface Overview](#2-plugin-interface-overview)
3. [Plugin API Entry Points](#3-plugin-api-entry-points)
4. [AuxData Interface](#4-auxdata-interface)
5. [Inter-Plugin Data Flow](#5-inter-plugin-data-flow)
6. [Scenario Script Interface](#6-scenario-script-interface)
7. [External Interfaces](#7-external-interfaces)
8. [References](#8-references)
9. [Acronyms](#9-acronyms)

---

## 1. Introduction

### 1.1 Purpose

This Interface Control Document (ICD) defines the software interfaces for the T-150 UAS AFSIM Plugin Suite. It specifies how each plugin communicates with the AFSIM simulation framework and with other plugins in the suite. The document serves as the authoritative reference for interface agreements between plugin components.

This document is adapted from the NGIC ICD template (originally designed for ODESSA DLL interfaces) to the AFSIM plugin architecture. The following mapping conventions apply:

- "DLL" maps to "Plugin (.dll/.so)"
- "Structs" maps to "AuxData Key-Value Interface"
- "DLL Functions" maps to "Plugin API Entry Points"
- Sections specific to RF, antenna patterns, or frequency interfaces are marked "Not Applicable"

### 1.2 Scope

This ICD covers the five plugins comprising the T-150 UAS simulation capability:

| Plugin Library | Registered Type | Base Class |
|---|---|---|
| t150_battery (.dll/.so) | T150_BATTERY_PROCESSOR | WsfProcessor |
| t150_propulsion (.dll/.so) | T150_PROPULSION_PROCESSOR | WsfProcessor |
| t150_vrs (.dll/.so) | T150_VRS_PROCESSOR | WsfProcessor |
| t150_slung_load (.dll/.so) | T150_SLUNG_LOAD_PROCESSOR | WsfProcessor |
| rotorcraft_6dof (.dll/.so) | WSF_ROTORCRAFT_6DOF_MOVER | WsfRouteMover |

The document covers plugin loading, API entry points, inter-plugin data exchange via the AuxData key-value store, scenario configuration interfaces, and external output interfaces.

### 1.3 Document Organization

- Section 2 describes how AFSIM discovers, loads, and initializes plugins.
- Section 3 specifies the required C-linkage entry points and registration classes.
- Section 4 defines the complete AuxData key-value interface organized by producer.
- Section 5 describes inter-plugin data flow and producer-consumer relationships.
- Section 6 documents the scenario script configuration interface.
- Section 7 covers external interfaces including file output and visualization.
- Sections 8 and 9 provide references and acronyms.

### 1.4 Applicable Standards

- AFSIM Plugin API (version determined at compile time via WSF_PLUGIN_API_MAJOR_VERSION / WSF_PLUGIN_API_MINOR_VERSION)
- C++17 language standard
- CMake 3.16+ build system

---

## 2. Plugin Interface Overview

### 2.1 Plugin Discovery and Loading

AFSIM discovers plugins by scanning shared library files in its plugin directories. On Linux, plugins are `.so` files; on Windows, `.dll` files. AFSIM loads each library and invokes standardized C-linkage entry points to negotiate version compatibility and register extensions.

### 2.2 Plugin Directories

Plugins must be installed to both AFSIM system directories to be discovered:

| Directory | Purpose |
|---|---|
| `<AFSIM_ROOT>/lib/wsf_plugins/` | Primary plugin search path |
| `<AFSIM_ROOT>/bin/mission_plugins/` | Secondary plugin search path |

On Linux, the typical AFSIM root is `/opt/afsim*/`. On Windows, it is the AFSIM installation directory. Plugins placed in project-local directories (e.g., `./lib/`) are **not** loaded by the AFSIM runtime.

### 2.3 Plugin Lifecycle

Each plugin follows a five-stage lifecycle:

```
1. Load         AFSIM loads the shared library (.dll/.so)
                Global constructors execute (PluginLoadVerifier)

2. Version      AFSIM calls WsfPluginVersion()
                Plugin returns compiled API major/minor/ABI string
                AFSIM verifies compatibility before proceeding

3. Setup        AFSIM calls WsfPluginSetup()
                Plugin registers feature name and ApplicationExtension

4. Scenario     AFSIM creates a scenario and calls
                ApplicationExtension::ScenarioCreated()
                Plugin registers ScenarioExtension with the scenario

5. Registration ScenarioExtension::AddedToScenario() is called
                Plugin registers its processor or mover type via
                AddCoreType<>() so scenario files can instantiate it
```

### 2.4 Feature Registration

Each plugin registers a feature name that scenario files can query to verify plugin availability:

| Plugin | Feature Name | Extension Name |
|---|---|---|
| t150_battery | `t150_battery` | `t150_battery_registration` |
| t150_propulsion | `t150_propulsion` | `t150_propulsion_registration` |
| t150_vrs | `t150_vrs` | `t150_vrs_registration` |
| t150_slung_load | `t150_slung_load` | `t150_slung_load_registration` |
| rotorcraft_6dof | `rotorcraft_6dof` | `rotorcraft_6dof_registration` |

---

## 3. Plugin API Entry Points

### 3.1 Common Entry Points

All five plugins export the same two C-linkage functions. The function signatures use AFSIM-provided types.

#### 3.1.1 WsfPluginVersion

```cpp
extern "C" EXPORT_MACRO void WsfPluginVersion(UtPluginVersion& aVersion)
```

**Purpose:** Returns the AFSIM API version the plugin was compiled against.

**Behavior:** Constructs a `UtPluginVersion` from compile-time constants:

| Constant | Description |
|---|---|
| `WSF_PLUGIN_API_MAJOR_VERSION` | AFSIM major API version |
| `WSF_PLUGIN_API_MINOR_VERSION` | AFSIM minor API version |
| `WSF_PLUGIN_API_SYSTEMABI_STRING` | Platform ABI identifier |

AFSIM compares these values against its own version. If the major version differs, the plugin is rejected.

#### 3.1.2 WsfPluginSetup

```cpp
extern "C" EXPORT_MACRO void WsfPluginSetup(WsfApplication& aApplication)
```

**Purpose:** Registers the plugin's feature name and application extension.

**Behavior:**
1. Checks `aApplication.ExtensionIsRegistered()` to prevent duplicate registration.
2. Calls `aApplication.RegisterFeature()` with the plugin's feature name.
3. Calls `aApplication.RegisterExtension()` with a new `ApplicationExtension` instance.

The battery plugin additionally prints a CUI banner on first plugin load (coordinated via the `NGIC_BANNER_PRINTED` environment variable to print only once across all NGIC plugins).

### 3.2 ApplicationExtension::ScenarioCreated

```cpp
void ScenarioCreated(WsfScenario& aScenario) override
```

**Purpose:** Registers a `ScenarioExtension` with the newly created scenario.

**Behavior:** Calls `aScenario.RegisterExtension()` with a scenario-specific registration object. Most plugins use `WsfDefaultApplicationExtension<RegistrationType>` as a convenience wrapper. The battery plugin uses a custom `ApplicationExtension` class.

### 3.3 ScenarioExtension::AddedToScenario

```cpp
void AddedToScenario() override
```

**Purpose:** Registers the plugin's processor or mover type so that scenario files can instantiate it.

**Behavior by plugin:**

| Plugin | Registration Call | Type Name |
|---|---|---|
| t150_battery | `WsfProcessorTypes::Get(scen).AddCoreType(...)` | `T150_BATTERY_PROCESSOR` |
| t150_propulsion | `WsfProcessorTypes::Get(scen).AddCoreType(...)` | `T150_PROPULSION_PROCESSOR` |
| t150_vrs | `WsfProcessorTypes::Get(scen).AddCoreType(...)` | `T150_VRS_PROCESSOR` |
| t150_slung_load | `WsfProcessorTypes::Get(scen).AddCoreType(...)` | `T150_SLUNG_LOAD_PROCESSOR` |
| rotorcraft_6dof | `WsfMoverTypes::Get(scen).AddCoreType(...)` | `WSF_ROTORCRAFT_6DOF_MOVER` |

Each `AddCoreType` call provides a prototype instance that AFSIM clones (via the `Clone()` method) for each platform that uses that type.

---

## 4. AuxData Interface

The AuxData interface is the primary mechanism for inter-plugin communication. Each plugin writes key-value pairs to the platform's `WsfAttributeContainer` (accessed via `GetPlatform()->GetAuxData()`). Other plugins and scenario scripts can read these values. All keys are string identifiers; values are typed as double, int, bool, or string.

### 4.1 Battery Processor Outputs (9 Keys)

Producer: `T150_BATTERY_PROCESSOR`

| AuxData Key | Type | Units | Range | Description |
|---|---|---|---|---|
| `BATTERY_SOC` | double | dimensionless | 0.0 -- 1.0 | State of charge (1.0 = fully charged) |
| `BATTERY_POWER_W` | double | W | >= 0 | Current total electrical power draw |
| `BATTERY_ENERGY_WH` | double | Wh | >= 0 | Remaining energy in battery |
| `BATTERY_VOLTAGE` | double | V | 39.0 -- 54.6 | Terminal voltage under load |
| `BATTERY_OCV` | double | V | 39.0 -- 54.6 | Open-circuit voltage (no-load) |
| `BATTERY_VOLTAGE_SAG` | double | V | >= 0 | Voltage drop due to internal resistance |
| `BATTERY_CURRENT_A` | double | A | >= 0 | Current draw from battery |
| `BATTERY_LOW` | bool | -- | true/false | SOC below low-battery threshold (default 20%) |
| `BATTERY_VOLTAGE_LOW` | bool | -- | true/false | Terminal voltage below sag warning level |

**Update rate:** 10 Hz (100 ms update interval)

**Notes:** `BATTERY_LOW` is set once when SOC drops below threshold and does not reset. `BATTERY_VOLTAGE_LOW` resets when voltage recovers above the warning level plus 1.0 V hysteresis.

### 4.2 Propulsion Processor Outputs (19 Keys)

Producer: `T150_PROPULSION_PROCESSOR`

| AuxData Key | Type | Units | Range | Description |
|---|---|---|---|---|
| `PROP_TOTAL_THRUST` | double | N | >= 0 | Total thrust from all rotors (with ground effect) |
| `PROP_TOTAL_POWER` | double | W | >= 0 | Total electrical power consumption |
| `PROP_SYSTEM_EFF` | double | dimensionless | 0.0 -- 1.0 | Overall system efficiency (motor * coaxial) |
| `PROP_MOTOR_COUNT` | int | count | 0 -- 8 | Number of currently operational motors |
| `PROP_MAX_TEMP` | double | deg C | -40 -- 200 | Hottest motor temperature |
| `PROP_AVG_TEMP` | double | deg C | -40 -- 200 | Average motor temperature |
| `PROP_GROUND_EFFECT` | double | dimensionless | 1.0 -- 1.3 | Ground effect thrust multiplier |
| `PROP_ALTITUDE_AGL` | double | m | >= 0 | Current altitude above ground level |
| `PROP_TOTAL_MASS` | double | kg | > 0 | Total mass (empty + payload) |
| `PROP_PAYLOAD_MASS` | double | kg | >= 0 | Payload mass |
| `PROP_THRUST_MARGIN` | double | dimensionless | 0.0 -- 1.0 | Available thrust margin above hover |
| `PROP_CG_OFFSET_X` | double | m | any | Combined CG offset, forward axis |
| `PROP_CG_OFFSET_Y` | double | m | any | Combined CG offset, lateral axis |
| `PROP_CG_OFFSET_Z` | double | m | any | Combined CG offset, vertical axis |
| `PROP_OVERLOADED` | bool | -- | true/false | Payload exceeds max capacity |
| `PROP_FORWARD_SPEED` | double | m/s | >= 0 | Horizontal ground speed |
| `PROP_CLIMB_RATE` | double | m/s | any | Vertical speed (positive = climbing) |
| `PROP_BANK_ANGLE` | double | deg | 0 -- 60 | Estimated bank angle |
| `PROP_ACCEL_MAG` | double | m/s^2 | >= 0 | Horizontal acceleration magnitude |

**Update rate:** 10 Hz (100 ms update interval)

### 4.3 VRS Processor Outputs (10 Keys)

Producer: `T150_VRS_PROCESSOR`

| AuxData Key | Type | Units | Range | Description |
|---|---|---|---|---|
| `VRS_MARGIN` | double | dimensionless | 0.0 -- 1.0 | Distance from VRS boundary (0 = in VRS, 1 = clear) |
| `VRS_STATE` | int | enum | 0 -- 4 | VRS state machine value (see below) |
| `VRS_LIFT_FACTOR` | double | dimensionless | 0.4 -- 1.0 | Lift degradation factor (1.0 = no degradation) |
| `VRS_CONTROL_FACTOR` | double | dimensionless | 0.5 -- 1.0 | Control authority factor (1.0 = full authority) |
| `VRS_DEVELOPMENT` | double | dimensionless | 0.0 -- 1.0 | VRS development progression |
| `VRS_DESCENT_RATE` | double | m/s | >= 0 | Current descent rate (positive = descending) |
| `VRS_FORWARD_SPEED` | double | m/s | >= 0 | Current horizontal speed |
| `VRS_CAUTION` | bool | -- | true/false | Approaching VRS envelope |
| `VRS_ONSET` | bool | -- | true/false | VRS onset detected |
| `VRS_CRITICAL` | bool | -- | true/false | VRS fully developed, recovery action required |

**VRS_STATE enum values:**

| Value | State | Description |
|---|---|---|
| 0 | CLEAR | No VRS risk |
| 1 | CAUTION | Approaching VRS boundary |
| 2 | ONSET | VRS conditions beginning |
| 3 | DEVELOPED | Full VRS, severe lift/control degradation |
| 4 | RECOVERY | Transitioning out of VRS |

**Update rate:** 10 Hz (100 ms update interval)

### 4.4 Slung Load Processor Outputs (14 Keys)

Producer: `T150_SLUNG_LOAD_PROCESSOR`

| AuxData Key | Type | Units | Range | Description |
|---|---|---|---|---|
| `SLUNG_LOAD_STATE` | int | enum | 0 -- 4 | Load state (see below) |
| `SLUNG_LOAD_MASS` | double | kg | >= 0 | Attached load mass |
| `SLUNG_CABLE_LENGTH` | double | m | >= 0.5 | Cable length |
| `SLUNG_CABLE_TENSION` | double | N | >= 0 | Current cable tension |
| `SLUNG_SWING_X` | double | deg | -70 -- 70 | Pendulum angle in pitch (X-Z plane) |
| `SLUNG_SWING_Y` | double | deg | -70 -- 70 | Pendulum angle in roll (Y-Z plane) |
| `SLUNG_SWING_MAG` | double | deg | 0 -- 70 | Combined swing angle magnitude |
| `SLUNG_FORCE_X` | double | N | any | Cable reaction force on aircraft, forward axis |
| `SLUNG_FORCE_Y` | double | N | any | Cable reaction force on aircraft, lateral axis |
| `SLUNG_FORCE_Z` | double | N | >= 0 | Cable reaction force on aircraft, vertical axis |
| `SLUNG_POS_X` | double | m | any | Load position relative to aircraft CG, forward |
| `SLUNG_POS_Y` | double | m | any | Load position relative to aircraft CG, lateral |
| `SLUNG_POS_Z` | double | m | any | Load position relative to aircraft CG, vertical |
| `SLUNG_NATURAL_FREQ` | double | Hz | > 0 | Pendulum natural frequency: f = (1/2*pi)*sqrt(g/L) |
| `SLUNG_RESONANCE` | bool | -- | true/false | Resonance condition detected |

**SLUNG_LOAD_STATE enum values:**

| Value | State | Description |
|---|---|---|
| 0 | DETACHED | No load attached |
| 1 | ATTACHED | Load attached, stable |
| 2 | SWINGING | Swing exceeds resonance threshold |
| 3 | RESONANCE | Swing frequency matches natural frequency at large amplitude |
| 4 | RELEASED | Load released (cable overload or commanded) |

**Update rate:** 50 Hz (20 ms update interval, configurable)

### 4.5 6DOF Mover Outputs (16 Keys)

Producer: `WSF_ROTORCRAFT_6DOF_MOVER`

| AuxData Key | Type | Units | Description |
|---|---|---|---|
| `6dof_pos_n` | double | m | Local NED position, North |
| `6dof_pos_e` | double | m | Local NED position, East |
| `6dof_pos_d` | double | m | Local NED position, Down |
| `6dof_vel_n` | double | m/s | NED velocity, North |
| `6dof_vel_e` | double | m/s | NED velocity, East |
| `6dof_vel_d` | double | m/s | NED velocity, Down (positive = descending) |
| `6dof_roll_deg` | double | deg | Roll angle |
| `6dof_pitch_deg` | double | deg | Pitch angle |
| `6dof_yaw_deg` | double | deg | Yaw angle (heading) |
| `6dof_p_dps` | double | deg/s | Roll rate |
| `6dof_q_dps` | double | deg/s | Pitch rate |
| `6dof_r_dps` | double | deg/s | Yaw rate |
| `6dof_total_thrust` | double | N | Total rotor thrust |
| `6dof_total_power` | double | W | Total mechanical power (shaft) |
| `6dof_throttle_cmd` | double | 0--1 | Throttle command |
| `6dof_avg_rpm` | double | RPM | Average rotor RPM |
| `6dof_mass` | double | kg | Current aircraft mass |
| `6dof_weight` | double | N | Current weight force |
| `6dof_target_alt` | double | m MSL | Target altitude |

**Update rate:** 100 Hz (10 ms update interval)

### 4.6 6DOF Mover Consumed Keys

The 6DOF mover reads AuxData keys published by other processors to integrate external forces and moments:

| Consumed Key | Source Plugin | Purpose |
|---|---|---|
| `SLUNG_FORCE_X/Y/Z` | Slung Load Processor | Cable reaction forces on aircraft |
| `WEAPON_FORCE_X/Y/Z` | Weapon Processor (legacy) | Weapon recoil forces |
| `WEAPON_<name>_FORCE_X/Y/Z` | Weapon Processor (registry) | Per-weapon recoil forces |
| `WEAPON_MOMENT_X/Y/Z` | Weapon Processor (legacy) | Weapon recoil moments |
| `WEAPON_<name>_MOMENT_X/Y/Z` | Weapon Processor (registry) | Per-weapon recoil moments |
| `WEAPON_REGISTRY` | Weapon Processor | Comma-separated list of registered weapon names |
| `WEAPON_<name>_MASS_DELTA` | Weapon Processor | Ammunition mass change per weapon |

---

## 5. Inter-Plugin Data Flow

### 5.1 Data Flow Diagram

```
+---------------------+        6dof_total_power         +---------------------+
| WSF_ROTORCRAFT_     | ------------------------------> | T150_BATTERY_       |
| 6DOF_MOVER          |                                 | PROCESSOR           |
|                     |                                 |                     |
| Publishes:          |    PROP_TOTAL_POWER (fallback)  | Consumes:           |
|   6dof_total_power  | <--+                            |   6dof_total_power  |
|   6dof_total_thrust |    |                            |   PROP_TOTAL_POWER  |
|   6dof_vel_*        |    |                            |   (fallback chain)  |
|   6dof_roll/pitch/  |    |                            |                     |
|   yaw_deg           |    |   +--------------------+   | Publishes:          |
|                     |    +-- | T150_PROPULSION_   |   |   BATTERY_SOC       |
| Consumes:           |        | PROCESSOR          |   |   BATTERY_VOLTAGE   |
|   SLUNG_FORCE_X/Y/Z|        |                    |   |   BATTERY_POWER_W   |
|   WEAPON_FORCE_*    |        | Publishes:         |   +---------------------+
|   WEAPON_MOMENT_*   |        |   PROP_TOTAL_POWER |
+---------------------+        |   PROP_TOTAL_THRUST|
        ^                      |   PROP_GROUND_EFF  |
        |                      |   PROP_*_SPEED     |
        |                      +--------------------+
        |
        |  SLUNG_FORCE_X/Y/Z
        |
+---------------------+
| T150_SLUNG_LOAD_    |
| PROCESSOR           |
|                     |        +---------------------+
| Consumes:           |        | T150_VRS_           |
|   Platform velocity |        | PROCESSOR           |
|   (GetVelocityNED)  |        |                     |
|                     |        | Consumes:           |
| Publishes:          |        |   Platform velocity |
|   SLUNG_FORCE_X/Y/Z |       |   (GetVelocityNED)  |
|   SLUNG_CABLE_*     |        |                     |
|   SLUNG_SWING_*     |        | Publishes:          |
+---------------------+        |   VRS_MARGIN        |
                               |   VRS_STATE         |
                               |   VRS_LIFT_FACTOR   |
                               +---------------------+
```

### 5.2 Producer-Consumer Relationships

| Data Path | Producer | Consumer | Key(s) | Direction |
|---|---|---|---|---|
| Mover power to battery | 6DOF Mover | Battery Processor | `6dof_total_power` | Mover --> Battery |
| Propulsion power to battery | Propulsion Processor | Battery Processor | `PROP_TOTAL_POWER` | Propulsion --> Battery (fallback) |
| Slung load forces to mover | Slung Load Processor | 6DOF Mover | `SLUNG_FORCE_X/Y/Z` | Slung Load --> Mover |
| Platform velocity to VRS | AFSIM Mover (any) | VRS Processor | `GetVelocityNED()` | Platform API |
| Platform velocity to slung load | AFSIM Mover (any) | Slung Load Processor | `GetVelocityNED()` | Platform API |

### 5.3 Battery Power Source Fallback Chain

The Battery Processor uses a three-level fallback chain when `use_dynamic_power` is enabled:

1. **Configured key** -- reads from `power_source_key` (default: `PROP_TOTAL_POWER`, typically overridden to `6dof_total_power`)
2. **6DOF mover power** -- reads `6dof_total_power` if the configured key returns zero or is absent
3. **Propulsion processor power** -- reads `PROP_TOTAL_POWER` if neither of the above yields a positive value
4. **Fixed fallback** -- uses `hover_power_w` if no dynamic source is available

### 5.4 Platform API Interfaces

The VRS Processor and Slung Load Processor do not consume AuxData from other plugins. Instead, they read flight state directly from the AFSIM platform API:

- `GetPlatform()->GetVelocityNED(velocityNED)` -- provides NED velocity vector
- `GetPlatform()->GetAltitude()` -- provides altitude MSL

Both processors derive acceleration by numerical differentiation of velocity across timesteps, with acceleration values clamped to filter numerical noise.

---

## 6. Scenario Script Interface

### 6.1 Configuration Mechanism

Each plugin parses its configuration via the `ProcessInput(UtInput& aInput)` method. AFSIM calls this method for each line within a `processor ... end_processor` or `mover ... end_mover` block in the scenario file. The method reads the command keyword via `aInput.GetCommand()` and the value via `aInput.ReadValue()` or `aInput.ReadBool()`.

### 6.2 String Parameter Quote Stripping

AFSIM's `ReadValue()` for string parameters includes surrounding double quotes in the returned value. Plugins that accept string parameters (e.g., `power_source_key`, `csv_output_file`) must strip these quotes. The Battery Processor implements this explicitly:

```cpp
if (mPowerSourceKey.front() == '"' && mPowerSourceKey.back() == '"')
    mPowerSourceKey = mPowerSourceKey.substr(1, mPowerSourceKey.size() - 2);
```

Scenario authors must enclose string values in quotes: `power_source_key "6dof_total_power"`.

### 6.3 Battery Processor Configuration Parameters

Type name: `T150_BATTERY_PROCESSOR`

| Parameter | Type | Default | Units | Description |
|---|---|---|---|---|
| `battery_capacity_wh` | double | 7000.0 | Wh | Total battery energy capacity |
| `initial_soc` | double | 1.0 | 0--1 | Starting state of charge |
| `hover_power_w` | double | 18000.0 | W | Fallback hover power consumption |
| `low_battery_warning` | double | 0.20 | 0--1 | SOC threshold for low battery warning |
| `nominal_voltage` | double | 46.8 | V | Nominal pack voltage |
| `full_charge_voltage` | double | 54.6 | V | Full charge pack voltage |
| `cutoff_voltage` | double | 39.0 | V | Discharge cutoff voltage |
| `internal_resistance` | double | 0.025 | ohm | Pack internal resistance |
| `voltage_sag_warning` | double | 42.0 | V | Voltage warning threshold |
| `use_dynamic_power` | bool | false | -- | Enable reading power from AuxData |
| `power_source_key` | string | "PROP_TOTAL_POWER" | -- | AuxData key for power input |
| `auxiliary_power_w` | double | 4000.0 | W | Constant auxiliary systems draw |
| `motor_efficiency` | double | 0.85 | 0.1--1.0 | Mechanical-to-electrical conversion efficiency |
| `min_flight_power_factor` | double | 0.50 | 0.1--1.0 | Minimum power as fraction of hover |
| `cruise_power_w` | double | 17000.0 | W | Power at cruise speed |
| `cruise_speed_mps` | double | 30.0 | m/s | Cruise speed for velocity model |
| `use_velocity_power_model` | bool | true | -- | Enable velocity-based power estimation |
| `csv_output_file` | string | (empty) | path | CSV telemetry output file path |
| `csv_log_interval` | double | 1.0 | s | Interval between CSV log entries |

**Example scenario configuration:**

```
processor battery_monitor T150_BATTERY_PROCESSOR
   battery_capacity_wh 7000
   initial_soc 1.0
   nominal_voltage 46.8
   full_charge_voltage 54.6
   cutoff_voltage 39.0
   internal_resistance 0.025
   use_dynamic_power true
   power_source_key "6dof_total_power"
   hover_power_w 18000
   motor_efficiency 0.37
   auxiliary_power_w 4000
   use_velocity_power_model false
   csv_output_file /path/to/output/battery.csv
   csv_log_interval 1.0
end_processor
```

### 6.4 Propulsion Processor Configuration Parameters

Type name: `T150_PROPULSION_PROCESSOR`

| Parameter | Type | Default | Units | Description |
|---|---|---|---|---|
| `motor_count` | int | 8 | count | Number of motors (max 8) |
| `kv_rating` | double | 100.0 | RPM/V | Motor Kv rating |
| `max_power_per_motor` | double | 3500.0 | W | Maximum power per motor |
| `propeller_diameter` | double | 0.76 | m | Propeller diameter |
| `coaxial_efficiency` | double | 0.85 | 0.5--1.0 | Coaxial rotor interference factor |
| `motor_thermal_limit` | double | 120.0 | deg C | Motor overtemperature threshold |
| `esc_thermal_limit` | double | 90.0 | deg C | ESC overtemperature threshold |
| `nominal_voltage` | double | 46.8 | V | Battery pack nominal voltage |
| `peak_motor_efficiency` | double | 0.92 | 0.5--0.98 | Peak motor efficiency |
| `ambient_temperature` | double | 25.0 | deg C | Ambient temperature |
| `motor_time_constant` | double | 0.15 | s | First-order motor lag time constant |
| `ground_effect_enabled` | bool | true | -- | Enable ground effect model |
| `rotor_height` | double | 0.5 | m | Rotor height above ground reference |
| `ground_effect_max_boost` | double | 1.3 | 1.0--1.5 | Maximum ground effect thrust multiplier |
| `empty_mass` | double | 68.0 | kg | Aircraft empty mass |
| `payload_mass` | double | 0.0 | kg | Payload mass |
| `max_payload_mass` | double | 55.0 | kg | Maximum payload capacity |
| `payload_cg_offset_x/y/z` | double | 0/0/0.2 | m | Payload CG offset from aircraft CG |
| `drag_coefficient` | double | 1.0 | 0.1--2.0 | Bluff body drag coefficient |
| `frontal_area` | double | 0.30 | m^2 | Frontal reference area |
| `induced_power_factor` | double | 1.15 | 1.0--1.5 | Non-ideal rotor power factor |
| `csv_output_file` | string | (empty) | path | CSV telemetry output file path |
| `csv_log_interval` | double | 1.0 | s | Interval between CSV log entries |

### 6.5 VRS Processor Configuration Parameters

Type name: `T150_VRS_PROCESSOR`

| Parameter | Type | Default | Units | Description |
|---|---|---|---|---|
| `vrs_descent_threshold` | double | 4.0 | m/s | Descent rate for VRS onset |
| `vrs_speed_threshold` | double | 5.0 | m/s | Forward speed that prevents VRS |
| `caution_margin` | double | 0.8 | 0.5--0.95 | VRS margin level for caution |
| `induced_velocity` | double | 8.0 | m/s | Estimated hover induced velocity |
| `coaxial_factor` | double | 0.85 | 0.5--1.0 | Coaxial rotor VRS resistance factor |
| `vrs_development_time` | double | 2.0 | s | Time to develop full VRS |
| `vrs_recovery_time` | double | 3.0 | s | Time to recover from VRS |
| `max_lift_degradation` | double | 0.6 | 0.0--1.0 | Maximum lift loss fraction in VRS |
| `max_control_degradation` | double | 0.5 | 0.0--1.0 | Maximum control authority loss in VRS |

### 6.6 Slung Load Processor Configuration Parameters

Type name: `T150_SLUNG_LOAD_PROCESSOR`

| Parameter | Type | Default | Units | Description |
|---|---|---|---|---|
| `load_mass` | double | 0.0 | kg | Attached load mass (>0 auto-attaches) |
| `cable_length` | double | 5.0 | m | Cable length (min 0.5 m) |
| `cable_damping` | double | 0.1 | -- | Pendulum damping coefficient |
| `attach_point_x/y/z` | double | 0/0/0.5 | m | Cable attach point relative to CG |
| `max_cable_tension` | double | 5000.0 | N | Cable tension limit (triggers release) |
| `resonance_threshold` | double | 20.0 | deg | Swing angle for resonance warning (input in degrees) |
| `update_interval` | double | 0.02 | s | Physics update interval (min 1 ms) |

### 6.7 6DOF Mover Configuration Parameters

Type name: `WSF_ROTORCRAFT_6DOF_MOVER`

The 6DOF mover inherits route-handling parameters from `WsfRouteMover` and adds physics configuration:

| Parameter | Type | Default | Units | Description |
|---|---|---|---|---|
| `mass` | double | -- | kg | Total aircraft mass |
| `inertia_xx/yy/zz` | double | -- | kg*m^2 | Moments of inertia |
| `rotor_configuration` | string | -- | -- | COAXIAL_QUAD, COAXIAL_HEX, COAXIAL_OCTO, SINGLE_QUAD, SINGLE_HEX, SINGLE_OCTO |
| `rotor_diameter` | double | -- | m | Rotor/propeller diameter |
| `arm_length` | double | -- | m | Motor arm length from CG |
| `coaxial_spacing` | double | -- | m | Vertical spacing between coaxial rotors |
| `thrust_coefficient` | double | -- | -- | Rotor thrust coefficient (Ct) |
| `torque_coefficient` | double | -- | -- | Rotor torque coefficient (Cq) |
| `max_rpm` | double | -- | RPM | Maximum rotor RPM |
| `motor_time_constant` | double | -- | s | Motor first-order lag |
| `drag_enabled` | bool | -- | -- | Enable aerodynamic drag model |
| `drag_coefficient_x/y/z` | double | -- | -- | Directional drag coefficients |
| `frontal_area` | double | -- | m^2 | Reference frontal area |
| `ground_effect_enabled` | bool | -- | -- | Enable ground effect model |
| `ground_effect_height_factor` | double | -- | -- | Ground effect height scaling |
| `ground_effect_max_boost` | double | -- | -- | Maximum thrust augmentation |
| `roll/pitch/yaw_mix_gain` | double | -- | -- | Control mixing gains |
| `altitude_hold` | bool | -- | -- | Enable altitude hold controller |
| `altitude_kp/kd` | double | -- | -- | Altitude controller gains |
| `position_hold` | bool | -- | -- | Enable position hold controller |
| `position_kp/kd` | double | -- | -- | Position controller gains |
| `velocity_kp` | double | -- | -- | Velocity controller gain |
| `max_vel_accel` | double | -- | m/s^2 | Maximum velocity acceleration |
| `max_tilt_angle` | double | -- | deg | Maximum tilt for position control |
| `heading_hold` | bool | -- | -- | Enable heading hold controller |
| `heading_kp` | double | -- | -- | Heading controller gain |
| `waypoint_arrival_radius` | double | -- | m | Distance to consider waypoint reached |
| `telemetry_enabled` | bool | -- | -- | Enable AuxData telemetry publishing |
| `spline_interpolation` | bool | false | -- | Enable Catmull-Rom spline path |
| `spline_tension` | double | -- | -- | Spline tension parameter |
| `max_deceleration` | double | -- | m/s^2 | Max deceleration for speed profiling |
| `default_waypoint_behavior` | string | FLY_THROUGH | -- | FLY_THROUGH, STOP_AND_HOVER, SLOW_PASS |
| `default_pass_speed` | double | -- | m/s | Speed for SLOW_PASS behavior |
| `default_hover_duration` | double | -- | s | Hover time for STOP_AND_HOVER |

---

## 7. External Interfaces

### 7.1 CSV File Output

Both the Battery Processor and Propulsion Processor support CSV telemetry logging when configured with the `csv_output_file` parameter.

**Battery Processor CSV columns:**
```
Time_s, SOC_pct, Voltage_V, Current_A, Power_W, Energy_Consumed_Wh,
OCV_V, Voltage_Sag_V, Remaining_Energy_Wh
```

**Propulsion Processor CSV columns:**
```
Time_s, Total_Thrust_N, Total_Power_W, System_Efficiency,
Altitude_AGL_m, Ground_Effect_Factor, Total_Mass_kg, Payload_Mass_kg,
Thrust_Margin, Forward_Speed_ms, Climb_Rate_ms, Bank_Angle_deg,
Motor_Avg_RPM, Motor_Avg_Temp_C, Motor_Avg_Current_A
```

The Battery Processor appends a summary block to the CSV at simulation end, including flight duration, initial/final SOC, total energy consumed, and average power.

Log interval is controlled by `csv_log_interval` (default 1.0 s, minimum 0.1 s).

### 7.2 AFSIM Event Pipe (AER Output)

All plugins output warnings and informational messages via the AFSIM logging API (`ut::log::info()`, `ut::log::warning()`). These messages appear in the AFSIM event log and, when configured, are captured in AER (AFSIM Event Record) output files via the `event_pipe` scenario directive. AER output captures simulation events including battery warnings, motor overheat events, VRS state transitions, and slung load releases.

### 7.3 AFSIM Mystic Visualization

All AuxData keys are accessible to the AFSIM Mystic visualization tool for real-time monitoring during simulation. The 6DOF mover additionally synchronizes platform position, velocity, and orientation to the AFSIM platform object via `SetLocationLLA()`, `SetVelocityNED()`, and `SetOrientationNED()`, enabling standard AFSIM 3D visualization.

### 7.4 Simulation Federation Interfaces

Not Applicable. The T-150 UAS plugin suite does not implement HLA, DIS, or other simulation federation interfaces.

### 7.5 RF and Antenna Interfaces

Not Applicable. The T-150 UAS plugin suite does not implement RF transmission, antenna pattern, or frequency management interfaces. Radar detectability is handled through standard AFSIM radar signature definitions in the platform type, not through the plugin interface.

---

## 8. References

| Ref | Document |
|---|---|
| [1] | AFSIM User's Guide, Air Force Research Laboratory |
| [2] | AFSIM Plugin Developer's Guide |
| [3] | Malloy Aeronautics T-150 / TRV-150 Technical Specifications |
| [4] | Johnson, W., *Helicopter Theory*, Dover Publications, 1994 |
| [5] | Leishman, J.G., *Principles of Helicopter Aerodynamics*, Cambridge University Press, 2006 |
| [6] | Cheeseman, I.C. and Bennett, W.E., "The Effect of the Ground on a Helicopter Rotor in Forward Flight," ARC R&M 3021, 1957 |
| [7] | Newman, S. et al., "Vortex Ring State Prediction Models," *Aeronautical Journal*, 2001 |
| [8] | Cicolani, L.S. and Kanning, G., "Equations of Motion of Slung-Load Systems," NASA TP-3280, 1992 |
| [9] | NGIC Software Interface Control Document Template |
| [10] | T-150 UAS AFSIM Plugin Suite -- User Acceptance Method (UAM), v3.0 |
| [11] | T-150 UAS AFSIM Plugin Suite -- Verification and Validation Plan |

---

## 9. Acronyms

| Acronym | Definition |
|---|---|
| ABI | Application Binary Interface |
| AER | AFSIM Event Record |
| AFSIM | Advanced Framework for Simulation, Integration, and Modeling |
| AGL | Above Ground Level |
| API | Application Programming Interface |
| CG | Center of Gravity |
| CSV | Comma-Separated Values |
| CUI | Controlled Unclassified Information |
| DIS | Distributed Interactive Simulation |
| DLL | Dynamic Link Library |
| DOF | Degrees of Freedom |
| ESC | Electronic Speed Controller |
| HLA | High Level Architecture |
| ICD | Interface Control Document |
| MSL | Mean Sea Level |
| MTOW | Maximum Takeoff Weight |
| NED | North-East-Down (coordinate frame) |
| NGIC | National Ground Intelligence Center |
| OCV | Open-Circuit Voltage |
| OGE | Out of Ground Effect |
| RF | Radio Frequency |
| RPM | Revolutions Per Minute |
| SOC | State of Charge |
| UAS | Unmanned Aircraft System |
| VRS | Vortex Ring State |
| WSF | Warfighter Simulation Framework |
