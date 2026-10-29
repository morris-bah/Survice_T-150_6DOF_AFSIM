# Rotorcraft 6DOF Mover - Comprehensive Reference

## Overview

The `rotorcraft_6dof` plugin provides a physics-based 6DOF (Six Degrees of Freedom) mover for multirotor aircraft in AFSIM. It implements Newton-Euler equations of motion with momentum theory rotor aerodynamics.

## Running AFSIM Simulations

### Command Line Simulation (mission)
```bash
# Run simulation - generates .aer output file
/opt/afsim2.2505/bin/mission scenarios/your_scenario.afsim

# Run from project directory
cd /home/shepard/afsim_projects/project005_t150_uas
/opt/afsim2.2505/bin/mission scenarios/t150_6dof_hover_test.afsim
```

### Visualization with Mystic
```bash
# Mystic visualizes .aer OUTPUT files (not .afsim scenario files)
# MUST set DISPLAY=:0 for GUI applications in WSL

# First run simulation to generate .aer file
/opt/afsim2.2505/bin/mission scenarios/t150_6dof_hover_test.afsim

# Then visualize the output
DISPLAY=:0 /opt/afsim2.2505/bin/mystic output/t150_6dof_hover_test.aer
```

### Important Notes
- `mission` = CLI, runs simulation, creates .aer output
- `mystic` = GUI, visualizes .aer files, requires `DISPLAY=:0`
- `engage` = Different application, not for standard scenarios
- All GUI AFSIM programs need `DISPLAY=:0` prefix in WSL

## Project Structure

```
/home/shepard/afsim_projects/project005_t150_uas/
├── plugins/
│   └── rotorcraft_6dof/
│       └── source/
│           ├── Rotorcraft6DOF_Mover.hpp   # Main mover class
│           ├── Rotorcraft6DOF_Mover.cpp   # Implementation
│           ├── RotorModel.hpp             # Single rotor physics
│           ├── RotorModel.cpp
│           ├── RigidBody6DOF.hpp          # 6DOF dynamics
│           ├── RigidBody6DOF.cpp
│           └── MathUtils.hpp              # Vector3, Quaternion, constants
├── platforms/
│   └── t150_6dof_platform.afsim           # Platform type definition
├── scenarios/
│   ├── t150_6dof_hover_test.afsim         # Vertical climb + hover test
│   ├── t150_6dof_simple_test.afsim        # Simple climb test
│   └── t150_6dof_flight_demo.afsim        # Complex flight patterns (zigzag, square, sine, loops)
├── output/                                 # Simulation output (.aer files)
└── docs/
```

## Building the Plugin

```bash
cd /home/shepard/afsim_projects/project005_t150_uas

# Build all plugins (auto-detects AFSIM)
./ngic_afsim_build.sh

# Install to AFSIM (requires sudo)
./ngic_afsim_install.sh
```

## Architecture

### Class Hierarchy
```
WsfRouteMover (AFSIM base class for route-following movers)
    └── Rotorcraft6DOF_Mover
            ├── RigidBody6DOF (Newton-Euler dynamics)
            └── RotorModel[] (momentum theory rotors)
```

### Key Design Decision: WsfRouteMover
The mover inherits from `WsfRouteMover` (not `WsfMover`) for proper AFSIM route integration. This requires implementing these pure virtual methods:

```cpp
void SetGuidanceFrom(const WsfWaypoint& aPreviousWaypoint) override;
void SetGuidanceTo(const WsfWaypoint& aNextWaypoint) override;
void AdvanceToNextWaypoint(double aSimTime, const WsfWaypoint& aTarget,
                           const WsfWaypoint* aNextPointPtr) override;
void UpdatePosition(double aSimTime) override;
void Pause(double aSimTime, double aUnpauseTime) override;
```

### Physics Update Flow
1. `WsfRouteMover::Update()` called by AFSIM
2. Calls our `UpdatePosition(aSimTime)`
3. Check waypoint arrival → `WaypointReached()`
4. Update altitude hold controller (PID)
5. Update position hold controller (cascaded P → velocity → attitude)
6. Apply control mixer → rotor RPM commands
7. Update rotor states (motor dynamics)
8. Compute forces/moments (thrust, drag, rotor torque)
9. Integrate 6DOF equations (`RigidBody6DOF::Integrate()`)
10. Sync state to AFSIM platform (`SyncToAFSIM()`)

## Controllers

### Altitude Hold (PID)
```cpp
// Gains (tuned for stable hover with no steady-state error)
Kp = 0.3    // Proportional
Ki = 0.05   // Integral (eliminates steady-state error)
Kd = 1.2    // Derivative (damping)

// Anti-windup: only integrate when |error| < 10m
// Integral limit: ±3.0
// Achieves ±3cm precision at hover
```

### Position Hold (Cascaded PD-P)
```
Position Error → Desired Velocity → Velocity Error → Tilt Command → Attitude
```

**Tuned Gains (T-150):**
```cpp
position_kp = 1.0      // Position error (m) → velocity (m/s)
position_kd = 0.1      // Velocity damping [s]
velocity_kp = 3.0      // Velocity error → tilt command (CRITICAL: was 0.03)
max_vel_accel = 8.0    // Maximum velocity acceleration [m/s²]
max_tilt_angle = 25.0  // Maximum tilt for translation [deg]
waypoint_arrival_radius = 3.0  // [m] Waypoint considered reached
```

