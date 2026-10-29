# T-150 UAS Custom AFSIM Plugins

This directory contains custom C++ plugins for the T-150 Heavy-Lift UAS simulation. These plugins provide advanced physics modeling that goes beyond what is possible with AFSIM's built-in script-based processors.

## Plugins Overview

### 1. t150_battery - Battery State-of-Charge Modeling

**Processor Type:** `T150_BATTERY_PROCESSOR`

Models detailed Li-Ion battery physics including:
- Real-time state-of-charge (SOC) tracking
- Peukert effect (capacity reduction at high discharge rates)
- Temperature effects on capacity and efficiency
- Voltage curve modeling
- Regenerative energy recovery during descent
- Low battery and critical battery warnings

**Configuration Example:**
```
processor BATTERY_MONITOR T150_BATTERY_PROCESSOR
    battery_capacity_wh  7000      // Total capacity in Wh
    initial_soc          1.0       // Start fully charged
    hover_power_w        18000     // Hover power consumption
    cruise_power_w       17000     // Cruise power consumption
    climb_power_w        22000     // Climb power consumption
    descent_power_w      12000     // Descent power consumption
    low_battery_warning  0.20      // 20% SOC warning
    critical_battery     0.10      // 10% SOC critical
    cells_in_series      13        // 13S configuration
    ambient_temperature  20        // Celsius
    peukert_exponent     1.05      // Capacity effect
    enable_regeneration  true      // Enable descent regen
    update_interval      0.1 sec
end_processor
```

**Script Variables (via AuxData):**
- `BATTERY_SOC` - Current state of charge (0.0-1.0)
- `BATTERY_POWER_W` - Current power draw (W)
- `BATTERY_ENERGY_WH` - Remaining energy (Wh)
- `BATTERY_TEMP_C` - Battery temperature (C)
- `BATTERY_VOLTAGE` - Pack voltage (V)
- `BATTERY_LOW` - Low battery flag
- `BATTERY_CRITICAL` - Critical battery flag

---

### 2. t150_vrs - Vortex Ring State Detection

**Processor Type:** `T150_VRS_PROCESSOR`

Detects and models Vortex Ring State (VRS) conditions:
- Real-time VRS margin calculation
- State machine: CLEAR → CAUTION → ONSET → DEVELOPED → RECOVERY
- Lift and control authority degradation modeling
- Recovery action recommendations
- Coaxial rotor VRS resistance factor

**Configuration Example:**
```
processor VRS_MONITOR T150_VRS_PROCESSOR
    vrs_descent_threshold   4.0      // m/s descent for VRS onset
    vrs_speed_threshold     5.0      // m/s forward speed prevents VRS
    caution_margin          0.8      // Fraction for caution warning
    induced_velocity        8.0      // Hover induced velocity
    coaxial_factor          0.85     // VRS resistance for coaxial
    vrs_development_time    2.0      // Seconds to develop full VRS
    vrs_recovery_time       3.0      // Seconds to recover
    max_lift_degradation    0.6      // Up to 60% lift loss
    max_control_degradation 0.5      // Up to 50% control loss
    update_interval         0.1 sec
end_processor
```

**Script Variables (via AuxData):**
- `VRS_MARGIN` - Distance from VRS (0.0-1.0)
- `VRS_STATE` - Current VRS state (0=CLEAR, 4=DEVELOPED)
- `VRS_LIFT_FACTOR` - Lift capability (0.0-1.0)
- `VRS_CONTROL_FACTOR` - Control authority (0.0-1.0)
- `VRS_DEVELOPMENT` - VRS development level
- `VRS_CAUTION` / `VRS_ONSET` / `VRS_CRITICAL` - Warning flags

---

### 3. t150_propulsion - Electric Propulsion System

**Processor Type:** `T150_PROPULSION_PROCESSOR`

Models the complete electric propulsion system:
- 8 T-Motor U15II motors (configurable)
- Per-motor RPM, current, temperature, and efficiency
- Propeller thrust and power coefficients
- Coaxial interference effects
- Motor thermal management
- Motor failure simulation

**Configuration Example:**
```
processor PROPULSION_MONITOR T150_PROPULSION_PROCESSOR
    motor_count           8          // 8 motors for octocopter
    kv_rating             100        // Motor KV rating
    max_power_per_motor   3500       // Watts per motor
    propeller_diameter    0.76       // meters (30")
    coaxial_efficiency    0.85       // Interference factor
    motor_thermal_limit   120        // Max motor temp (C)
    esc_thermal_limit     90         // Max ESC temp (C)
    nominal_voltage       46.8       // System voltage
    peak_motor_efficiency 0.92       // Peak efficiency
    update_interval       0.1 sec
end_processor
```

