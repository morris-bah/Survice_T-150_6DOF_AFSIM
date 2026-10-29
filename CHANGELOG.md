# Changelog

All notable structural and content changes to project005_t150_uas.

## [2.1.0] — 2026-05-19

### Repository restructure for analyst/evaluator discoverability + Agentic-AI framework integration

Reorganization driven by external evaluation needs. Project content
(plugins, scenarios, V&V documentation) is unchanged; this release
reorganizes layout and adds AI agent scaffolding.

#### Added
- `.ai/` directory with `system_instructions.md`, `project_context.md`, `tool_paths.json`, and learning logs
  (`session_journal.md`, `discoveries.md`, `error_log.md`, `reusable_components.md`).
  Wires the project into the NGIC AFSIM Agentic-AI framework at `C:\afsim_projects\` /
  `C:\afsim\.library\`.
- `AGENTS.md`, `GEMINI.md` agent entry points (Codex CLI, Gemini CLI).
- `scenarios/README.md` — newcomer's index to all scenario groups with suggested run order.
- `scenarios/INDEX.csv` — machine-readable scenario catalog (path, group, purpose, plugins exercised, V&V doc reference).
- `docs/archive/README.md` — describes archive contents and rationale.
- `docs/archive/restructure_audit_2026-05-19.txt` — pre-rename grep report capturing every external reference to old scenario names.
- `CHANGELOG.md` — this file.

#### Changed (scenario reorganization)

Every scenario was grouped into a purpose folder and renamed with an `NN_` prefix mirroring the existing `phase1_tests/` convention. Include paths updated from `../platforms/...` to `../../platforms/...` (scenarios now live one level deeper).

| Old path | New path |
|---|---|
| `scenarios/battery_processor_test.afsim` | `scenarios/batteries/01_processor_test.afsim` |
| `scenarios/battery_range_test.afsim` | `scenarios/batteries/02_range_test.afsim` |
| `scenarios/battery_range_test_no_payload.afsim` | `scenarios/batteries/03_range_test_no_payload.afsim` |
| `scenarios/battery_test_straight.afsim` | `scenarios/batteries/04_load_straight.afsim` |
| `scenarios/battery_test_sine.afsim` | `scenarios/batteries/05_load_sine.afsim` |
| `scenarios/battery_test_square.afsim` | `scenarios/batteries/06_load_square.afsim` |
| `scenarios/battery_test_sawtooth.afsim` | `scenarios/batteries/07_load_sawtooth.afsim` |
| `scenarios/battery_test_cycloid.afsim` | `scenarios/batteries/08_load_cycloid.afsim` |
| `scenarios/t150_6dof_simple_test.afsim` | `scenarios/6dof/01_simple_test.afsim` |
| `scenarios/t150_6dof_basic_test.afsim` | `scenarios/6dof/02_basic_test.afsim` |
| `scenarios/t150_6dof_test.afsim` | `scenarios/6dof/03_general_test.afsim` |
| `scenarios/t150_6dof_hover_test.afsim` | `scenarios/6dof/04_hover_test.afsim` |
| `scenarios/t150_6dof_telemetry_test.afsim` | `scenarios/6dof/05_telemetry_test.afsim` |
| `scenarios/t150_6dof_flight_demo.afsim` | `scenarios/6dof/06_flight_demo.afsim` |
| `scenarios/ground_effect_test.afsim` | `scenarios/aerodynamics/01_ground_effect.afsim` |
| `scenarios/payload_dynamics_test.afsim` | `scenarios/aerodynamics/02_payload_dynamics.afsim` |
| `scenarios/t150_attitude_extreme_test.afsim` | `scenarios/aerodynamics/03_attitude_extreme.afsim` |
| `scenarios/spline_behavior_test.afsim` | `scenarios/navigation/01_spline_behavior.afsim` |
| `scenarios/slung_load_test.afsim` | `scenarios/slung_load/01_basic_dynamics.afsim` |
| `scenarios/weapon_recoil_test.afsim` | `scenarios/weapons/01_recoil.afsim` |
| `scenarios/weapon_payload_drop_test.afsim` | `scenarios/weapons/02_payload_drop.afsim` |
| `scenarios/circular_engagement_test.afsim` | `scenarios/weapons/03_circular_engagement.afsim` |
| `scenarios/radar_tracking_demo.afsim` | `scenarios/radar/01_tracking_demo.afsim` |
| `scenarios/radar_vertical_detection.afsim` | `scenarios/radar/02_vertical_detection.afsim` |
| `scenarios/t150_flight_demo.afsim` | `scenarios/demos/01_mission_flight_demo.afsim` |
| `scenarios/t150_capability_demo.afsim` | `scenarios/demos/02_capability_demo.afsim` |
| `scenarios/t150_validation_suite.afsim` | `scenarios/demos/03_validation_suite.afsim` |
| `scenarios/t150_engineering_test.afsim` | `scenarios/demos/04_engineering_test.afsim` |
| `scenarios/t150_data_logging.afsim` | `scenarios/demos/05_data_logging.afsim` |

`scenarios/phase1_tests/` retained as-is (already `NN_`-prefixed historical validation set).

#### Changed (top-level)

- Moved `T150_Plugin_Presentation_Outline.md` → `docs/T150_Plugin_Presentation_Outline.md`.
- Archived 10 stale session/planning docs from `docs/` → `docs/archive/`.
- Moved unused `scripts/flight_path_generator.afsim` → `docs/archive/legacy_scripts/`; removed empty `scripts/` directory.
- Updated `tools/plot_engineering_data.py` to reference new `scenarios/demos/02_capability_demo.afsim` path.
- Rewrote root `README.md` with analyst quick-start as the lead section; deep-dive content moved below.
- Updated `.claude/CLAUDE.md` for GitLab migration (replaces obsolete GitHub auth section).

#### Removed

- `lib/t150_battery.dll`, `lib/t150_propulsion.dll`, `lib/t150_slung_load.dll`, `lib/t150_vrs.dll` — confirmed MD5-identical duplicates of `plugins/build/lib/*.dll`.
- `install_plugins.bat`, `install_plugins.sh` — superseded by `ngic_afsim_build.sh` / `ngic_afsim_install.sh`.

#### Migrated remotes

- GitHub (`DSGarrett/project005_t150_uas`) → GitLab (`code.cdso.army.mil/ngic_fsd/model_development/t150_afsim_ngic-development`).
- Pre-migration history retained as the local `github-archive` remote (GitHub network-unreachable; treat as museum). GitLab `main` starts from a single signed initial import (`fa51bbd`, 2026-05-19) per pre-receive signing policy.

#### Not changed

- Plugin source code under `plugins/<name>/` — no changes.
- Formal V&V deliverables in `VandV_docs/Final_Documents/` retain their v1.0 citations of old scenario names (frozen artifacts). A v1.1 advisory note has been added to TDD and V&V Report pointing to this changelog for path translation.
- Older V&V docs in `VandV_docs/Version1and2/` left untouched — citations reflect project state at the time of writing.
- `output/` AER/CSV files — historical run artifacts, names not changed.

---

## [2.0.x] — earlier 2026

See `git log github-archive/main` for pre-migration history. Notable milestones:
- 2.0: Phases 3–6 (dynamic power model, ground effect, payload dynamics, slung load, 6DOF mover, weapon plugin)
- v2.0 V&V documentation suite delivered

## [1.x] — 2025

- 1.0: Initial release with flight dynamics and energy tracking
- 1.1: Phase 1–2 (thermal tuning, VRS mover integration, voltage sag model)