**Note:** The original velocity_kp of 0.03 was far too conservative and prevented horizontal movement. 3.0 provides responsive tracking.

### Attitude Stabilization (Inner Loop)
```cpp
// PD controller on attitude error
attitudeKp = 8.0   // Proportional
attitudeKd = 2.0   // Derivative (rate damping)
```

### Heading Control (Optional)
```cpp
// Faces direction of travel when enabled
heading_hold true/false   // Currently disabled for stability
heading_kp = 2.0          // Heading error → yaw rate command
```
**Note:** Heading control is implemented but currently disabled (`heading_hold false`) to ensure stable waypoint tracking. When enabled, drone faces direction of travel but can interact with position control in some scenarios.

## Rotor Configurations

Supported configurations via `rotor_configuration` parameter:
- `QUAD_X` - Quadcopter X configuration (default)
- `QUAD_PLUS` - Quadcopter + configuration
- `HEX_X` - Hexacopter
- `OCTO_X` - Octocopter
- `COAXIAL_OCTO` - T-150 style (4 arms, 8 motors)

## Scenario File Usage

```
platform_type T150_6DOF_UAS WSF_PLATFORM
   mover Rotorcraft6DOF_Mover
      # Mass properties
      mass 12.3 kg
      inertia_xx 0.5
      inertia_yy 0.5
      inertia_zz 0.9

      # Rotor configuration
      rotor_configuration COAXIAL_OCTO
      rotor_diameter 0.4 m
      arm_length 0.35 m
      thrust_coefficient 0.12
      max_rpm 8000

      # Controller tuning (optional - defaults are good)
      altitude_kp 0.3
      altitude_ki 0.05
      altitude_kd 1.2
   end_mover
end_platform_type

platform my_drone T150_6DOF_UAS
   route
      # IMPORTANT: Waypoints must not be coincident (same lat/lon)
      # Add tiny offset if doing pure vertical climb
      position 35.0000n 120.0000w altitude 0 m agl speed 0 m/s
      position 35.00001n 120.0000w altitude 50 m agl speed 2 m/s stop
   end_route
end_platform
```

## Coordinate Systems

- **NED (North-East-Down)**: Primary frame for position/velocity
- **Body Frame**: X-forward, Y-right, Z-down
- **Geodetic**: Lat/lon/alt for AFSIM interface

### Geodetic Conversion (Flat Earth Approximation)
```cpp
// Local functions in Rotorcraft6DOF_Mover.cpp
void GeodeticOffsetToNED(fromLat, fromLon, toLat, toLon, outN, outE);
void NEDOffsetToGeodetic(originLat, originLon, north, east, outLat, outLon);
```

## Key Bug Fixes and Improvements

### Session 1 Fixes
1. **Route starting at destination** - Fixed by inheriting from WsfRouteMover
2. **5m altitude steady-state error** - Fixed by adding integral term to PID
3. **Physics frozen when paused** - Fixed by running physics while paused
4. **Target overwritten during init** - Fixed initialization order

### Session 2 Fixes (January 2026)
5. **Degrees vs Radians in Geodetic Conversion** - CRITICAL FIX
   - AFSIM provides lat/lon in DEGREES, but `GeodeticOffsetToNED()` assumed RADIANS
   - Caused position errors of 25+ km (showed ~57x larger than actual)
   - Fixed by adding DEG_TO_RAD conversion in `GeodeticOffsetToNED()` and `NEDOffsetToGeodetic()`

6. **NED-to-Body Frame Transformation**
   - Pitch/roll commands were computed in NED frame but applied in body frame
   - When heading wasn't aligned with North, drone would fly in wrong direction
   - Fixed by adding yaw-based rotation: pitch/roll commands now transform correctly
   ```cpp
   // Transform NED tilt commands to body frame
   double tiltCmdForward = tiltCmdN * cos(yaw) + tiltCmdE * sin(yaw);
   double tiltCmdRight = -tiltCmdN * sin(yaw) + tiltCmdE * cos(yaw);
   ```

7. **Position Controller Tuning** - Drone wouldn't move horizontally
   - Original velocity_kp (0.03) was too conservative
   - Increased to 3.0 for responsive waypoint tracking
   - Tuned waypoint_arrival_radius from 20m → 3m for tight waypoint accuracy

## Telemetry (AuxData)

Published to platform AuxData every frame:
- `6dof_pos_n/e/d` - NED position
- `6dof_vel_n/e/d` - NED velocity
- `6dof_roll/pitch/yaw_deg` - Attitude (degrees)
- `6dof_p/q/r_dps` - Angular rates (deg/s)
- `6dof_total_thrust` - Total thrust (N)
- `6dof_total_power` - Total power (W)
- `6dof_throttle_cmd` - Throttle command (0-1)

## Git Branch

Current development work is on branch `plugin_dev`:
```bash
git checkout plugin_dev
git log --oneline -5
```

## Common Issues

1. **"Coincident waypoints" error** - Add tiny lat/lon offset between waypoints
2. **Drone falls on startup** - Rotors pre-spin to hover RPM in Initialize()
3. **No visualization** - Use `DISPLAY=:0` prefix for Mystic
4. **Plugin not loading** - Run `sudo cmake --install build` after building
