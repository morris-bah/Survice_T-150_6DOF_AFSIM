# Discoveries

> Patterns, solutions, and insights discovered during development.
> Items marked [PROMOTE] are candidates for the shared Library.
>
> Agents: Write here when you discover something reusable - a pattern that
> worked well, a non-obvious solution, or a convention worth standardizing.

---

### 2026-10-20 - [PROMOTE] Wizard grammar validation

AFSIM grammar uses `<integer>`, not `<int>`, for integer tokens. DLL loading
checks do not validate editor grammar. Wizard loads all `.ag` files in its
grammar directory using `WsfParseDefinitions::AddGrammar`, then `Initialize`;
`GetDefinitionErrors` exposes unresolved references. This API can reproduce
Wizard grammar failures without starting its GUI.

### 2026-10-20 - [PROMOTE] Windows AFSIM local SDK and flat runtime layout

- The source directory `afsim_2.2505.0` contains a completed build reporting
  AFSIM 2.2505.3. Always check the build's actual version and runtime ABI.
- Its build directory was renamed from `build2` to `build`; generated VS
  projects/cache contain stale absolute paths. Package source headers,
  generated headers, and existing import libraries rather than importing or
  rebuilding those stale projects.
- The copied Windows `Release` directory is a runtime-only installation:
  `mission.exe`, `mission_plugins`, and `wsf_plugins` are directly under
  `C:\code\AFSIM\AFSIM`, not under `bin`/`lib`.
- VS 2026 uses CMake generator `Visual Studio 18 2026`, toolset `v145`, Conan
  `compiler=msvc`, `compiler.version=195`, dynamic Release CRT, and C++20.
  AFSIM's observed plugin ABI is `x86_64-windows-msvc^msvc1951,hwe`; `/EHa`
  and `PROMOTE_HARDWARE_EXCEPTIONS` are required to match it.
- Local `afsim-sdk/2.2505.3` exposes `AFSIM::wsf` through Conan CMakeDeps.
  It must not be uploaded to public remotes.

### AFSIM plugin path variables in file inputs

Custom plugin commands that accept output paths must call `UtInput::ReadPath()`
to expand `define_path_variable` references such as `$(OUTPUT_DIR)`. A plain
`ReadValue()` preserves the token literally, causing a later `ofstream::open()`
to fail even when built-in commands such as `event_pipe file` resolve the same
path correctly.

### Scenario distance accumulation

For a scenario-level traveled-distance estimate, periodically sample the
platform's `WsfGeoPoint` location and add `previous.SlantRangeTo(current)` to a
running total. This sums 3D point-to-point distance in meters; shorter sample
intervals more closely approximate curved paths.

For the custom rotorcraft 6DOF mover, keep the initial route in the platform
input so `InitFromAFSIM()` captures its first waypoint as the local-frame
origin. A runtime `FollowRoute()` only updates route guidance; it does not
reinitialize the mover's internal rigid-body position. To parameterize a route,
copy `PLATFORM.Route()`, modify the copy, and follow it after initialization.
