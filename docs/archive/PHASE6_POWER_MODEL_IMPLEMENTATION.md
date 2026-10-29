# Phase 6: Realistic Power Model Implementation

## Overview

This document details the implementation of Phase 6 enhancements to the T-150 UAS battery processor plugin. The primary goal was to fix unrealistic range calculations (was ~160 km, should be 35-45 km per T-150 specs) by implementing a comprehensive, physics-based power model.

## Problem Statement

### Initial Issue
The battery simulation was producing unrealistic range estimates of ~160 km when the T-150 specifications indicate a maximum range of 35-45 km. Investigation revealed multiple contributing factors:

1. **Power source reporting near-zero values**: The 6DOF mover was not consistently providing power consumption data via AuxData
2. **No minimum power floor**: When reported power was low/zero, the battery barely drained
3. **Missing auxiliary systems**: No accounting for avionics, radio, camera, ECU power draw
4. **Voltage sag override bug**: The voltage sag model was overwriting the power value, bypassing any floors

### Root Cause Analysis
The battery processor was reading power from `PROP_TOTAL_POWER` AuxData key, but:
- The 6DOF mover uses `6dof_total_power` as its key
- Even when properly connected, reported power could be unrealistically low during steady cruise
- The voltage sag calculation at lines 501 and 511 was overwriting `mCurrentPower` with voltage-limited values

## Solution Architecture

### Phase 6 Power Model Components

```
Total Power = max(Reported Power, Estimated Power, Min Floor) + Auxiliary Power
```

Where:
- **Reported Power**: Value from mover via AuxData (with fallback chain)
- **Estimated Power**: Velocity-based calculation using platform kinematic state
- **Min Floor**: `mHoverPowerW * mMinFlightPowerFactor` (default: 50% of 18kW = 9kW)
- **Auxiliary Power**: Constant draw for avionics, radio, camera, ECU (default: 500W)

### Velocity-Based Power Estimation

```cpp
// Horizontal velocity magnitude
double velHoriz = sqrt(vN^2 + vE^2);
double velVert = -vD;  // NED convention

// Speed factor: 0 at hover, 1 at cruise speed
double speedFactor = min(1.0, velHoriz / mCruiseSpeedMps);

// Power interpolation
if (velHoriz < 1.0) {
    estimatedPower = mHoverPowerW;  // Hover
} else {
    estimatedPower = mHoverPowerW + (mCruisePowerW - mHoverPowerW) * speedFactor;
}

// Add climb power: P = m*g*v_climb / efficiency
if (velVert > 0.5) {
    climbPower = 120.0 * 9.81 * velVert / 0.7;
    estimatedPower += climbPower;
}
```

### Critical Bug Fix: Voltage Sag Override

The voltage sag model was recalculating current and power based on what the battery could deliver:

```cpp
// BEFORE (buggy):
mCurrentAmps = current;
mCurrentPower = mTerminalVoltage * mCurrentAmps;  // Overwrites requested power!

// AFTER (fixed):
double requestedPower = mCurrentPower;  // Store before voltage sag
// ... voltage sag calculations ...
double energyDelta = requestedPower * (deltaTime / 3600.0);  // Use requested
mCurrentPower = requestedPower;  // Restore for CSV reporting
```

This ensures the battery depletes based on what the aircraft *needs* to fly, not what the battery can physically deliver. In reality, if the battery can't deliver enough power, the aircraft crashes - we simulate this by depleting based on demand.

## Implementation Details

### New Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `power_source_key` | "PROP_TOTAL_POWER" | AuxData key to read power from |
| `auxiliary_power_w` | 500.0 | Constant auxiliary systems power (W) |
| `min_flight_power_factor` | 0.50 | Min power as fraction of hover (0.1-1.0) |
| `cruise_power_w` | 17000.0 | Power at cruise speed (W) |
| `cruise_speed_mps` | 30.0 | Reference cruise speed (m/s) |
| `use_velocity_power_model` | true | Enable velocity-based estimation |
| `csv_output_file` | "" | Path for CSV data logging |
| `csv_log_interval` | 1.0 | CSV logging interval (seconds) |

### Power Source Fallback Chain

The battery processor attempts to read power in this order:
1. Configured `power_source_key` (e.g., "PROP_TOTAL_POWER")
2. "6dof_total_power" (for 6DOF mover compatibility)
3. "PROP_TOTAL_POWER" (original propulsion processor)
4. Falls back to `mHoverPowerW` if all fail

### CSV Output Format

