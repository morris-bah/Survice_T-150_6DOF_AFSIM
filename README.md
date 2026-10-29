# T-150 / TRV-150 Heavy-Lift UAS — AFSIM Simulation

High-fidelity AFSIM simulation of the **Malloy Aeronautics T-150 / SURVICE TRV-150**
heavy-lift uncrewed aerial system. Six custom C++ plugins model 6-DOF flight dynamics,
Li-Ion battery behavior, electric propulsion, vortex ring state, slung load dynamics,
and weapon employment. Validated against AAIB accident-investigation data and
manufacturer specifications.

## Run your first scenario in 5 minutes

Prerequisites: AFSIM 2.2505.2 installed and the six plugins built/installed
(see [Building plugins](#building-plugins)).

```bash
# Linux / WSL — recommended:
/opt/afsim2.2505/bin/mission scenarios/demos/01_mission_flight_demo.afsim

# Windows:
C:\afsim\AFSIM-2.2505.2\bin\mission.exe scenarios\demos\01_mission_flight_demo.afsim
```

This runs the headline mission demonstration — a full flight profile (launch →
straight-line → multiple maneuver patterns → land) with battery, propulsion,
VRS, and slung load all active. Outputs go to `output/`.

**To visualize in Mystic:**
```bash
DISPLAY=:0 /opt/afsim2.2505/bin/mystic output/01_mission_flight_demo.aer
```

## What scenario should I run next?

| If you want to see... | Run |
|---|---|
| Mission-level capability | `scenarios/demos/01_mission_flight_demo.afsim` |
| Battery behavior under different load profiles | anything under `scenarios/batteries/` |
| Pure 6DOF flight dynamics (no battery / propulsion coupling) | `scenarios/6dof/04_hover_test.afsim` |
| Ground effect or payload mass effects | `scenarios/aerodynamics/` |
| Spline path navigation with mixed waypoint behaviors | `scenarios/navigation/01_spline_behavior.afsim` |
| Cable-suspended payload dynamics | `scenarios/slung_load/01_basic_dynamics.afsim` |
| Weapon recoil / engagement | `scenarios/weapons/` |
| Ground-radar tracking of the T-150 | `scenarios/radar/01_tracking_demo.afsim` |

Full scenario catalog: [`scenarios/README.md`](scenarios/README.md). Machine-readable: [`scenarios/INDEX.csv`](scenarios/INDEX.csv).

## Plugins quick reference

| Plugin | Scenario type | Provides |
|---|---|---|
| `rotorcraft_6dof` | `WSF_ROTORCRAFT_6DOF_MOVER` | Physics-based 6DOF rotorcraft mover with Catmull-Rom spline navigation |
| `t150_battery` | `T150_BATTERY_PROCESSOR` | Real-time SOC, voltage sag, Li-Ion discharge curves |
| `t150_propulsion` | `T150_PROPULSION_PROCESSOR` | Motor thermal, ground effect, payload dynamics, dynamic power |
| `t150_vrs` | `T150_VRS_PROCESSOR` | Vortex Ring State detection and warnings |
| `t150_slung_load` | `T150_SLUNG_LOAD_PROCESSOR` | Cable-suspended payload dynamics |
| `t150_weapon` | (weapon processor) | Multi-weapon support, ballistics, recoil reaction |

Plugins communicate via the `WsfAuxData` key-value store. Key values accessible
in scenarios as `platform.aux.VARIABLE_NAME`:
- `BATTERY_SOC` — state of charge (0–1)
- `BATTERY_VOLTAGE` — terminal voltage under load
- `PROP_TOTAL_POWER` — total power consumption (W)
- `VRS_MARGIN` — VRS safety margin (0=VRS, 1=safe)

Full physics documentation: [`docs/T150_PHYSICS_REFERENCE.md`](docs/T150_PHYSICS_REFERENCE.md).
Architecture: [`docs/plugin_architecture.md`](docs/plugin_architecture.md).

## Project structure

```
project005_t150_uas/
├── README.md                     # this file
├── CHANGELOG.md                  # version history with old→new path mapping
├── AGENTS.md / GEMINI.md         # AI agent entry points (Codex / Gemini)
├── .claude/                      # Claude Code project config
├── .ai/                          # Agentic-AI context (NGIC Library integration)
├── ngic_afsim_build.sh           # NGIC generic plugin build
├── ngic_afsim_install.sh         # NGIC generic plugin install (requires sudo)
│
├── plugins/                      # C++ plugin sources (6 plugins)
│   ├── rotorcraft_6dof/          # WSF_ROTORCRAFT_6DOF_MOVER
│   ├── t150_battery/             # T150_BATTERY_PROCESSOR
│   ├── t150_propulsion/          # T150_PROPULSION_PROCESSOR
│   ├── t150_vrs/                 # T150_VRS_PROCESSOR
│   ├── t150_slung_load/          # T150_SLUNG_LOAD_PROCESSOR
│   └── t150_weapon/              # weapon processor
│
├── platforms/                    # platform type definitions (shared)
├── sensors/                      # sensor type definitions
├── signatures/                   # signature definitions
│
├── scenarios/                    # all .afsim scenarios, grouped by purpose
│   ├── README.md                 # newcomer's scenario index
│   ├── INDEX.csv                 # machine-readable catalog
│   ├── demos/                    # capability / mission demonstrations
│   ├── 6dof/                     # 6DOF mover validation
│   ├── batteries/                # battery characterization
│   ├── aerodynamics/             # ground effect, payload, attitude
│   ├── navigation/               # spline path navigation
│   ├── slung_load/               # cable-suspended payload
│   ├── weapons/                  # weapon employment
│   ├── radar/                    # radar tracking
│   └── phase1_tests/             # historical Phase 1 validation suite
│
├── tools/                        # Python post-processing scripts
├── output/                       # AFSIM run outputs (.aer, .log, .csv)
├── docs/                         # user-facing reference documentation
│   └── archive/                  # superseded session notes / planning docs
└── VandV_docs/                   # formal V&V documentation suite
```

## Building plugins

**AFSIM plugins must be installed to the AFSIM runtime plugin directories.**
Building alone does not install them. All workflows build the same six plugins.

### Windows: Conan 2 + CMake + Visual Studio 2026

Prerequisites:
- Visual Studio 2026 Build Tools with the x64 C++ tools (v145) and Windows SDK.
- CMake 4.2+ (the installed VS Build Tools include CMake 4.3).
- Conan 2.32+.
- The existing AFSIM source at `C:\code\AFSIM\afsim_2.2505.0` and its completed
  Release build at `C:\code\AFSIM\afsim_2.2505.0\build` (formerly `build2`).
- The contents of that build's `Release` directory copied into
  `C:\code\AFSIM\AFSIM`, with `mission.exe` directly inside that directory.

The actual AFSIM build version is **2.2505.3**, despite the source directory's
name. The plugins must match its x64 MSVC ABI, dynamic Release CRT (`/MD`), and
hardware-exception support (`/EHa`). Debug builds against this Release SDK are
intentionally rejected.

Run these commands in PowerShell from the project root. First, package the
existing SDK into the **local** Conan cache (once per machine, or after rebuilding
AFSIM). This copies headers and import libraries, not the runtime executables,
and does not rebuild AFSIM or depend on the old `build2` paths in its CMake cache:

```powershell
conan export-pkg .\conan\afsim-sdk `
    --profile:all .\conan\profiles\windows-vs2026 `
    --output-folder .\build\afsim-sdk --no-remote `
    -c "user.afsim:source_dir=C:\code\AFSIM\afsim_2.2505.0" `
    -c "user.afsim:build_dir=C:\code\AFSIM\afsim_2.2505.0\build"
```

Then resolve dependencies, configure, and build:

```powershell
conan install . --profile:all .\conan\profiles\windows-vs2026 `
    --output-folder .\build\windows-release --no-remote
cmake --preset windows-release
cmake --build --preset windows-release --parallel
```

The six DLLs are produced in `build\windows-release\lib`. The Release build
**does not modify the installed AFSIM runtime**.

Install using the separate target, then verify installation:

```powershell
cmake --build --preset windows-install --parallel
ctest --preset windows-installed
```

The install target builds any out-of-date plugins, then installs their DLLs into
both `C:\code\AFSIM\AFSIM\mission_plugins` and
`C:\code\AFSIM\AFSIM\wsf_plugins`. The available rotorcraft grammar is installed
into `C:\code\AFSIM\AFSIM\grammar`; the source tree does not contain a battery
grammar. Other AFSIM files are left untouched. To install already-built artifacts
without invoking a build, use:

```powershell
cmake --install .\build\windows-release --config Release
```

The CTest check compares all twelve installed DLL copies against the six built
DLLs using SHA-256 and launches `mission.exe -help` to verify that AFSIM actually
loads all six plugins with a compatible ABI. This is a deployment smoke test,
not a flight-physics or scenario-validation suite.

The committed Conan profile selects the compiler without changing your default
profile. [conan.lock](conan.lock) pins the SDK recipe revision. Keep the SDK
package local; do not upload AFSIM headers or binaries to public Conan remotes.
If the SDK recipe changes, export it again and explicitly refresh the lockfile:

```powershell
conan lock create . --profile:all .\conan\profiles\windows-vs2026 `
    --no-remote --lockfile="" --lockfile-out conan.lock
```

In VS Code CMake Tools, use this project's root as the source directory and
select the `windows-release` configure preset. Select the `windows-release`
build preset for build-only, or `windows-install` for deployment. Run the Conan
setup commands above before configuring.

For a different runtime destination, configure with
`cmake --preset windows-release -DCMAKE_INSTALL_PREFIX="C:\path\to\AFSIM"`.
The default is always `C:\code\AFSIM\AFSIM`, not a system-wide installation path.
For a Windows distribution with a `bin` layout instead, also override
`AFSIM_PLUGIN_INSTALL_DIR`, `AFSIM_SHARED_PLUGIN_INSTALL_DIR`, and
`AFSIM_GRAMMAR_INSTALL_DIR` to the corresponding relative directories.

### Build and install (recommended, Linux/WSL)

```bash
# Build all plugins (auto-detects AFSIM installation)
./ngic_afsim_build.sh

# Install to AFSIM (requires sudo)
./ngic_afsim_install.sh
```

These shell scripts install Linux `.so` files to both
`/opt/afsim*/lib/wsf_plugins/` and `/opt/afsim*/bin/mission_plugins/`.

### Manual CMake build (Linux, installed AFSIM SDK)

```bash
cmake -S plugins -B plugins/build -DCMAKE_BUILD_TYPE=Release \
    -DAFSIM_ROOT=/opt/afsim2.2505 -DCMAKE_INSTALL_PREFIX=/opt/afsim2.2505
cmake --build plugins/build --parallel
sudo cmake --install plugins/build
```

## T-150 platform specifications

| Parameter | Value | Source |
|---|---|---|
| Configuration | Coaxial Octocopter (8 rotors, 4 arms) | Technical Docs |
| Motors | 8× T-Motor U15II KV100 | FAA Registry |
| Total Thrust | 240 kg | AAIB Report |
| MTOW | 123 kg (271 lbs) | AAIB Report |
| Max Payload | 68 kg (150 lbs) | Manufacturer |
| Empty Weight | ~55 kg | Derived |
| Cruise Speed | 30 m/s (108 km/h) | Manufacturer |
| Max Airspeed | 40+ m/s | AAIB (demonstrated) |
| Max Rate of Climb | 8 m/s | Derived |
| Max Rate of Descent | 4 m/s (VRS limit) | Aerodynamic |
| Max Bank Angle | 60.9° | AAIB (demonstrated) |
| Battery | 13S Li-Ion, 46.8V nominal | Patent |
| Battery Capacity | ~7 kWh (148.75 Ah) | Patent |
| Flight Time | 36 minutes (nominal) | Manufacturer |
| Range | 70 km | Manufacturer |

## Mission profile (demo)

`scenarios/demos/01_mission_flight_demo.afsim` flies at 500m AGL:

1. **Launch** — Takeoff from Start point
2. **Hover** — 2 minutes at Start position
3. **Straight Line** — 2km flight to Point A
4. **Hover** — 1 minute at Point A
5. **Sawtooth Pattern** — 2km zigzag flight to Point B (8 segments, 200m amplitude)
6. **Hover** — 1 minute at Point B
7. **Square Wave Pattern** — 2km to Point C
8. **Hover** — 1 minute at Point C
9. **Sine Wave Pattern** — 2km to Point D
10. **Hover** — 1 minute at Point D
11. **Prolate Cycloid Pattern** — 2km with loops to Point E
12. **Hover** — 1 minute at Point E
13. **Return** — Straight line back to Start
14. **Land** — Descend and land at original position

## V&V documentation

Formal V&V suite delivered under `VandV_docs/Final_Documents/`:

- **NGIC set**: ADD v3.0, ICD v1.0, TDD v1.0, UAM v3.0
- **MIL-STD-3022 set**: Accreditation Plan, V&V Plan, V&V Report, Accreditation Report
- Standards: DoDI 5000.61 (Sep 2024), MIL-STD-3022 (Jan 2008), NGIC 4.0 Templates

Note: V&V documents at v1.0 cite pre-restructure scenario filenames (e.g.,
`t150_validation_suite.afsim`). See [`CHANGELOG.md`](CHANGELOG.md) for the
old → new path mapping.

## Adapting for other drones

The simulation framework is designed to be adapted to other rotary-wing UAS.
To model a different vehicle:

1. **Modify `platforms/t150_mover.afsim`:**
   - Update `performance_limits` with operational values
   - Update `global_limits` with physical envelope constraints
   - Adjust performance tables for altitude/speed effects

2. **Modify `platforms/t150_platform.afsim`:**
   - Update `script_variables` with battery specifications
   - Adjust power consumption values
   - Modify mass properties

| Parameter | Location | Description |
|---|---|---|
| `available_airspeed` | mover | Max cruise speed |
| `maximum_rate_of_ascent/descent` | mover | Climb/sink rates |
| `battery_capacity_wh` | platform | Total energy capacity |
| `power_hover_*` | platform | Hover power requirements |
| `mass_*_kg` | platform | Mass properties |

Full guidance: [`docs/uas_adaptation_guide.md`](docs/uas_adaptation_guide.md).

## Telemetry output

During simulation, the platform outputs telemetry every 5 seconds:
- Position (lat/lon/alt)
- Speed and heading
- Flight phase
- Power consumption
- Battery state
- Remaining flight time estimate

## Known limitations

1. **Terrain** — no terrain model; flat terrain assumed
2. **Wind** — no wind effects modeled (future enhancement)
3. **Payload** — static payload; no dynamic loading/unloading
4. **Hover Hold** — uses 0-speed waypoints; true hover loiter requires additional scripting

## AI-assisted development

The project is wired into the NGIC AFSIM Agentic-AI framework. Launch any
supported AI coding tool from the project root and it will pick up project
context plus shared AFSIM knowledge from `C:\afsim\.library\`:

```bash
claude    # Claude Code   - reads CLAUDE.md (via .claude/) + .ai/
codex     # Codex CLI     - reads AGENTS.md + .ai/
gemini    # Gemini CLI    - reads GEMINI.md + .ai/
```

Per-project AI context lives in [`.ai/project_context.md`](.ai/project_context.md).

## References

- AAIB Accident Investigation Report (South Scarle, 2023)
- US Patent US20240262499A1 (Battery topology)
- FAA Aircraft Registry (TRV-150)
- T-Motor U15II Specifications
- SURVICE / BAE Systems Product Information

## Version history

| Version | Date | Changes |
|---|---|---|
| 2.1 | 2026-05-19 | Repository restructure, agentic-AI integration, GitLab migration. See [CHANGELOG.md](CHANGELOG.md). |
| 2.0 | 2026-01-08 | Phase 3–6: dynamic power, ground effect, payload dynamics, slung load, 6DOF mover, weapon plugin |
| 1.1 | 2025-11-29 | Phase 1–2: thermal tuning, VRS mover integration, voltage sag |
| 1.0 | 2025-11-25 | Initial release with full flight dynamics and energy tracking |

## Git

- **Remote**: GitLab — `https://code.cdso.army.mil/ngic_fsd/model_development/t150_afsim_ngic-development.git`
- **Commit signing required** (x509 / smimesign). See `.claude/CLAUDE.md`.
- Pre-restructure history: see local `github-archive` remote (network-unreachable).

## Contact

Project maintained as part of the NGIC AFSIM model development collection.
