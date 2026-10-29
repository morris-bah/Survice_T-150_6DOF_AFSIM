# 6DOF Battery Integration Troubleshooting Reference

## Date: January 12, 2026

## Overview

This document captures the debugging process for integrating the 6DOF rotorcraft physics mover with the battery processor plugin. The goal was to ensure the battery drain accurately reflects the physics-based power consumption calculated by the 6DOF mover.

---

## Problem Statement

The battery processor was showing only a **16% power increase** when mass increased by **124%** (55 kg to 123 kg). According to rotorcraft physics, power scales as W^1.5, so a 2.24x mass increase should result in a **3.35x power increase**.

### Expected vs Observed (Before Fix)

| Configuration | Expected Power | Observed Power |
|--------------|----------------|----------------|
| 55 kg (no payload) | ~6 kW | ~17.5 kW |
| 123 kg (with payload) | ~20 kW | ~20.4 kW |
| **Ratio** | **3.35x** | **1.16x** |

---

## Root Causes Identified

### Issue 1: Old Plugin Files Persisting in Multiple Locations

**Symptom:** Debug output added to the battery processor never appeared in simulation output.

**Root Cause:** Multiple versions of battery plugin files existed:
- `/home/shepard/afsim_projects/project005_t150_uas/lib/t150_battery.so` (old, Jan 9)
- `/home/shepard/afsim_projects/project005_t150_uas/lib/libt150_battery.so` (old)
- `/opt/afsim2.2505/bin/mission_plugins/libt150_battery.so` (old)
- `/opt/afsim2.2505/lib/wsf_plugins/t150_battery.so` (old)

The `ngic_afsim_install.sh` script's `discover_local_plugins()` function was finding and installing ALL `.so` files from the `lib/` directory.

**Fix:**
1. Removed old plugin files from `lib/` directory (files without `lib` prefix from Jan 9)
2. Removed old plugin from `/opt/afsim2.2505/bin/mission_plugins/libt150_battery.so`
3. Ensured only current plugins with `lib` prefix remain

**Files cleaned:**
```bash
rm lib/t150_battery.so
rm lib/t150_propulsion.so
rm lib/t150_vrs.so
sudo rm /opt/afsim2.2505/bin/mission_plugins/libt150_battery.so
```

---

### Issue 2: power_source_key Parsing Including Quotes

**Symptom:** Battery processor TryGet for AuxData key failing with `FAILED | Value=0W`.

**Debug Output:**
```
[BATTERY POWER READ] PowerSourceKey='"6dof_total_power"'
[BATTERY POWER READ] TryGet Result: FAILED | Value=0W | >0? NO
```

**Root Cause:** The `ReadValue()` function was including the surrounding double quotes from the scenario file as part of the string value.

- Scenario file: `power_source_key "6dof_total_power"`
- Parsed value: `"6dof_total_power"` (with quotes)
- 6DOF publishes to: `6dof_total_power` (without quotes)
- Key mismatch caused TryGet to fail

**Fix:** Added quote stripping in `BatteryProcessor.cpp`:

```cpp
else if (command == "power_source_key")
{
    aInput.ReadValue(mPowerSourceKey);
    // Strip surrounding quotes if present (ReadValue includes them)
    if (mPowerSourceKey.size() >= 2 &&
        mPowerSourceKey.front() == '"' && mPowerSourceKey.back() == '"')
    {
        mPowerSourceKey = mPowerSourceKey.substr(1, mPowerSourceKey.size() - 2);
    }
}
```

**Location:** `plugins/t150_battery/source/BatteryProcessor.cpp` lines 199-208

---

### Issue 3: Plugin Naming Convention

**Symptom:** Battery plugin not loading when named `t150_battery.so`.

**Root Cause:** AFSIM plugin loader expects plugin filename to match the processor type name pattern.

**Fix:** Renamed plugin from `t150_battery` to `t150_battery_processor` in CMakeLists.txt:
```cmake
set(PLUGIN_NAME t150_battery_processor)
```

---