```csv
Time_s,SOC_pct,Voltage_V,Current_A,Power_W,Energy_Consumed_Wh,OCV_V,Voltage_Sag_V,Remaining_Energy_Wh
1.100,99.752,39.000,479.714,20352.784,5.653,54.550,15.550,6994.347
...
```

Summary appended at end of file:
```
# BATTERY SUMMARY
# Flight Duration: 1200.7 seconds
# Initial SOC: 100.000 %
# Final SOC: 0.000 %
# Total Energy Consumed: 6988.850 Wh
# Average Power: 20.925 kW
```

## Files Modified

### BatteryProcessor.hpp
- Added `<string>` and `<fstream>` includes
- Added Phase 6 member variables for power model parameters
- Added CSV output members (`mCsvFile`, `mCsvOutputPath`, etc.)
- Added flight statistics members (`mFlightStartTime`, `mSumPower`, etc.)
- Added destructor declaration for CSV cleanup
- Added `WriteCSVRow()` and `WriteSummary()` method declarations

### BatteryProcessor.cpp
- Constructor: Initialize all new member variables with T-150 defaults
- Copy constructor: Copy new members (except file handle)
- Destructor: Implemented to call `WriteSummary()` and close CSV
- `ProcessInput()`: Added handlers for all new configuration parameters
- `Initialize()`: Open CSV file, write header, log enhanced configuration
- `Update()`: Implemented complete Phase 6 power model:
  - Power source fallback chain
  - Velocity-based estimation with climb power
  - Maximum of reported/estimated/floor
  - Auxiliary power addition
  - Critical fix for voltage sag override
  - CSV row writing at interval
- `WriteCSVRow()`: Outputs time-series data
- `WriteSummary()`: Outputs final statistics to CSV and log

### New Files Created
- `platforms/t150_6dof_battery_platform.afsim` - Combined 6DOF + battery platform
- `scenarios/battery_range_test.afsim` - Maximum range test scenario
- `scenarios/battery_test_straight.afsim` - Straight line baseline
- `scenarios/battery_test_sawtooth.afsim` - Zigzag pattern
- `scenarios/battery_test_square.afsim` - Square wave pattern
- `scenarios/battery_test_sine.afsim` - Sinusoidal pattern
- `scenarios/battery_test_cycloid.afsim` - Cycloidal loops pattern

## Plugin Installation Notes

AFSIM loads plugins from TWO directories:
1. `/opt/afsim2.2505/lib/wsf_plugins/`
2. `/opt/afsim2.2505/bin/mission_plugins/`

Use the NGIC scripts to build and install to both locations:

```bash
cd /home/shepard/afsim_projects/project005_t150_uas

# Build all plugins
./ngic_afsim_build.sh

# Install to AFSIM (both directories)
./ngic_afsim_install.sh

# Verify installation
./ngic_afsim_install.sh --verify
```

## Verification Results

### Battery Range Test
```
Flight time to depletion: 1200.7 seconds (~20 minutes)
Energy at depletion: 6988.85 Wh (of 7000 Wh capacity)
Average power: 20.9 kW
Range at 30 m/s: 36.0 km
```

### Comparison to T-150 Specifications
| Metric | Simulated | Spec | Status |
|--------|-----------|------|--------|
| Range | 36 km | 35-45 km | PASS |
| Cruise Speed | 30 m/s | 30 m/s | PASS |
| Battery Capacity | 7000 Wh | 7000 Wh | PASS |
| Hover Power | 18 kW | ~18 kW | PASS |
| Cruise Power | 17 kW | ~17 kW | PASS |

## Power Model Validation

### Power Breakdown During Cruise
- Base cruise power: ~17 kW (T-150 spec at 30 m/s)
- Auxiliary systems: 0.5 kW
- Effective power: ~17.5-21 kW (varies with flight conditions)

### Why Minimum Floor is Needed
Even when the mover reports low power values, a flying aircraft requires significant power to:
1. Overcome rotor drag
2. Maintain altitude against gravity
3. Power control surfaces/motors at operating RPM
4. Run all flight-critical systems

The 50% hover floor (9 kW) ensures the simulation never unrealistically assumes the aircraft can fly on minimal power.

## Future Improvements

1. **Temperature modeling**: Internal resistance varies with battery temperature
2. **Regenerative descent**: Could reduce power consumption during descent
3. **Payload-aware power**: Adjust power based on current payload mass
4. **Wind effects**: Headwind/tailwind impact on power consumption
5. **Battery degradation**: SOH (State of Health) modeling over charge cycles

## Commit Reference

```
commit 17b463a
Phase 6: Realistic power model with velocity-based estimation

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>
```
