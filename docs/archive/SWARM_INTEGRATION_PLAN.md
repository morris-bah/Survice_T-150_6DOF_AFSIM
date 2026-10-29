# T-150 Physics Plugins + Drone Swarm Integration Plan

## Executive Summary

This plan integrates the T-150 UAS physics plugins (battery, VRS, propulsion, slung load) with the drone swarm plugin to create a **physics-aware swarm** capable of realistic autonomous behavior based on actual system state.

---

## Current State

### T-150 Physics Plugins (project005_t150_uas)
| Plugin | Processor Type | Key AuxData Outputs |
|--------|---------------|---------------------|
| t150_battery | T150_BATTERY_PROCESSOR | BATTERY_SOC, BATTERY_POWER, BATTERY_VOLTAGE |
| t150_vrs | T150_VRS_PROCESSOR | VRS_MARGIN, VRS_STATE, VRS_LIFT_FACTOR |
| t150_propulsion | T150_PROPULSION_PROCESSOR | PROP_THRUST_TOTAL, PROP_THRUST_MARGIN, PROP_MAX_TEMP |
| t150_slung_load | T150_SLUNG_LOAD_PROCESSOR | SLUNG_CABLE_TENSION, SLUNG_SWING_MAG, SLUNG_FORCE_X/Y/Z |

### Drone Swarm Plugin (project001_drone_swarm_plugin)
| Processor | Purpose | Update Rate |
|-----------|---------|-------------|
| SwarmOrchestratorProcessor | Formation commanding, swarm coordination | 1.0 sec |
| SwarmBehaviorProcessor | Individual flocking (cohesion/separation/alignment) | 0.5 sec |
| AttackCoordinationProcessor | Multi-vector attack planning | Event-driven |
| ContainerLaunchProcessor | Launch sequencing | Event-driven |
| SwarmCommunicationProcessor | Mesh networking, leader election | 1.0 sec |

---

## Integration Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    T150_SWARM_DRONE Platform                     │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │ T150_BATTERY    │  │ T150_VRS        │  │ T150_PROPULSION │ │
│  │ PROCESSOR       │  │ PROCESSOR       │  │ PROCESSOR       │ │
│  │                 │  │                 │  │                 │ │
│  │ BATTERY_SOC ────┼──┼─────────────────┼──┼──────────────┐  │ │
│  │ BATTERY_VOLTAGE │  │ VRS_MARGIN ─────┼──┼────────────┐ │  │ │
│  └─────────────────┘  │ VRS_STATE       │  │ PROP_THRUST │ │  │ │
│                       └─────────────────┘  │ PROP_TEMP ──┼─┼──┤ │
│  ┌─────────────────┐                       └─────────────┘ │  │ │
│  │ T150_SLUNG_LOAD │                                       │  │ │
│  │ PROCESSOR       │  ┌────────────────────────────────────┘  │ │
│  │                 │  │                                       │ │
│  │ SLUNG_TENSION ──┼──┤   ┌─────────────────────────────────┐ │ │
│  │ SLUNG_SWING ────┼──┼──>│ SwarmBehaviorProcessor          │ │ │
│  └─────────────────┘  │   │ (Physics-Aware Flocking)        │ │ │
│                       │   │                                 │ │ │
│                       │   │ - Battery RTB threshold         │ │ │
│                       │   │ - VRS-safe descent              │ │ │
│                       │   │ - Thermal throttling            │ │ │
│                       │   │ - Load-adjusted separation      │ │ │
│                       │   └─────────────────────────────────┘ │ │
└─────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────┐
│                 SwarmOrchestratorProcessor                       │
│                 (Physics-Aware Coordination)                     │
├─────────────────────────────────────────────────────────────────┤
│  - Monitor fleet battery status (lowest SOC drone)              │
│  - Adjust formation based on thermal health                     │
│  - Plan VRS-safe descent profiles                               │
│  - Account for slung loads in spacing                           │
│  - Trigger RTB for critical drones                              │
└─────────────────────────────────────────────────────────────────┘
```

---

## Implementation Phases

### Phase A: Physics-Aware Swarm Platform Type (4 hours)

**Goal:** Create `T150_SWARM_DRONE` platform type with all physics processors.

**Tasks:**
1. Create `platforms/t150_swarm_drone.afsim` in drone_swarm plugin
2. Inherit from base swarm drone, add T-150 processors
3. Configure realistic physics parameters
4. Test with existing swarm scenarios

**New Platform Definition:**
```afsim
platform_type T150_SWARM_DRONE SWARM_DRONE

    # Battery subsystem (T-150 specs)
    processor BATTERY T150_BATTERY_PROCESSOR
        battery_capacity_wh    7000.0      # 7 kWh pack
        initial_soc            1.0
        hover_power_w          18000.0     # 18 kW hover
        cruise_power_w         15000.0     # 15 kW cruise
        low_battery_warning    0.25        # 25% RTB threshold
        critical_battery       0.10        # 10% emergency
        cells_in_series        13          # 13S Li-Ion
        update_interval        0.1 sec
    end_processor

    # VRS detection
    processor VRS T150_VRS_PROCESSOR
        vrs_descent_threshold  4.0         # m/s descent rate
        vrs_speed_threshold    5.0         # m/s forward speed
        caution_margin         0.85
        induced_velocity       8.0         # m/s hover induced
        coaxial_factor         0.85        # Coaxial VRS resistance
        update_interval        0.1 sec
    end_processor

    # Propulsion system
    processor PROPULSION T150_PROPULSION_PROCESSOR
        motor_count            8
        kv_rating              100
        max_power_per_motor    3500        # 3.5 kW per motor
        propeller_diameter     0.76        # 30" props
        coaxial_efficiency     0.85
        motor_thermal_limit    120         # C
        nominal_voltage        46.8
        base_mass              68          # kg empty
        payload_mass           0           # kg payload
        ground_effect_enabled  true
        update_interval        0.1 sec
    end_processor

