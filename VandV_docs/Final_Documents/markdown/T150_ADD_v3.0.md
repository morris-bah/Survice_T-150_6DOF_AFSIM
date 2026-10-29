# Architecture Description Document (ADD)

# T-150 UAS AFSIM Plugin Suite

**Version 3.0**

| Field | Value |
|---|---|
| Document Title | Architecture Description Document -- T-150 UAS AFSIM Plugin Suite |
| Version | 3.0 |
| Date | 5 March 2026 |
| Classification | UNCLASSIFIED |
| Preparing Organization | [PLACEHOLDER -- Preparing Organization] |
| Sponsor | [PLACEHOLDER -- Sponsor Organization] |
| Distribution Statement | [PLACEHOLDER -- Distribution Statement] |
| POC | [PLACEHOLDER -- Point of Contact] |

---

**Document Change History**

| Version | Date | Author | Description |
|---|---|---|---|
| 1.0 | 2025-06-15 | [PLACEHOLDER] | Initial draft -- battery, propulsion, VRS processors |
| 2.0 | 2025-10-01 | [PLACEHOLDER] | Added slung load processor, 6DOF mover, inter-plugin communication |
| 3.0 | 2026-03-05 | [PLACEHOLDER] | Complete architecture description, UML diagrams, design decisions |

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Model Architecture Description](#2-model-architecture-description)
3. [Component: Rotorcraft6DOF Mover](#3-component-rotorcraft6dof-mover)
4. [Component: BatteryProcessor](#4-component-batteryprocessor)
5. [Component: PropulsionProcessor](#5-component-propulsionprocessor)
6. [Component: VRSProcessor](#6-component-vrsprocessor)
7. [Component: SlungLoadProcessor](#7-component-slungloadprocessor)
8. [Design Decisions](#8-design-decisions)
9. [Installation and Build](#9-installation-and-build)
10. [Sources and References](#10-sources-and-references)
11. [Acronyms](#11-acronyms)

---

## 1. Introduction

### 1.1 Purpose

This Architecture Description Document (ADD) defines the software architecture of the T-150 UAS AFSIM Plugin Suite. The document describes the code structure, class hierarchies, inter-component communication mechanisms, plugin registration patterns, and design decisions that govern the implementation. This ADD is intended to satisfy the architecture documentation requirements of DoDI 5000.61 [1] and MIL-STD-3022 [2] for Modeling and Simulation accreditation.

This document focuses exclusively on software architecture. The underlying physics algorithms, mathematical formulations, and validation data are documented separately in the Underlying Algorithms Model (UAM) [3].

### 1.2 Scope

The T-150 Plugin Suite comprises five AFSIM plugins (six shared libraries) that collectively model a coaxial octocopter UAS within the Advanced Framework for Simulation, Integration, and Modeling (AFSIM) environment. The suite totals approximately 7,736 lines of C++ source across 26 files.

The plugins are:

| Plugin | Shared Library | Type Registered | LOC |
|---|---|---|---|
| Rotorcraft 6DOF Mover | `rotorcraft_6dof` | `WSF_ROTORCRAFT_6DOF_MOVER` | 2,939 |
| Battery Processor | `t150_battery_processor` | `T150_BATTERY_PROCESSOR` | 1,139 |
| Propulsion Processor | `t150_propulsion` | `T150_PROPULSION_PROCESSOR` | 1,281 |
| VRS Processor | `t150_vrs` | `T150_VRS_PROCESSOR` | 697 |
| Slung Load Processor | `t150_slung_load` | `T150_SLUNG_LOAD_PROCESSOR` | 825 |

### 1.3 Threat System Description

The T-150 is a heavy-lift coaxial octocopter UAS with eight motors on four arms in a coaxial configuration (two motors per arm, counter-rotating). It is designed for cargo delivery and slung-load transport missions in contested environments. Further system description is provided in [3].

### 1.4 Simulation Framework

The plugins are developed for AFSIM version 2.2505. They are compiled as shared libraries (.so on Linux, .dll on Windows) and loaded at runtime by the AFSIM plugin loader. The codebase requires C++17 or later (CMake is configured for C++20) and is compiled with GCC 13.3 on Linux and MSVC on Windows.

AFSIM provides the following base classes that the plugins extend:

- **WsfRouteMover** -- Base class for movers that follow routes composed of waypoints. Inherits from WsfMover.
- **WsfProcessor** -- Base class for processors that attach to platforms and execute per-timestep logic.
- **WsfPlatform** -- Represents a simulation entity. Hosts mover, processors, sensors, and an AuxData attribute store.
- **WsfScenario** -- Top-level container for all simulation entities and types.

### 1.5 Design Standards

- All plugins conform to the AFSIM plugin API defined by `WsfPlugin.hpp`.
- Inter-plugin communication uses the AuxData key-value store on the shared WsfPlatform, avoiding direct coupling between plugins.
- Each plugin is independently deployable; fallback values ensure graceful degradation when dependencies are unavailable.
- No global mutable state is shared between plugins beyond the AuxData mechanism.
- Naming conventions follow AFSIM patterns: `PascalCase` for classes, `mCamelCase` for member variables, `UPPER_SNAKE_CASE` for AuxData keys.

---

## 2. Model Architecture Description

### 2.1 Architecture Overview

The T-150 Plugin Suite follows a loosely-coupled component architecture. Each plugin is compiled into a separate shared library and loaded independently by AFSIM at runtime. The plugins communicate exclusively through the platform's AuxData key-value store, which acts as a shared blackboard. No plugin holds a direct reference to any other plugin's classes.

```
+------------------------------------------------------------------+
|                        AFSIM Runtime                             |
|                                                                  |
|  +-----------------------------------------------------------+  |
|  |                     WsfPlatform                            |  |
|  |                                                            |  |
|  |  +-------------------+     +---------------------------+   |  |
|  |  | WsfRouteMover     |     |     AuxData Store         |   |  |
|  |  |  (Rotorcraft6DOF) |<--->| (WsfAttributeContainer)   |   |  |
|  |  |                   |     |                           |   |  |
|  |  | +---------------+ |     |  6dof_total_power ------->|---+--+--> BatteryProcessor
|  |  | | RigidBody6DOF | |     |  SLUNG_FORCE_X/Y/Z ----->|---+--+--> (reads)
|  |  | +---------------+ |     |  PROP_TOTAL_POWER ------->|   |  |
|  |  | +---------------+ |     |  VRS_LIFT_FACTOR -------->|   |  |
|  |  | | RotorModel[8] | |     |  BATTERY_SOC ----------->|   |  |
|  |  | +---------------+ |     |  WEAPON_FORCE_X/Y/Z ---->|   |  |
|  |  +-------------------+     +---------------------------+   |  |
|  |                                    ^   ^   ^   ^           |  |
|  |  +----------------+  +----------+ | +-+-+ | +---------+   |  |
|  |  | Battery        |  | Propul-  | | |VRS| | | Slung   |   |  |
|  |  | Processor      |  | sion     | | |Pro| | | Load    |   |  |
|  |  | (WsfProcessor) |  | Proc.    | | |ces| | | Proc.   |   |  |
|  |  +----------------+  +----------+ | +---+ | +---------+   |  |
|  +-----------------------------------------------------------+  |
+------------------------------------------------------------------+
```

**Data flow direction**: Each processor writes its outputs to AuxData and reads its inputs from AuxData. The mover writes telemetry (position, velocity, thrust, power) and reads external forces (slung load, weapon recoil). The battery processor reads power from the propulsion processor or mover. This blackboard pattern ensures that any subset of plugins can operate without the others present.

### 2.2 UML Class Diagram

```
+-----------------------------+
|      WsfMover (AFSIM)       |
+-----------------------------+
              ^
              |  inherits
+-----------------------------+
|    WsfRouteMover (AFSIM)    |
|-----------------------------|
| + SetGuidanceFrom()         |
| + SetGuidanceTo()           |
| + AdvanceToNextWaypoint()   |
| + UpdatePosition()          |
| + Pause()                   |
+-----------------------------+
              ^
              |  inherits
+----------------------------------------------+
|        Rotorcraft6DOF_Mover                  |
|----------------------------------------------|
| - mRigidBody : RigidBody6DOF                |
| - mRotors : vector<RotorModel>              |
| - mConfigType : RotorConfigType             |
| - mDragConfig : DragConfig                  |
| - mGroundEffectConfig : GroundEffectConfig   |
| - mNavState : SplineNavState                |
| - mWaypointCache : vector<CachedWaypoint>   |
|----------------------------------------------|
| + ProcessInput(UtInput&) : bool              |
| + Initialize(double) : bool                 |
| + UpdatePosition(double) : void             |
| + Clone() : WsfMover*                       |
| # ApplyControlMixer() : void                |
| # UpdateAltitudeHold(double) : void          |
| # UpdatePositionHold(double) : void          |
| # PublishTelemetry() : void                  |
| # SyncToAFSIM() : void                      |
| # ReadExternalForces() : Vector3             |
| # ReadExternalMoments() : Vector3            |
| # UpdateSplineNavigation(double,double)      |
+----------------------------------------------+
          |  contains            |  contains
          v                      v
+-------------------+   +-------------------+
| RigidBody6DOF     |   | RotorModel        |
|-------------------|   |-------------------|
| - mState :        |   | - mConfig :       |
|   RigidBodyState  |   |   RotorConfig     |
| - mMassProps :    |   | - mState :        |
|   MassProperties  |   |   RotorState      |
|-------------------|   |-------------------|
| + Integrate(      |   | + Update(dt,      |
|   force,moment,dt)|   |   rho, GE) : void |
| + GetState()      |   | + GetThrustVec()  |
| + SetMassProps()  |   | + GetMomentVec()  |
+-------------------+   +-------------------+


+--------------------------+
|  WsfProcessor (AFSIM)    |
|--------------------------|
| + ProcessInput()         |
| + Initialize()           |
| + Update()               |
| + Clone()                |
| + GetPlatform()          |
+--------------------------+
     ^          ^          ^          ^
     |          |          |          |
+----------+ +----------+ +--------+ +-----------+
| Battery  | | Propul-  | | VRS    | | SlungLoad |
| Proc.    | | sionProc | | Proc.  | | Proc.     |
+----------+ +----------+ +--------+ +-----------+
```

### 2.3 Plugin Registration Pattern

All five plugins follow the same two-level registration pattern to register their types with AFSIM. This is the standard AFSIM plugin API contract.

**Registration sequence (pseudocode):**

```
AFSIM Runtime loads shared library (.so / .dll)
  |
  +--> calls extern "C" WsfPluginVersion(version)
  |      Sets major/minor version and ABI string
  |      AFSIM verifies plugin API compatibility
  |
  +--> calls extern "C" WsfPluginSetup(application)
         |
         +--> Check if extension already registered
         |
         +--> application.RegisterFeature("plugin_name")
         |
         +--> application.RegisterExtension(
         |      "extension_name",
         |      make_unique<ApplicationExtension>())
         |
         +--> ApplicationExtension::ScenarioCreated(scenario)
                |
                +--> scenario.RegisterExtension(
                       "extension_name",
                       make_unique<ScenarioExtension>())
                |
                +--> ScenarioExtension::AddedToScenario()
                       |
                       +--> For Mover:
                       |      WsfMoverTypes::Get(scenario).AddCoreType(
                       |        "TYPE_NAME", make_unique<MoverClass>(scenario))
                       |
                       +--> For Processor:
                              WsfProcessorTypes::Get(scenario).AddCoreType(
                                "TYPE_NAME", make_unique<ProcessorClass>(scenario))
```

**Preconditions**: AFSIM must be initialized and the plugin shared library must be locatable via the plugin search path or scenario `plugin_directory` directive.

**Expected outcomes**: After registration, the type name (e.g., `WSF_ROTORCRAFT_6DOF_MOVER` or `T150_BATTERY_PROCESSOR`) becomes available for use in scenario files.

The 6DOF mover plugin uses the `WsfDefaultApplicationExtension` template helper, while the four processor plugins define a custom `ApplicationExtension` class with an identical pattern. Both approaches produce the same result: a `ScenarioExtension` is attached to each new scenario to register the plugin's type.

### 2.4 Inter-Processor Communication via AuxData

Plugins communicate through the platform's `WsfAttributeContainer` (AuxData). Each plugin writes its outputs as named key-value pairs and reads inputs from keys published by other plugins. The following table catalogs all AuxData keys in the suite.

**Mover (Rotorcraft6DOF) -- Published Keys:**

| Key | Type | Description |
|---|---|---|
| `6dof_pos_n`, `_e`, `_d` | double | NED position [m] |
| `6dof_vel_n`, `_e`, `_d` | double | NED velocity [m/s] |
| `6dof_roll_deg`, `_pitch_deg`, `_yaw_deg` | double | Euler angles [deg] |
| `6dof_p_dps`, `_q_dps`, `_r_dps` | double | Angular rates [deg/s] |
| `6dof_total_thrust` | double | Sum of all rotor thrust [N] |
| `6dof_total_power` | double | Sum of all rotor power [W] |
| `6dof_throttle_cmd` | double | Throttle command [0-1] |
| `6dof_avg_rpm` | double | Average rotor RPM |
| `6dof_mass`, `6dof_weight` | double | Current mass [kg], weight [N] |

**Mover (Rotorcraft6DOF) -- Read Keys:**

| Key | Source Plugin | Description |
|---|---|---|
| `SLUNG_FORCE_X/Y/Z` | SlungLoadProcessor | Cable reaction forces [N] |
| `WEAPON_FORCE_X/Y/Z` | WeaponProcessor | Recoil forces [N] |
| `WEAPON_MOMENT_X/Y/Z` | WeaponProcessor | Recoil moments [Nm] |
| `WEAPON_REGISTRY` | WeaponProcessor | Comma-separated weapon names |
| `WEAPON_<name>_MASS_DELTA` | WeaponProcessor | Ammunition mass change [kg] |
| `SLUNG_LOAD_STATE` | SlungLoadProcessor | Load state enumeration |
| `SLUNG_LOAD_MASS` | SlungLoadProcessor | Load mass for dynamic mass update [kg] |

**BatteryProcessor -- Published Keys:**

| Key | Type | Description |
|---|---|---|
| `BATTERY_SOC` | double | State of charge [0-1] |
| `BATTERY_POWER_W` | double | Current power draw [W] |
| `BATTERY_ENERGY_WH` | double | Remaining energy [Wh] |
| `BATTERY_VOLTAGE` | double | Terminal voltage [V] |
| `BATTERY_OCV` | double | Open-circuit voltage [V] |
| `BATTERY_VOLTAGE_SAG` | double | Voltage drop from internal resistance [V] |
| `BATTERY_CURRENT_A` | double | Pack current [A] |
| `BATTERY_LOW` | bool | Low SOC warning flag |
| `BATTERY_VOLTAGE_LOW` | bool | Voltage sag warning flag |

**BatteryProcessor -- Read Keys:**

| Key | Source Plugin | Description |
|---|---|---|
| `PROP_TOTAL_POWER` | PropulsionProcessor | Propulsion power [W] (primary source) |
| `6dof_total_power` | Rotorcraft6DOF Mover | Mover power [W] (fallback source) |

**PropulsionProcessor -- Published Keys:**

| Key | Type | Description |
|---|---|---|
| `PROP_TOTAL_THRUST` | double | Total thrust [N] |
| `PROP_TOTAL_POWER` | double | Total electrical power [W] |
| `PROP_SYSTEM_EFF` | double | System efficiency [0-1] |
| `PROP_MOTOR_COUNT` | int | Operational motor count |
| `PROP_GROUND_EFFECT` | double | Ground effect multiplier |
| `PROP_ALTITUDE_AGL` | double | Altitude AGL [m] |
| `PROP_TOTAL_MASS` | double | Total mass [kg] |
| `PROP_PAYLOAD_MASS` | double | Payload mass [kg] |
| `PROP_THRUST_MARGIN` | double | Available thrust margin [0-1] |
| `PROP_CG_OFFSET_X/Y/Z` | double | Combined CG offset [m] |
| `PROP_FORWARD_SPEED` | double | Horizontal speed [m/s] |
| `PROP_CLIMB_RATE` | double | Vertical speed [m/s] |
| `PROP_BANK_ANGLE` | double | Bank angle [deg] |
| `PROP_OVERLOADED` | bool | Payload exceeds capacity |
| `MOTOR_OVERTEMP` | bool | Motor over-temperature flag |
| `MOTOR_FAILURE` | bool | Motor failure flag |

**VRSProcessor -- Published Keys:**

| Key | Type | Description |
|---|---|---|
| `VRS_MARGIN` | double | VRS proximity margin [0-1] |
| `VRS_STATE` | int | VRS state enumeration |
| `VRS_LIFT_FACTOR` | double | Lift degradation factor [0-1] |
| `VRS_CONTROL_FACTOR` | double | Control authority factor [0-1] |
| `VRS_DEVELOPMENT` | double | VRS development level [0-1] |
| `VRS_DESCENT_RATE` | double | Current descent rate [m/s] |
| `VRS_FORWARD_SPEED` | double | Current forward speed [m/s] |
| `VRS_CAUTION`, `_ONSET`, `_CRITICAL` | bool | Warning flags |

**SlungLoadProcessor -- Published Keys:**

| Key | Type | Description |
|---|---|---|
| `SLUNG_LOAD_STATE` | int | Load state enumeration |
| `SLUNG_LOAD_MASS` | double | Load mass [kg] |
| `SLUNG_CABLE_LENGTH` | double | Cable length [m] |
| `SLUNG_CABLE_TENSION` | double | Cable tension [N] |
| `SLUNG_SWING_X`, `_Y` | double | Swing angles [deg] |
| `SLUNG_SWING_MAG` | double | Swing magnitude [deg] |
| `SLUNG_FORCE_X/Y/Z` | double | Reaction forces on aircraft [N] |
| `SLUNG_POS_X/Y/Z` | double | Load position relative to CG [m] |
| `SLUNG_RESONANCE` | bool | Resonance warning flag |
| `SLUNG_NATURAL_FREQ` | double | Pendulum natural frequency [Hz] |

---

## 3. Component: Rotorcraft6DOF Mover

### 3.1 Structural Overview

The Rotorcraft6DOF Mover is the core flight dynamics component. It extends `WsfRouteMover` (which itself extends `WsfMover`) to inherit route/waypoint navigation integration while replacing the position update logic with full 6DOF physics.

**Source files:**

| File | Purpose | LOC |
|---|---|---|
| `Rotorcraft6DOF_Mover.hpp` | Class declaration, enums, structs | 489 |
| `Rotorcraft6DOF_Mover.cpp` | Implementation | ~2,450 |
| `RigidBody6DOF.hpp` | Rigid body state and integrator | 214 |
| `RigidBody6DOF.cpp` | Integration implementation | ~150 |
| `RotorModel.hpp` | Single rotor model | 181 |
| `RotorModel.cpp` | Rotor physics implementation | ~120 |
| `MathUtils.hpp` | Vector3, Quaternion, Matrix3x3, spline math | 511 |
| `Rotorcraft6DOF_Registration.hpp` | Scenario extension | 56 |
| `Rotorcraft6DOF_PluginRegistration.cpp` | Plugin entry points | 57 |

**Namespace**: `rotorcraft_6dof`

### 3.2 Composition Structure

The mover contains the following owned objects:

- **RigidBody6DOF** (`mRigidBody`) -- Holds the kinematic state (position, velocity, orientation quaternion, angular velocity) and mass properties. Provides the `Integrate()` method that advances state by one timestep given total forces and moments.
- **vector\<RotorModel\>** (`mRotors`) -- Array of N rotor models, sized by configuration (4 for QUAD, 8 for COAXIAL_QUAD). Each rotor independently computes thrust, torque, and power from its commanded RPM.
- **DragConfig** (`mDragConfig`) -- Aerodynamic drag coefficients and reference area.
- **GroundEffectConfig** (`mGroundEffectConfig`) -- Ground effect parameters.
- **map\<int, WaypointConfig\>** (`mWaypointConfigs`) -- Per-waypoint behavior overrides for spline navigation.
- **vector\<CachedWaypoint\>** (`mWaypointCache`) -- Sliding window of four waypoints for Catmull-Rom spline interpolation.

### 3.3 WsfRouteMover Interface

The mover implements the pure virtual methods required by `WsfRouteMover`:

| Method | Behavior |
|---|---|
| `SetGuidanceFrom(waypoint)` | Caches the previous waypoint for speed reference. |
| `SetGuidanceTo(waypoint)` | Sets target altitude and speed from the next waypoint. |
| `AdvanceToNextWaypoint(time, target, next)` | Configures guidance for the next segment. If spline navigation is enabled, advances the waypoint cache and recomputes the speed profile. |
| `UpdatePosition(simTime)` | Core physics loop -- computes forces, integrates rigid body, synchronizes AFSIM platform state. |
| `Pause(simTime, unpauseTime)` | Enters hover-hold at current position until `unpauseTime`. |

### 3.4 UpdatePosition Sequence

The `UpdatePosition()` method executes the following sequence each timestep:

```
UpdatePosition(simTime)
  |
  +--> Compute dt = simTime - mLastSimTime
  |
  +--> Update air density from altitude (ISA model)
  |
  +--> IF spline navigation enabled:
  |      UpdateSplineNavigation(dt, simTime)
  |    ELSE:
  |      UpdatePositionHold(dt)   -- waypoint tracking
  |      UpdateAltitudeHold(dt)   -- altitude PID
  |
  +--> ApplyControlMixer()
  |      Maps throttle/roll/pitch/yaw commands to per-rotor RPM
  |      RPM_i = hover_RPM * (throttle + roll*mix_i + pitch*mix_i + yaw*mix_i)
  |
  +--> FOR EACH rotor:
  |      rotor.Update(dt, airDensity, groundEffectFactor)
  |        Motor first-order lag: actualRPM += (cmdRPM - actualRPM) * dt / tau
  |        Compute thrust, torque, power from actualRPM
  |
  +--> ComputeTotalThrustForce()    -- sum rotor thrust vectors (body frame)
  +--> ComputeTotalRotorMoment()    -- sum rotor moments (body frame)
  +--> ComputeDragForce(bodyVel)    -- aerodynamic drag (body frame)
  +--> ReadExternalForces()         -- slung load + weapon forces from AuxData
  +--> ReadExternalMoments()        -- weapon moments from AuxData
  |
  +--> Add gravity in world frame, rotate to body frame
  +--> Sum all forces and moments
  |
  +--> mRigidBody.Integrate(totalForce, totalMoment, dt)
  |      Compute linear acceleration: a = F / m
  |      Compute angular acceleration: alpha = I^{-1} (M - omega x I*omega)
  |      Integrate velocity, position (Euler method)
  |      Integrate quaternion: q_dot = 0.5 * q * omega
  |      Normalize quaternion, update Euler angles
  |
  +--> UpdateDynamicMass()          -- weapon ammo + slung load mass changes
  +--> SyncToAFSIM()               -- copy 6DOF state to WsfPlatform position
  +--> PublishTelemetry()           -- write state to AuxData
```

### 3.5 Spline Navigation State Machine

When spline navigation is enabled, the mover uses a Catmull-Rom spline interpolation across a sliding window of four cached waypoints (P0, P1, P2, P3) to produce smooth flight paths. The navigation state machine governs behavior at each waypoint.

```
Statechart: SplineNavState

    +---> APPROACHING ---[reached P2]--->+
    |         |                          |
    |         |                   [check behavior]
    |         |                     /    |     \
    |         |            FLY_THROUGH SLOW   STOP_AND
    |         |              |       _PASS    _HOVER
    |         |              |         |         |
    |         |              |         |         v
    |         |              |         |    AT_WAYPOINT
    |         |              |         |      |  [duration elapsed]
    |         |              |         |      v
    |         |              |         |    ROTATING
    |         |              |         |      |  [heading achieved]
    |         |              v         v      v
    |         |              +----DEPARTING---+
    |         |                     |
    |         +-----[advance cache]-+
    |                               |
    +-------------------------------+
```

**State descriptions:**

| State | Precondition | Behavior | Exit Condition |
|---|---|---|---|
| APPROACHING | Waypoint cache initialized | Follow spline toward P2 at profiled speed | Distance to P2 < arrival radius |
| AT_WAYPOINT | STOP_AND_HOVER behavior | Hold position, zero velocity | Hover duration elapsed |
| ROTATING | Departure heading specified | Yaw to face next waypoint | Heading error < threshold |
| DEPARTING | Rotation complete or FLY_THROUGH | Advance waypoint cache, accelerate | Spline parameter reset, transition to APPROACHING |

### 3.6 Rotor Configuration System

The mover supports multiple rotor configurations through the `RotorConfigType` enumeration:

| Configuration | Rotors | Arms | Description |
|---|---|---|---|
| `QUAD_X` | 4 | 4 | X-pattern quadcopter |
| `QUAD_PLUS` | 4 | 4 | Plus-pattern quadcopter |
| `HEX_X` | 6 | 6 | Hexacopter |
| `OCTO_X` | 8 | 8 | Flat octocopter |
| `COAXIAL_QUAD` | 8 | 4 | Coaxial quadcopter (T-150 default) |
| `COAXIAL_OCTO` | 16 | 8 | Coaxial octocopter |
| `CUSTOM` | N | N | User-defined rotor positions |

Each `Setup*()` method populates the `mRotors` vector with `RotorModel` instances whose `RotorConfig` specifies body-frame position, rotation direction (CW/CCW), thrust axis, and control mixing factors (roll, pitch, yaw).

### 3.7 Public Interface Summary

| Method | Precondition | Expected Outcome |
|---|---|---|
| `ProcessInput(input)` | Scenario file parsed | Configuration loaded: mass, rotors, gains, spline settings |
| `Initialize(simTime)` | ProcessInput complete | 6DOF state initialized from AFSIM platform position, hover RPM computed |
| `UpdatePosition(simTime)` | Initialize complete | Platform position/velocity/attitude advanced by one timestep |
| `SetRotorConfigType(type)` | Before Initialize | Rotor array populated with preset geometry |
| `GetTotalThrust()` | Any time after Initialize | Returns sum of all rotor thrust values [N] |
| `GetTotalPower()` | Any time after Initialize | Returns sum of all rotor power values [W] |

---

## 4. Component: BatteryProcessor

### 4.1 Structural Overview

The BatteryProcessor models a lithium-ion battery pack, tracking state-of-charge (SOC), terminal voltage under load, and energy consumption. It extends `WsfProcessor` and follows the standard ProcessInput/Initialize/Update lifecycle.

**Source files:**

| File | Purpose |
|---|---|
| `BatteryProcessor.hpp` | Class declaration |
| `BatteryProcessor.cpp` | Implementation |
| `BatteryRegistration.hpp` | Scenario extension |
| `BatteryPluginRegistration.cpp` | Plugin entry points |

**Namespace**: `t150`

### 4.2 Lifecycle

```
Sequence: BatteryProcessor Lifecycle

AFSIM              BatteryProcessor
  |                      |
  |-- ProcessInput() --->|  Parse: capacity_wh, initial_soc,
  |                      |         voltage model params, CSV config,
  |                      |         power_source_key, auxiliary_power_w
  |                      |
  |-- Initialize() ----->|  Set initial SOC, compute OCV,
  |                      |  open CSV file, publish initial AuxData
  |                      |
  |-- Update(simTime) -->|  [per timestep, repeated]
  |                      |    Read power from AuxData source
  |                      |    Apply motor efficiency, add auxiliary power
  |                      |    Compute energy consumed = power * dt
  |                      |    Update SOC = SOC - (energy / capacity)
  |                      |    Compute OCV from SOC lookup
  |                      |    Solve for terminal voltage (quadratic)
  |                      |    Publish state to AuxData
  |                      |    Write CSV row at configured interval
  |                      |    Check low-battery / voltage-sag warnings
  |                      |
  |-- ~Destructor() ---->|  Write CSV summary, close file
```

### 4.3 Power Source Resolution

The battery processor reads power consumption through a configurable AuxData key. The resolution order provides fallback behavior:

```
  Read power from AuxData:
    1. Try mPowerSourceKey (configurable, default "PROP_TOTAL_POWER")
    2. If not found or zero, try "6dof_total_power" (from mover)
    3. If not found or zero, try "PROP_TOTAL_POWER" (from propulsion)
    4. If none available, use mHoverPowerW (static fallback)
```

This cascade ensures the battery processor functions regardless of which other plugins are loaded.

### 4.4 Voltage Sag Solver

The terminal voltage under load is computed by solving for the current that satisfies both:
- `V_terminal = V_oc - I * R_internal`
- `P = V_terminal * I`

Substituting yields a quadratic in voltage. The processor selects the physically meaningful root (higher voltage). If the quadratic has no real roots (power demand exceeds what the battery can deliver), the terminal voltage is clamped to the cutoff voltage.

### 4.5 Public Interface Summary

| Method | Precondition | Expected Outcome |
|---|---|---|
| `GetSOC()` | After Initialize | Returns current state of charge [0-1] |
| `GetVoltage()` | After Initialize | Returns terminal voltage [V] |
| `GetCurrent()` | After Update | Returns pack current [A] |
| `GetVoltageSag()` | After Update | Returns voltage drop from internal resistance [V] |
| `GetRemainingEnergy()` | After Update | Returns SOC * capacity [Wh] |
| `GetCurrentPower()` | After Update | Returns power draw this timestep [W] |

---

## 5. Component: PropulsionProcessor

### 5.1 Structural Overview

The PropulsionProcessor models the electric motor/ESC/propeller system at a subsystem level. It tracks per-motor state including RPM, current, temperature, and efficiency. It extends `WsfProcessor`.

**Source files:**

| File | Purpose |
|---|---|
| `PropulsionProcessor.hpp` | Class declaration, MotorData struct, MotorState enum |
| `PropulsionProcessor.cpp` | Implementation |
| `PropulsionRegistration.hpp` | Scenario extension |
| `PropulsionPluginRegistration.cpp` | Plugin entry points |

**Namespace**: `t150`

### 5.2 Motor State Array

The processor maintains a fixed-size array of motor states:

```
  std::array<MotorData, NUM_MOTORS> mMotors    (NUM_MOTORS = 8)

  MotorData contains:
    state       : MotorState {OFF, STARTING, RUNNING, OVERTEMP, FAILED}
    rpm         : double     -- current RPM
    current     : double     -- current draw [A]
    temperature : double     -- winding temperature [C]
    efficiency  : double     -- operating efficiency [0-1]
    thrust      : double     -- thrust output [N]
    torque      : double     -- torque output [Nm]
    throttle    : double     -- commanded throttle [0-1]
```

### 5.3 Update Loop

```
  Update(simTime):
    dt = simTime - mLastUpdateTime

    Read platform velocity from mover state
    Compute acceleration by finite differencing velocity
    Compute forward speed, climb rate, bank angle estimate

    Compute required thrust = ComputeRequiredThrust(simTime)
      Accounts for weight, payload, acceleration demand

    Compute ground effect factor from altitude AGL

    DistributeThrust(totalThrust)
      Divide among operational motors
      Apply coaxial efficiency factor for lower rotors

    FOR i = 0 to NUM_MOTORS-1:
      UpdateMotor(i, dt)
        Apply motor time constant (first-order lag on RPM)
        Compute thrust, torque from RPM
        Compute motor efficiency at operating point
        Compute electrical current
      UpdateMotorTemperature(i, dt, power)
        Thermal model: dT/dt = (P_loss - cooling) / thermal_mass
      CheckMotorHealth(i, simTime)
        If temperature > limit: transition to OVERTEMP
        If OVERTEMP persists: transition to FAILED

    Sum total thrust, total power, system efficiency
    Compute dynamic power from flight state (Phase 6)
    Publish all state to AuxData
    Write CSV row at configured interval
```

### 5.4 MotorState Machine

```
Statechart: MotorState

    OFF --[power on]--> STARTING --[RPM > idle]--> RUNNING
                                                     |
                                          [temp > limit]
                                                     v
                                                  OVERTEMP
                                                     |
                                          [temp still high after timeout]
                                                     v
                                                   FAILED
                                                  (terminal)
```

Transitions are governed by motor temperature relative to the configurable thermal limit. A failed motor produces zero thrust and is excluded from thrust distribution.

### 5.5 Public Interface Summary

| Method | Precondition | Expected Outcome |
|---|---|---|
| `GetTotalThrust()` | After Update | Returns sum of all motor thrust [N] |
| `GetTotalPower()` | After Update | Returns total electrical power [W] |
| `GetOperationalMotorCount()` | Any time | Returns count of motors not in FAILED state |
| `GetMotorData(index)` | 0 <= index < NUM_MOTORS | Returns reference to individual motor state |
| `GetMaxMotorTemperature()` | After Update | Returns highest motor temperature [C] |
| `FailMotor(index)` | Motor is RUNNING | Forces motor to FAILED state |
| `GetThrustMargin()` | After Update | Returns available thrust margin [0-1] |

---

## 6. Component: VRSProcessor

### 6.1 Structural Overview

The VRSProcessor detects and models Vortex Ring State conditions by monitoring descent rate and forward speed. It extends `WsfProcessor`.

**Source files:**

| File | Purpose |
|---|---|
| `VRSProcessor.hpp` | Class declaration, VRSState enum |
| `VRSProcessor.cpp` | Implementation |
| `VRSRegistration.hpp` | Scenario extension |
| `VRSPluginRegistration.cpp` | Plugin entry points |

**Namespace**: `t150`

### 6.2 VRS State Machine

```
Statechart: VRSState

    CLEAR --[margin < caution_threshold]--> CAUTION
      ^                                       |
      |                            [margin < onset_threshold]
      |                                       v
      |                                     ONSET
      |                                       |
      |                       [development_time elapsed]
      |                                       v
      +---[recovery_time elapsed]--- RECOVERY <--- DEVELOPED
                                       ^               |
                                       +---[exit VRS]--+
```

| State | Condition | Effect |
|---|---|---|
| CLEAR | VRS margin > caution threshold | No degradation (lift factor = 1.0) |
| CAUTION | Margin between caution and onset thresholds | Warning issued, no degradation yet |
| ONSET | Margin below onset threshold | Partial lift degradation begins |
| DEVELOPED | Onset sustained for development time | Full lift and control degradation applied |
| RECOVERY | Conditions improving | Gradual restoration of lift and control |

### 6.3 Velocity Reading

The VRS processor reads descent rate and forward speed from the platform's mover state (via AFSIM's `WsfPlatform` API), not from AuxData. It calls the platform's velocity accessors to obtain NED velocity components, then derives descent rate (negative of down velocity) and horizontal speed.

### 6.4 Public Interface Summary

| Method | Precondition | Expected Outcome |
|---|---|---|
| `GetVRSState()` | After Update | Returns current VRSState enumeration |
| `GetVRSMargin()` | After Update | Returns proximity to VRS [0-1], lower = closer |
| `GetLiftFactor()` | After Update | Returns lift multiplier [0-1], 1.0 = no degradation |
| `GetControlFactor()` | After Update | Returns control authority [0-1] |
| `IsInVRS()` | After Update | Returns true if state == DEVELOPED |
| `GetTimeInState()` | After Update | Returns seconds in current state |

---

## 7. Component: SlungLoadProcessor

### 7.1 Structural Overview

The SlungLoadProcessor models a cable-suspended payload as a spherical pendulum. It computes swing dynamics, cable tension, and reaction forces that are published to AuxData for the mover to read. It extends `WsfProcessor`.

**Source files:**

| File | Purpose |
|---|---|
| `SlungLoadProcessor.hpp` | Class declaration, SlungLoadState enum |
| `SlungLoadProcessor.cpp` | Implementation |
| `SlungLoadRegistration.hpp` | Scenario extension |
| `SlungLoadPluginRegistration.cpp` | Plugin entry points |

**Namespace**: `t150`

### 7.2 Pendulum Integration Loop

Each timestep, the processor:

```
  Update(simTime):
    IF loadState is DETACHED or RELEASED:
      return (no dynamics to compute)

    dt = simTime - mLastUpdateTime

    Read platform velocity from mover state
    Compute aircraft acceleration by finite differencing velocity

    UpdatePendulumDynamics(dt, accelX, accelY, accelZ)
      Compute angular accelerations:
        theta_x'' = -(g/L)*sin(theta_x) - (a_x/L)*cos(theta_x) - c*theta_x'
        theta_y'' = -(g/L)*sin(theta_y) - (a_y/L)*cos(theta_y) - c*theta_y'
      Integrate with semi-implicit Euler:
        theta_x' += theta_x'' * dt
        theta_y' += theta_y'' * dt
        theta_x  += theta_x'  * dt
        theta_y  += theta_y'  * dt

    ComputeCableTension(accelZ)
      T = m * sqrt((g + a_z)^2 + (L*theta_x'')^2 + (L*theta_y'')^2)

    ComputeLoadPosition()
      Convert swing angles to Cartesian offset from attach point

    Compute reaction forces on aircraft (Newton's 3rd law):
      F_x = -T * sin(theta_x)
      F_y = -T * sin(theta_y)
      F_z = -T * cos(theta_x) * cos(theta_y)

    CheckResonance(dt)
      Track zero crossings to measure swing frequency
      Flag if swing angle exceeds resonance threshold

    Publish all state to AuxData (forces, angles, tension, position)
```

### 7.3 SlungLoad State Machine

```
Statechart: SlungLoadState

    DETACHED --[AttachLoad(mass)]--> ATTACHED
                                      |
                              [swing > threshold]
                                      v
                                   SWINGING
                                      |
                              [resonance detected]
                                      v
                                   RESONANCE
                                      |
    RELEASED <--[ReleaseLoad()]-- any attached state
```

| State | Condition | Behavior |
|---|---|---|
| DETACHED | No load | No dynamics computed |
| ATTACHED | Load connected, small swing | Normal pendulum dynamics |
| SWINGING | Swing angle > threshold | Warning issued, dynamics continue |
| RESONANCE | Measured frequency near natural frequency | Critical warning |
| RELEASED | Load intentionally dropped | Forces zeroed, state frozen |

### 7.4 Force Coupling to Mover

The slung load processor publishes `SLUNG_FORCE_X/Y/Z` to AuxData. The Rotorcraft6DOF mover reads these keys in its `ReadExternalForces()` method and adds them to the total force vector before integration. This creates a bidirectional coupling: aircraft motion drives pendulum dynamics (via acceleration), and pendulum forces affect aircraft motion (via AuxData).

### 7.5 Public Interface Summary

| Method | Precondition | Expected Outcome |
|---|---|---|
| `AttachLoad(mass)` | State is DETACHED | Transitions to ATTACHED, sets load mass |
| `ReleaseLoad()` | State is ATTACHED/SWINGING/RESONANCE | Transitions to RELEASED, zeroes forces |
| `GetSwingMagnitude()` | After Update | Returns RSS swing angle [rad] |
| `GetCableTension()` | After Update | Returns cable tension [N] |
| `GetLoadForceX/Y/Z()` | After Update | Returns reaction force components [N] |
| `IsResonanceWarning()` | After Update | Returns true if resonance detected |

---

## 8. Design Decisions

### 8.1 Decision Log

| ID | Decision | Rationale | Alternatives Considered |
|---|---|---|---|
| DD-01 | Extend WsfRouteMover rather than WsfMover | WsfRouteMover provides built-in route/waypoint management, arrival detection, and pause/resume. Extending WsfMover directly would require reimplementing all route logic. | WsfMover (rejected: too much route logic to reimplement) |
| DD-02 | AuxData blackboard for inter-plugin communication | Decouples plugins; any subset can run independently. Plugins use fallback values when expected AuxData keys are absent. | Direct object references (rejected: creates build-time coupling); AFSIM message bus (rejected: higher complexity, not standard for processor-to-processor data) |
| DD-03 | Quaternion attitude representation | Avoids gimbal lock inherent in Euler angle integration. Quaternion normalization is computationally cheap. Euler angles are derived only for telemetry output. | Euler angles (rejected: gimbal lock at 90-deg pitch); rotation matrices (rejected: 9 elements vs 4, more expensive to normalize) |
| DD-04 | Custom math library (MathUtils.hpp) | AFSIM's built-in math (UtVec3) uses different conventions. A self-contained Vector3/Quaternion/Matrix3x3 library ensures NED consistency and avoids API breakage across AFSIM versions. | UtVec3/UtQuat (rejected: convention mismatch); Eigen (rejected: large dependency for plugin deployment) |
| DD-05 | Per-plugin shared library | Each plugin can be deployed, updated, and tested independently. Scenario files load only the plugins needed. | Monolithic library (rejected: all-or-nothing deployment); header-only (rejected: AFSIM plugins must be shared libraries) |
| DD-06 | First-order motor lag | Captures motor spin-up/spin-down dynamics with a single time constant parameter. Good fidelity-to-complexity ratio for engagement-level simulation. | Instantaneous response (rejected: unrealistic); second-order dynamics (rejected: added complexity with minimal fidelity gain at this resolution) |
| DD-07 | Semi-implicit Euler for pendulum integration | Unconditionally stable for the slung load pendulum, simple to implement. Energy dissipation from damping term prevents numerical divergence. | RK4 (considered: more accurate but pendulum damping makes Euler adequate); symplectic Euler (equivalent for this application) |
| DD-08 | Catmull-Rom splines for path interpolation | C1-continuous curves through waypoints without requiring user-specified tangents. Tension parameter provides tuning. Sliding 4-point window keeps memory bounded. | Linear interpolation (rejected: abrupt heading changes); B-splines (rejected: do not pass through control points); Bezier (rejected: require tangent specification) |
| DD-09 | Configurable power source key | Battery processor can read power from any AuxData key, allowing it to pair with the propulsion processor, the 6DOF mover, or a custom source without code changes. | Hard-coded key (rejected: limits reuse); direct processor reference (rejected: coupling) |
| DD-10 | generate_export_header() for DLL support | CMake's `GenerateExportHeader` module automatically generates the correct `__declspec(dllexport)` / visibility macros for cross-platform builds. | Manual export macros (rejected: error-prone); WINDOWS_EXPORT_ALL_SYMBOLS only (rejected: fragile with templates) |

---

## 9. Installation and Build

### 9.1 Build System

The plugin suite uses CMake (minimum version 3.16) with the following structure:

```
  plugins/
    CMakeLists.txt              <-- Top-level: finds AFSIM, adds subdirectories
    rotorcraft_6dof/
      CMakeLists.txt            <-- Builds rotorcraft_6dof.so / .dll
      source/                   <-- 5 .hpp + 4 .cpp files
      grammar/                  <-- AFSIM grammar file (.ag)
    t150_battery/
      CMakeLists.txt            <-- Builds t150_battery_processor.so / .dll
      source/                   <-- 3 .hpp + 2 .cpp files
      grammar/
    t150_propulsion/
      CMakeLists.txt
      source/                   <-- 2 .hpp + 2 .cpp files
      grammar/
    t150_vrs/
      CMakeLists.txt
      source/                   <-- 2 .hpp + 2 .cpp files
      grammar/
    t150_slung_load/
      CMakeLists.txt
      source/                   <-- 2 .hpp + 2 .cpp files
      grammar/
```

### 9.2 CMake Configuration

Each plugin's `CMakeLists.txt` follows the same pattern:

```
  1. Define PLUGIN_NAME
  2. Glob source files (*.cpp, *.hpp)
  3. Create shared library: add_library(${PLUGIN_NAME} SHARED ...)
  4. Generate export header: generate_export_header(${PLUGIN_NAME} ...)
  5. Set include directories: source/, build dir, AFSIM includes
  6. Link AFSIM::wsf
  7. Set properties: POSITION_INDEPENDENT_CODE, PREFIX "", WINDOWS_EXPORT_ALL_SYMBOLS
  8. Install to platform-appropriate directory:
       Linux:  lib/wsf_plugins/
       Windows: bin/mission_plugins/
```

The top-level `CMakeLists.txt` detects the AFSIM installation via `find_package(afsim REQUIRED)` and sets the C++ standard to C++20 (AFSIM 2.2505 requirement).

### 9.3 Build and Install Scripts

The project provides NGIC convenience scripts at the repository root:

| Script | Purpose |
|---|---|
| `ngic_afsim_build.sh` | Auto-detects AFSIM installation, runs CMake configure and build |
| `ngic_afsim_install.sh` | Installs built plugins to AFSIM system directories (requires sudo) |

**Build procedure (pseudocode):**

```
  cd plugins/
  mkdir -p build && cd build
  cmake .. -DAFSIM_ROOT=/opt/afsim2.2505
  cmake --build . --parallel
```

**Install destinations:**

| Platform | Plugin Directory | Grammar Directory |
|---|---|---|
| Linux | `/opt/afsim2.2505/lib/wsf_plugins/` | `/opt/afsim2.2505/bin/grammar/` |
| Windows | `C:\AFSIM\afsim2.2505\bin\mission_plugins\` | `C:\AFSIM\afsim2.2505\bin\grammar\` |

### 9.4 Compiler Requirements

| Compiler | Minimum Version | Platform |
|---|---|---|
| GCC | 13.3 | Linux (AlmaLinux 9 WSL) |
| MSVC | 2022 (v143) | Windows 11 |
| CMake | 3.16 | Both |
| C++ Standard | C++17 (CMake configured for C++20) | Both |

---

## 10. Sources and References

| Ref | Citation |
|---|---|
| [1] | Department of Defense Instruction 5000.61, "DoD Modeling and Simulation (M&S) Verification, Validation, and Accreditation (VV&A)," current version. |
| [2] | MIL-STD-3022, "Documentation of Verification, Validation, and Accreditation (VV&A) for Models and Simulations," current version. |
| [3] | T-150 UAS AFSIM Plugin Suite -- Underlying Algorithms Model (UAM), Version 3.0, March 2026. |
| [4] | AFSIM User's Guide, Version 2.2505, Air Force Research Laboratory, 2025. |
| [5] | Johnson, W. *Helicopter Theory*, Dover Publications, 1994. |
| [6] | Leishman, J. G. *Principles of Helicopter Aerodynamics*, 2nd ed., Cambridge University Press, 2006. |
| [7] | Brand, A. et al. "Vortex Ring State (VRS) Prediction," AIAA Paper, 2004. |
| [8] | Cicolani, L. S. et al. "Simulation of the Dynamics of Helicopter Slung Load Systems," NASA/TP-2009-215656, 2009. |
| [9] | Tremblay, O. and Bhatt, A. "Experimental Validation of a Battery Dynamic Model for EV Applications," *World Electric Vehicle Journal*, Vol. 3, 2009. |
| [10] | T-150 UAS System Specification, [PLACEHOLDER -- Document ID], current version. |
| [11] | AFSIM Plugin Developer's Guide, Air Force Research Laboratory, 2025. |

---

## 11. Acronyms

| Acronym | Definition |
|---|---|
| ADD | Architecture Description Document |
| AFSIM | Advanced Framework for Simulation, Integration, and Modeling |
| AGL | Above Ground Level |
| API | Application Programming Interface |
| CG | Center of Gravity |
| CMake | Cross-platform build system |
| CW/CCW | Clockwise / Counter-Clockwise |
| DLL | Dynamic Link Library |
| DoD | Department of Defense |
| DoDI | Department of Defense Instruction |
| DOF | Degrees of Freedom |
| ESC | Electronic Speed Controller |
| GCC | GNU Compiler Collection |
| GE | Ground Effect |
| ISA | International Standard Atmosphere |
| LOC | Lines of Code |
| MSL | Mean Sea Level |
| MSVC | Microsoft Visual C++ |
| NED | North-East-Down (coordinate frame) |
| NGIC | National Ground Intelligence Center |
| OCV | Open-Circuit Voltage |
| PID | Proportional-Integral-Derivative (controller) |
| RPM | Revolutions Per Minute |
| SOC | State of Charge |
| UAS | Unmanned Aircraft System |
| UAM | Underlying Algorithms Model |
| UML | Unified Modeling Language |
| VRS | Vortex Ring State |
| VV&A | Verification, Validation, and Accreditation |
| WSF | Warfighter Simulation Framework |