## Verification Results (After Fixes)

### Final Test Results

| Configuration | Cruise Power | Flight Time | Range |
|--------------|--------------|-------------|-------|
| 55 kg (no payload) | 5.94 kW | 64.4 min | 116 km |
| 123 kg (68 kg payload) | 19.88 kW | 20.4 min | 37 km |
| **Ratio** | **3.35x** | **3.15x** | **3.15x** |

### Physics Validation

- Mass ratio: 123/55 = 2.24x
- Theoretical power ratio (W^1.5): 2.24^1.5 = **3.35x**
- Actual cruise power ratio: 19.88/5.94 = **3.35x** (MATCHES!)

---

## Key Files Modified

### Plugin Source Code
- `plugins/t150_battery/source/BatteryProcessor.cpp` - Fixed power_source_key parsing, added debug output
- `plugins/t150_battery/source/BatteryPluginRegistration.cpp` - Debug logging
- `plugins/t150_battery/source/BatteryRegistration.hpp` - Debug logging
- `plugins/t150_battery/CMakeLists.txt` - Renamed plugin to t150_battery_processor
- `plugins/rotorcraft_6dof/source/Rotorcraft6DOF_Mover.cpp` - Added diagnostic telemetry
- `plugins/build.sh` - Updated plugin list

### Scenario Files
- `scenarios/battery_range_test.afsim` - Added `use_velocity_power_model false`
- `scenarios/battery_range_test_no_payload.afsim` - New scenario for baseline testing

---

## Debug Output Added

### 6DOF Mover Diagnostics (every 30 seconds)
```
[6DOF DIAG] T=300s | Mass=55kg | Alt=350m | Vel=28.8m/s | Tilt=-15.3deg | Throttle=0.397 | RPM=2173 | Thrust=0.429kN | Power=5.94kW
```

### Battery Processor Diagnostics (every 5 minutes)
```
[BATTERY UPDATE] Called! Counter=3000 Time=299.9s
[BATTERY POWER READ] DynamicMode=TRUE | PowerSourceKey='6dof_total_power'
[BATTERY POWER READ] TryGet Result: SUCCESS | Value=5936.11W | >0? YES
```

---

## AuxData Communication Flow

1. **6DOF Mover** calculates total power based on:
   - Rotor thrust requirements (mass-dependent)
   - RPM and torque coefficients
   - Aerodynamic drag

2. **6DOF Mover** publishes via AuxData:
   ```cpp
   auxData.Assign("6dof_total_power", totalPower);
   ```

3. **Battery Processor** reads power:
   ```cpp
   GetPlatform()->GetAuxData().TryGet("6dof_total_power", sourcePower);
   ```

4. **Battery Processor** drains energy based on read power + auxiliary systems

---

## Lessons Learned

1. **Always verify the correct plugin is loading** - Check `Plugins Loaded:` output in AFSIM startup
2. **AFSIM's ReadValue includes quotes** - Must strip them for string parameters
3. **Multiple plugin directories exist** - Check both `lib/wsf_plugins/` and `bin/mission_plugins/`
4. **Add debug output to verify data flow** - Counter-based periodic logging is useful
5. **Plugin filename must match type pattern** - `foo_processor.so` for `FOO_PROCESSOR` type

---

## Commands for Future Debugging

```bash
# Check all battery plugin locations
find /home/shepard/afsim_projects -name "*battery*.so" 2>/dev/null
find /opt/afsim2.2505 -name "*battery*.so" 2>/dev/null

# Verify correct plugin loads
/opt/afsim2.2505/bin/mission scenario.afsim 2>&1 | grep "Plugins Loaded"

# Check debug output
/opt/afsim2.2505/bin/mission scenario.afsim 2>&1 | grep -E "(BATTERY|6DOF DIAG)"

# Rebuild and install
cd plugins && ./build.sh && sudo cmake --install build

# Or use install script (after cleaning old plugins)
sudo ./ngic_afsim_install.sh
```

---

## Contact

For questions about this troubleshooting, contact the development team.