end_platform_type
```

**Deliverables:**
- `project001_drone_swarm_plugin/scenarios/platforms/t150_swarm_drone.afsim`
- Updated `swarm_demo.afsim` with T150 option

---

### Phase B: Battery-Aware Formation Control (6 hours)

**Goal:** SwarmOrchestratorProcessor monitors battery and triggers RTB.

**Behavior Changes:**
1. Monitor `BATTERY_SOC` for all subordinate drones
2. Switch to energy-efficient formation when fleet avg < 40%
3. Trigger individual RTB when drone SOC < 25%
4. Emergency land when any drone SOC < 10%

**Implementation:**
```cpp
// In SwarmOrchestratorProcessor::Update()
void CheckFleetBattery() {
    double lowestSOC = 1.0;
    WsfPlatform* criticalDrone = nullptr;

    for (WsfPlatform* drone : mSubordinates) {
        double soc = drone->GetAuxData().Double("BATTERY_SOC", 1.0);
        if (soc < lowestSOC) {
            lowestSOC = soc;
            criticalDrone = drone;
        }

        // Trigger RTB for low battery drone
        if (soc < 0.25 && !drone->GetAuxData().Bool("RTB_COMMANDED")) {
            CommandRTB(drone);
            drone->GetAuxData().AssignBool("RTB_COMMANDED", true);
        }
    }

    // Switch to efficient formation if fleet low
    if (lowestSOC < 0.40) {
        SwitchToEfficientFormation();
    }

    // Expose fleet status
    GetPlatform()->GetAuxData().AssignDouble("FLEET_LOWEST_SOC", lowestSOC);
}
```

**New AuxData Outputs:**
- `FLEET_LOWEST_SOC` - Lowest battery in swarm
- `FLEET_AVG_SOC` - Average battery level
- `RTB_COMMANDED` - Per-drone RTB flag

**Deliverables:**
- Modified `SwarmOrchestratorProcessor.cpp`
- New `SwarmHealthMonitor` helper class
- Test scenario: `t150_swarm_endurance_test.afsim`

---

### Phase C: VRS-Safe Descent Planning (4 hours)

**Goal:** SwarmBehaviorProcessor checks VRS margin before descent.

**Behavior Changes:**
1. Read `VRS_MARGIN` before commanding descent waypoints
2. Maintain forward speed > 5 m/s during descent
3. Abort descent and level off if VRS_MARGIN < 0.3
4. Implement spiral descent pattern for VRS avoidance

**Implementation:**
```cpp
// In SwarmBehaviorProcessor - descent waypoint generation
WsfWaypoint ComputeSafeDescentWaypoint(double targetAlt) {
    double vrsMargin = GetPlatform()->GetAuxData().Double("VRS_MARGIN", 1.0);

    if (vrsMargin < 0.3) {
        // VRS danger - abort descent, increase forward speed
        ut::log::warning() << "VRS caution - aborting descent for "
                          << GetPlatform()->GetName();

        // Return level waypoint with increased speed
        return WsfWaypoint(mCurrentLat, mCurrentLon, mCurrentAlt,
                          mForwardSpeed + 5.0);  // Add 5 m/s
    }

    if (vrsMargin < 0.6) {
        // Spiral descent to maintain forward speed
        return ComputeSpiralDescentWaypoint(targetAlt);
    }

    // Normal descent
    return WsfWaypoint(mTargetLat, mTargetLon, targetAlt, mDescentSpeed);
}
```

**New Formation Pattern:**
- `SPIRAL_DESCENT` - Maintain 8 m/s forward speed while descending

**Deliverables:**
- Modified `SwarmBehaviorProcessor.cpp`
- VRS-aware descent algorithm
- Test scenario: `t150_swarm_vrs_test.afsim`

---

### Phase D: Thermal-Aware Thrust Management (4 hours)

**Goal:** Reduce formation speed when motors overheating.

**Behavior Changes:**
1. Monitor `PROP_MAX_TEMP` across fleet
2. Reduce cruise speed when any motor > 100C
3. Increase separation for better cooling
4. Abort attack run if thermal margin critical

**Implementation:**
```cpp
// In SwarmOrchestratorProcessor
void CheckFleetThermal() {
    double maxTemp = 0.0;
    int overheatingCount = 0;

    for (WsfPlatform* drone : mSubordinates) {
        double temp = drone->GetAuxData().Double("PROP_MAX_TEMP", 25.0);
        maxTemp = std::max(maxTemp, temp);

        if (temp > 100.0) {
            overheatingCount++;
        }
    }

    // Thermal throttling
    if (maxTemp > 100.0) {
        mCruiseSpeedKnots *= 0.8;  // Reduce to 80%
        ut::log::warning() << "Thermal throttle - reducing cruise to "
                          << mCruiseSpeedKnots << " kts";
    }

    // Increase separation for cooling
    if (overheatingCount > mSubordinates.size() * 0.3) {
        mFormationSpacing *= 1.2;  // 20% more separation
    }

    GetPlatform()->GetAuxData().AssignDouble("FLEET_MAX_TEMP", maxTemp);
}
```

**Deliverables:**
- Modified `SwarmOrchestratorProcessor.cpp`
- Thermal management algorithm
- Test scenario: `t150_swarm_thermal_test.afsim`

---

### Phase E: Slung Load Swarm Operations (8 hours)

**Goal:** Support cargo delivery missions with swarm coordination.

**New Capabilities:**
1. Payload-aware separation (heavier drones need more space)
2. Cable swing compensation in flocking
3. Synchronized cargo release
4. Formation adjustment for load CG shift

**Implementation:**
```cpp
// Extended T150_CARGO_DRONE platform
processor SLUNG_LOAD T150_SLUNG_LOAD_PROCESSOR
    load_mass          25.0        # 25 kg cargo
    cable_length       5.0         # 5 meter cable
    cable_damping      0.1
    max_cable_tension  2500.0
    resonance_threshold 15.0
    update_interval    0.02        # 50 Hz for dynamics
