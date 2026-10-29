# Project Context

## Project Name
T-150 / TRV-150 Heavy-Lift UAS Simulation (project005_t150_uas)

## Project Description
High-fidelity AFSIM simulation of the **Malloy Aeronautics T-150 / SURVICE TRV-150**
heavy-lift uncrewed aerial system. Six custom C++ plugins model 6-DOF flight dynamics,
Li-Ion battery behavior under load, electric motor propulsion with thermal effects,
vortex ring state (VRS) detection, slung-load cable dynamics, and weapon
employment/recoil. Used as a reference platform for unmanned heavy-lift VTOL
modeling and as the subject of a formal V&V documentation package.

## Development Type
- [x] Scenario scripting (.afsim files)
- [x] C++ Plugin development
- [x] Analysis/post-processing
- [ ] External physics model integration (physics is in-tree, not external)

## Key Requirements
- Maintain all 6 plugins (`rotorcraft_6dof`, `t150_battery`, `t150_propulsion`,
  `t150_vrs`, `t150_slung_load`, `t150_weapon`) with their existing AuxData
  interfaces — inter-plugin communication goes through `platform.aux.*` keys
- Preserve V&V traceability: every scenario should be discoverable via
  `scenarios/README.md` and citable from V&V documents
- Build/install via `ngic_afsim_build.sh` + `ngic_afsim_install.sh` (Linux/WSL)
- Windows: root Conan/CMake presets, VS 2026 x64 Release; separate
  `windows-release` and `windows-install` build presets. SDK comes from
  `C:\code\AFSIM\afsim_2.2505.0\build` (actual version 2.2505.3), runtime/install
  prefix is `C:\code\AFSIM\AFSIM` with executables directly in the prefix.
- Plugins must work as both `.so` (Linux) and `.dll` (Windows)
- New analyst should be able to run a representative scenario in under 5 minutes
  following only the root `README.md`

## Domain
- [x] Air
- [x] Ground (slung-load drop points, weapon impact)

## External Dependencies
- AFSIM 2.2505.2 at `C:\afsim\AFSIM-2.2505.2\` (Windows) or `/opt/afsim2.2505/` (Linux)
- Python 3.12 for post-processing (`tools/plot_engineering_data.py`,
  `docs/generate_plugin_diagram.py`)
- python-docx for V&V document generation
- Optional: LaTeX (MiKTeX/TeX Live) for V&V PDF compilation

## Reference Scenarios
- `scenarios/demos/01_mission_flight_demo.afsim` — main capability demonstration
- `scenarios/phase1_tests/` — original Phase 1 validation suite (10 numbered tests)
- AFSIM demos at `C:/afsim/AFSIM-2.2505.2/demos/six_dof/` for 6DOF reference patterns

## Project-Specific Conventions (deviations from template)
- **Multi-plugin layout**: `plugins/<plugin_name>/` instead of single `src/` —
  the template's single-plugin assumption does not fit this project
- **Top-level `platforms/`, `sensors/`, `signatures/`** instead of under `scenarios/`
- **Scenarios use `../../platforms/X.afsim`** (two-level include path) because
  they live one level deeper than the template assumes
- **Inter-plugin communication** uses `WsfAuxData` key-value store, not direct
  C++ coupling. Keys include `BATTERY_SOC`, `BATTERY_VOLTAGE`, `PROP_TOTAL_POWER`,
  `VRS_MARGIN`. New plugins should follow this convention.

## V&V Status (as of 2026-05)
Formal V&V documentation suite delivered in `VandV_docs/Final_Documents/`:
- **NGIC set**: ADD v3.0, ICD v1.0, TDD v1.0, UAM v3.0
- **MIL-STD-3022 set**: Accreditation Plan, V&V Plan, V&V Report, Accreditation Report
- Standards: DoDI 5000.61 (Sep 2024), MIL-STD-3022 (Jan 2008), NGIC 4.0 Templates

## Git Remote
- **Active**: GitLab at `https://code.cdso.army.mil/ngic_fsd/model_development/t150_afsim_ngic-development.git`
- **Archive**: original GitHub repo retained locally as `github-archive` remote
  (network-unreachable). Pre-import history available via `git log github-archive/main`.
- **Commit signing required** by GitLab pre-receive hook — x509 / smimesign already
  configured. Never use `--no-gpg-sign`.

## Notes
- This project is the source for ongoing V&V deliverables; structural changes
  should preserve scenario filenames or update CHANGELOG.md with old→new mapping
- Older session-context docs and superseded planning notes live in `docs/archive/`
- See `CHANGELOG.md` for the v2.0 → v2.1 restructure path mapping
