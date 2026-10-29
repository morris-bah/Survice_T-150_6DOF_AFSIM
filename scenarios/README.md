# Scenarios

This directory contains all AFSIM scenarios for the T-150 UAS simulation,
organized by purpose. Every scenario is self-contained (loads platform / sensor
/ signature definitions via `../../` includes) and produces output in `../output/`
when run with the AFSIM `mission` executable.

## Quick start

**New to the project?** Start here:

```bash
# Linux/WSL:
/opt/afsim2.2505/bin/mission scenarios/demos/01_mission_flight_demo.afsim

# Windows:
C:\afsim\AFSIM-2.2505.2\bin\mission.exe scenarios\demos\01_mission_flight_demo.afsim
```

This runs the headline mission demonstration — full flight profile with
battery, propulsion, VRS, and slung load all active. Outputs `01_mission_flight_demo.aer`
(viewable in Mystic) and `01_mission_flight_demo.log` to `output/`.

## Folder map

| Folder | What it demonstrates | Run order |
|---|---|---|
| `demos/` | Mission-level capability demos. Start here for evaluation. | `01_mission_flight_demo` → others as interest dictates |
| `phase1_tests/` | Original Phase 1 validation suite — historical reference for v1.x physics fidelity claims | numbered, run in order |
| `6dof/` | Rotorcraft-6DOF mover validation. Isolated tests of motor response, attitude control, hover, telemetry. | `01_simple_test` → `06_flight_demo` |
| `batteries/` | Li-Ion battery characterization under different load profiles (constant, sine, square, sawtooth, cycloid). | `01_processor_test` → `08_load_cycloid` |
| `aerodynamics/` | Aerodynamic-phenomenon isolation: ground effect, payload mass dynamics, extreme attitude regimes. | any order |
| `navigation/` | Catmull-Rom spline waypoint navigation with per-waypoint behavior modes (FLY_THROUGH / SLOW_PASS / STOP_AND_HOVER). | `01_spline_behavior` |
| `slung_load/` | Cable-suspended payload dynamics — pendulum motion, tension, attachment effects. | `01_basic_dynamics` |
| `weapons/` | Weapon employment: recoil reaction on the 6DOF platform, payload drop ballistics, circular engagement geometry. | `01_recoil` → `03_circular_engagement` |
| `radar/` | Ground-based radar tracking of the T-150 platform. Exercises sensor + signature plugins. | `01_tracking_demo` → `02_vertical_detection` |

## Naming convention

`scenarios/<group>/NN_<descriptive_name>.afsim`

- `NN_` is a two-digit ordering hint (lowest first). The order reflects increasing complexity within each group, not a hard prerequisite chain.
- `<descriptive_name>` is lowercase with underscores. The `t150_` prefix is omitted from filenames (the entire project is T-150 — prefix was noise).

## Include path convention

Scenarios use relative includes one level up to reach platform / sensor / signature definitions:

```afsim
include ../../platforms/t150_6dof_platform.afsim
include ../../sensors/ground_surveillance_radar.afsim
include ../../signatures/t150_radar_signature.afsim
```

If you add a new scenario in `scenarios/<group>/`, follow the same `../../` pattern.

## Adding a new scenario

1. Identify the group it belongs to (or create one — keep groups purpose-focused, not feature-named).
2. Number it as the next `NN_` in that group.
3. Use `../../platforms/<file>.afsim` for includes.
4. Add a row to `INDEX.csv` documenting its purpose and which plugins it exercises.
5. If it validates a specific V&V claim, note the V&V doc reference in `INDEX.csv`.

## Pre-restructure name mapping

For historical reference: `CHANGELOG.md` at the repository root contains the full
old→new path mapping from the v2.0 → v2.1 restructure (May 2026). Formal V&V
documents in `VandV_docs/Final_Documents/` cite the pre-restructure names; use
the changelog to translate.