end_processor

// In SwarmBehaviorProcessor
void AdjustForSlungLoad() {
    double swingMag = GetPlatform()->GetAuxData().Double("SLUNG_SWING_MAG", 0.0);
    double loadMass = GetPlatform()->GetAuxData().Double("SLUNG_LOAD_MASS", 0.0);

    // Increase separation based on swing magnitude
    if (swingMag > 10.0) {  // > 10 degrees swing
        mSeparationMin *= 1.5;  // 50% more separation
    }

    // Reduce maneuver rate to damp oscillations
    if (swingMag > 5.0) {
        mMaxTurnRate *= 0.7;  // 30% slower turns
    }

    // Adjust formation radius for loaded drones
    if (loadMass > 0.0) {
        mFormationRadius += loadMass * 0.2;  // 0.2m per kg
    }
}
```

**New Formation Patterns:**
- `CARGO_DELIVERY` - Wide spacing, slow maneuvers
- `SYNCHRONIZED_DROP` - Coordinated release timing

**Deliverables:**
- `T150_CARGO_DRONE` platform type
- Modified `SwarmBehaviorProcessor.cpp` for slung load awareness
- Modified `AttackCoordinationProcessor.cpp` for cargo release
- Test scenario: `t150_swarm_cargo_delivery.afsim`

---

## File Structure

```
project001_drone_swarm_plugin/
├── scenarios/
│   ├── platforms/
│   │   ├── swarm_drone.afsim           # Original
│   │   ├── t150_swarm_drone.afsim      # NEW: Physics-aware drone
│   │   └── t150_cargo_drone.afsim      # NEW: Cargo variant
│   ├── swarm_demo.afsim                # Original demo
│   ├── t150_swarm_demo.afsim           # NEW: T-150 swarm demo
│   ├── t150_swarm_endurance.afsim      # NEW: Battery test
│   ├── t150_swarm_vrs_test.afsim       # NEW: VRS test
│   ├── t150_swarm_thermal_test.afsim   # NEW: Thermal test
│   └── t150_cargo_delivery.afsim       # NEW: Cargo mission
├── include/
│   ├── SwarmHealthMonitor.hpp          # NEW: Fleet health tracking
│   └── PhysicsAwareFlocking.hpp        # NEW: Physics-integrated behavior
├── source/
│   ├── SwarmOrchestratorProcessor.cpp  # MODIFIED: Add physics checks
│   ├── SwarmBehaviorProcessor.cpp      # MODIFIED: Add physics awareness
│   └── SwarmHealthMonitor.cpp          # NEW: Implementation
└── PHYSICS_INTEGRATION.md              # NEW: Integration documentation
```

---

## Dependencies

### Required Plugins
The T-150 plugins must be installed in AFSIM's mission_plugins directory:
- `libt150_battery.so`
- `libt150_vrs.so`
- `libt150_propulsion.so`
- `libt150_slung_load.so`

### Plugin Load Order
AFSIM loads plugins alphabetically. Ensure T-150 plugins load before accessing their processor types.

---

## Validation Plan

### Phase A Validation
1. Launch 20 T150_SWARM_DRONE platforms
2. Verify all processors initialize
3. Check AuxData populated correctly
4. Confirm formation behavior unchanged

### Phase B Validation
1. Run 30-minute endurance mission
2. Verify RTB triggered at 25% SOC
3. Confirm efficient formation switch at 40%
4. Check no crashes from battery depletion

### Phase C Validation
1. Command descent from 500m to 50m
2. Create VRS conditions (fast descent, low speed)
3. Verify abort and recovery
4. Confirm no VRS-induced failures

### Phase D Validation
1. Run aggressive maneuvers
2. Monitor motor temperatures
3. Verify thermal throttling activates
4. Confirm cooling recovery

### Phase E Validation
1. Launch 10 cargo drones with 25kg loads
2. Form up with increased separation
3. Transit to drop zone
4. Execute synchronized release
5. RTB without cargo

---

## Estimated Effort

| Phase | Description | Hours | Priority |
|-------|-------------|-------|----------|
| A | Physics-Aware Platform Type | 4 | Critical |
| B | Battery-Aware Formation | 6 | Critical |
| C | VRS-Safe Descent | 4 | High |
| D | Thermal Management | 4 | Medium |
| E | Slung Load Operations | 8 | Future |
| **Total** | | **26 hours** | |

---

## Success Criteria

1. **No crashes from physics** - Drones RTB before battery/thermal failure
2. **Realistic endurance** - 18-20 min flight time matches T-150 specs
3. **Safe descent** - Zero VRS events with physics-aware planning
4. **Thermal margin** - Motors stay below 100C during normal ops
5. **Cargo ops** - Successful delivery with stable slung loads

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-11-30 | Initial integration plan |
