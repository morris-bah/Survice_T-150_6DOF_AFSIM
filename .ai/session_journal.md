# Session Journal

> Auto-populated by AI agents. Records learnings, decisions, and discoveries
> from each development session. Newest entries appear first.
>
> Agents: Append a summary to this file at the end of each work session.
> Mark entries with [PROMOTE] if they should be shared to the global Library.

---

### 2026-10-20 - Correcting Wizard rotorcraft grammar

**Task:** Investigate Wizard failing after the custom rotorcraft grammar is
installed alongside the base AFSIM grammars.
**Accomplished:** Reproduced four unresolved `int` rule references with a small
native checker using Wizard's `WsfParseDefinitions` API. Replaced the four
waypoint index tokens with documented/existing `<integer>` tokens, reinstalled
the grammar, and verified the complete installed grammar initializes.
**Scope:** No mover runtime behavior or DLL code changed.

### 2026-10-20 - Windows Conan and CMake build/install

**Task:** Build all six plugins with Conan, CMake, and installed VS 2026 Build
Tools; separate Release build from installation into `C:\code\AFSIM\AFSIM`.

**Accomplished:** Added a local AFSIM SDK Conan recipe, pinned Windows profile,
dependency manifest/lockfile, root CMake project, configure/build/test presets,
shared plugin install rules, and an installed-plugin CTest smoke test. Updated
VS Code's local CMake settings to use root presets and documented the workflow.
Fixed three Windows-only compilation/link issues and the absent battery grammar
install rule. Existing Linux shell workflows remain available.

**Key decisions:** Consume the completed build at
`C:\code\AFSIM\afsim_2.2505.0\build` (formerly `build2`, actual version 2.2505.3).
Do not modify AFSIM source or rebuild its stale generated VS projects. Package
its headers/import libraries locally and deploy only this project's plugins
and available grammar to the flat Windows runtime layout.

**Verification:** Offline Conan export/install, VS 2026 configure and all-six
Release build succeeded. Before install, CTest correctly rejected missing
deployment. The separate install target deployed twelve DLL copies and the
rotorcraft grammar. CTest passed SHA-256 checks and confirmed all six plugins
load through the installed `mission.exe`.

**Open items:** Flight scenario/physics validation and Linux builds were not run.
Several existing scenarios contain Linux-specific output paths; those were
left unchanged because this task concerns building and installing.

### 2026-10-29 - Battery CSV path substitution

**Task:** Diagnose why `test11_endurance.afsim` writes its event file but not
the configured battery CSV.
**Accomplished:** Changed `BatteryProcessor`'s `csv_output_file` input to use
`UtInput::ReadPath()`, which expands scenario path variables before the plugin
opens the CSV. The existing scenario output configuration remains unchanged.
**Verification:** C++ editor diagnostics report no errors. The active CMake
Tools session has no configured targets for this separate project, so the
plugin was not rebuilt and runtime CSV creation remains to be verified after
rebuilding/reinstalling the plugin.

### 2026-10-29 - Endurance distance reporting

**Task:** Accumulate total platform distance during the endurance scenario and
report it at battery depletion.
**Accomplished:** Added one-second location sampling, summed 3D slant ranges,
and included the total in the depletion report in kilometers.
**Verification:** Scenario editor diagnostics report no errors. The scenario
was not rerun to avoid overwriting the existing output files.

### 2026-10-29 - Endurance route initialization

**Task:** Resolve the dynamic-route endurance scenario remaining at zero
altitude and velocity.
**Accomplished:** Restored a statically initialized route so the custom 6DOF
mover establishes its local origin correctly, then copied the route and applied
the configured cruise speed before following it. Added a route-start failure
message.
**Verification:** Scenario editor diagnostics report no errors. Runtime
verification was deferred to preserve the existing scenario output files.