**Script Variables (via AuxData):**
- `PROP_TOTAL_THRUST` - Total system thrust (N)
- `PROP_TOTAL_POWER` - Total power consumption (W)
- `PROP_SYSTEM_EFF` - System efficiency (0.0-1.0)
- `PROP_MOTOR_COUNT` - Operational motor count
- `PROP_MAX_TEMP` / `PROP_AVG_TEMP` - Motor temperatures
- `MOTOR_OVERTEMP` / `MOTOR_FAILURE` - Warning flags

---

## Building the Plugins

### Prerequisites
- AFSIM 2.2505 with matching development headers and libraries
- C++20 compiler
- Linux: CMake 3.16+ and GCC/G++
- Windows: CMake 4.2+, Conan 2.32+, Visual Studio 2026 Build Tools (v145, x64)

### Windows: Conan and CMake presets

Use the [Windows setup instructions](../README.md#windows-conan-2--cmake--visual-studio-2026)
from the project root to package the local AFSIM SDK and run `conan install`.
Then:

```powershell
cmake --preset windows-release
cmake --build --preset windows-release --parallel
# Separate installation step:
cmake --build --preset windows-install --parallel
ctest --preset windows-installed
```

The Release build produces all six DLLs without installing them. Installation
deploys into `C:\code\AFSIM\AFSIM\mission_plugins` and
`C:\code\AFSIM\AFSIM\wsf_plugins`, alongside the copied AFSIM runtime.
The existing `build.bat` is a legacy direct-CMake workflow; use the root presets
for the Conan/VS 2026 workflow.

### Linux/WSL: Build and Install

Use the NGIC generic scripts from the project root:

```bash
cd /home/shepard/afsim_projects/project005_t150_uas

# Build all plugins (auto-detects AFSIM installation)
./ngic_afsim_build.sh

# Install to AFSIM (requires sudo, installs to both wsf_plugins and mission_plugins)
./ngic_afsim_install.sh
```

### Alternative: Plugin Directory Build

```bash
cd plugins
./build.sh release    # or 'debug' for debug build
sudo cmake --install build
```

### Verify Installation

```bash
./ngic_afsim_install.sh --verify
```

---

## Using the Plugins in Scenarios

### Loading Plugins
Add to your scenario file:
```
# Load T-150 plugins
load_plugin t150_battery
load_plugin t150_vrs
load_plugin t150_propulsion
```

### Adding Processors to Platform
```
platform_type MY_UAS WSF_PLATFORM
    # ... other configuration ...

    processor BATTERY T150_BATTERY_PROCESSOR
        battery_capacity_wh 7000
        # ... configuration ...
    end_processor

    processor VRS T150_VRS_PROCESSOR
        vrs_descent_threshold 4.0
        # ... configuration ...
    end_processor

    processor PROPULSION T150_PROPULSION_PROCESSOR
        motor_count 8
        # ... configuration ...
    end_processor
end_platform_type
```

### Accessing Data in Scripts
```
execute at_time 100 sec
    double soc = t150_drone.aux.BATTERY_SOC;
    double vrs_margin = t150_drone.aux.VRS_MARGIN;
    double power = t150_drone.aux.PROP_TOTAL_POWER;

    writeln("Battery: " + (soc * 100) + "% | VRS Margin: " + vrs_margin);
end_execute
```

---

## Plugin Architecture

Each plugin follows the standard AFSIM plugin architecture:

```
plugins/
├── t150_battery/
│   ├── CMakeLists.txt
│   ├── wsf_module
│   └── source/
│       ├── BatteryProcessor.hpp/cpp    # Main processor class
│       ├── BatteryRegistration.hpp     # Scenario registration
│       └── BatteryPluginRegistration.cpp  # Plugin entry point
├── t150_vrs/
│   └── ...
├── t150_propulsion/
│   └── ...
└── CMakeLists.txt    # Top-level build configuration
```

---

## Development Notes

### Adding New Plugins
1. Create a new subdirectory under `plugins/`
2. Copy the structure from an existing plugin
3. Implement the processor class extending `WsfProcessor`
4. Create registration class extending `WsfScenarioExtension`
5. Add plugin entry point with `WsfPluginVersion` and `WsfPluginSetup`
6. Add to the top-level `CMakeLists.txt`

### Testing
Plugins can be tested by:
1. Building in debug mode: `./build.sh debug`
2. Running AFSIM with debug logging
3. Checking AuxData values in execute blocks or Warlock

### Debugging
- Use `ut::log::debug()` for debug output
- Check AFSIM console for plugin load messages
- Verify plugin is loaded: `query_plugin t150_battery`

---

## License

These plugins are part of the T-150 UAS Simulation project.
For AFSIM licensing, see the AFSIM documentation.
