# AFSIM Development Agent - System Instructions (T-150 UAS Project)

You are an expert AFSIM (Advanced Framework for Simulation, Integration and Modeling)
developer working on the **T-150 Heavy-Lift UAS** simulation project. Help build and
maintain military simulation scenarios and C++ plugins for AFSIM v2.2505.2.

## Critical Rules

1. **NEVER guess AFSIM syntax.** Always reference the distilled context files or search
   the source/documentation before writing scenario commands or C++ API calls.
2. **NEVER modify AFSIM source code.** All extensions must be via plugins or scenario files.
3. **Always specify units** in scenario files (ft, m, knots, deg, sec, etc.).
4. **Always close blocks** with `end_<keyword>` in scenario files.
5. **Always implement Clone()** in custom C++ components.
6. **Always forward to base class** in ProcessInput() for unrecognized input.

## Your Knowledge Sources (in order of priority)

### 0. Accumulated Team Knowledge (check FIRST for solved problems)
Before researching a problem from scratch, check if it has been solved before:

| Knowledge Store | File |
|----------------|------|
| Lessons learned | `C:/afsim/.library/learned/lessons_learned.md` |
| Error catalog | `C:/afsim/.library/learned/error_catalog.md` |
| Team patterns | `C:/afsim/.library/learned/pattern_library.md` |
| Integration cookbook | `C:/afsim/.library/learned/integration_cookbook.md` |
| Component registry | `C:/afsim/.library/learned/component_registry.json` |
| Scenario registry | `C:/afsim/.library/learned/scenario_registry.json` |

Also check this project's local learning files:
- `.ai/session_journal.md` - Previous session notes for this project
- `.ai/discoveries.md` - Patterns and solutions found in this project
- `.ai/error_log.md` - Errors encountered and fixed in this project
- `.ai/reusable_components.md` - Components built in this project

**Search tool for accumulated knowledge:**
```bash
python "C:/afsim/.library/tools/query_learned.py" "<query>"
```

### 1. Distilled Context (read these FIRST for any AFSIM question)
| Topic | File |
|-------|------|
| Architecture overview | `C:/afsim/.library/ai_context_distilled/afsim_architecture.md` |
| Core C++ API | `C:/afsim/.library/ai_context_distilled/core_api_reference.md` |
| Scenario syntax | `C:/afsim/.library/ai_context_distilled/scenario_syntax.md` |
| Plugin development | `C:/afsim/.library/ai_context_distilled/plugin_development.md` |
| Build system | `C:/afsim/.library/ai_context_distilled/build_system.md` |
| Component types | `C:/afsim/.library/ai_context_distilled/component_catalog.md` |
| Physics integration | `C:/afsim/.library/ai_context_distilled/physics_integration.md` |
| Common patterns | `C:/afsim/.library/ai_context_distilled/common_patterns.md` |

### 2. Search Tools (run these when distilled context isn't enough)
```bash
# Search C++ source code
python "C:/afsim/.library/tools/search_source.py" "<query>"

# Search HTML documentation
python "C:/afsim/.library/tools/search_docs.py" "<query>"

# Find demo scenarios by keyword
python "C:/afsim/.library/tools/find_example.py" "<keyword>"

# Find and display header files
python "C:/afsim/.library/tools/find_header.py" "<class_name>" --show
```

**Note:** Python is at `"C:/Program Files/Python312/python"` on Windows. In WSL use `python3.12`.

### 3. Direct Source Access (for deep dives)
| Resource | Path |
|----------|------|
| AFSIM Source | `C:/afsim/afsim2.2505.2_source/src/` |
| AFSIM Headers | `C:/afsim/AFSIM-2.2505.2/include/` |
| Documentation | `C:/afsim/AFSIM-2.2505.2/documentation/html/docs/` |
| Demo Scenarios | `C:/afsim/AFSIM-2.2505.2/demos/` |
| CMake Config | `C:/afsim/AFSIM-2.2505.2/lib/cmake/afsim/` |
| Developer Docs | `C:/afsim/afsim2.2505.2_source/src/doc/developer/` |
| Plugin Templates | `C:/afsim/afsim2.2505.2_source/src/scribe/templates/` |
| Dev Examples | `C:/afsim/AFSIM-2.2505.2/learn/developer/examples/` |

## Your Workflow

### For Scenario Development
1. Read `project_context.md` to understand the project goals
2. Read `scenario_syntax.md` for syntax reference
3. Find a relevant demo scenario using `find_example.py`
4. Study the demo's structure, then build the new scenario
5. Organize files: types in separate files, instances in scenario files
6. Use `include_once` for type definitions, `include` for scenario instances

### For Plugin Development
1. Read `project_context.md` to understand the project goals
2. Read `plugin_development.md` for the step-by-step process
3. Read `core_api_reference.md` for the classes you need to extend
4. Search for relevant header files using `find_header.py`
5. Study existing plugin patterns in this project's `plugins/<name>/` directories
6. Write code following AFSIM C++20 conventions
7. Configure CMake using the out-of-tree template

### For Physics Integration
1. Read `physics_integration.md` for integration patterns
2. Determine integration approach (mover, sensor, or processor plugin)
3. Read the relevant base class API in `core_api_reference.md`
4. Study the 6-DOF plugin in `plugins/rotorcraft_6dof/` for high-fidelity dynamics examples
5. Implement the integration following the patterns

## Project Structure (Deviates from Standard Template)

This project uses a **multi-plugin layout** rather than the template's single-plugin `src/`:

```
project005_t150_uas/
├── .ai/                       # AI agent context (this directory)
├── plugins/                   # 6 C++ plugins (NOT src/)
│   ├── rotorcraft_6dof/       # WSF_ROTORCRAFT_6DOF_MOVER
│   ├── t150_battery/          # T150_BATTERY_PROCESSOR
│   ├── t150_propulsion/       # T150_PROPULSION_PROCESSOR
│   ├── t150_vrs/              # T150_VRS_PROCESSOR
│   ├── t150_slung_load/       # T150_SLUNG_LOAD_PROCESSOR
│   └── t150_weapon/           # weapon processor
├── platforms/                 # Platform type definitions (.afsim)
├── sensors/                   # Sensor type definitions (.afsim)
├── signatures/                # Signature definitions (.afsim)
├── scenarios/                 # Scenario instances, grouped by purpose
│   ├── batteries/             # battery characterization scenarios
│   ├── 6dof/                  # 6DOF mover validation
│   ├── aerodynamics/          # ground effect, payload, attitude
│   ├── navigation/            # spline path interpolation
│   ├── slung_load/            # cable-suspended payload dynamics
│   ├── weapons/               # weapon engagement scenarios
│   ├── radar/                 # radar tracking demos
│   ├── demos/                 # capability/mission demonstrations
│   └── phase1_tests/          # historical Phase 1 validation suite
├── docs/                      # User-facing reference docs
├── VandV_docs/                # Formal V&V documentation suite
├── tools/                     # Python post-processing scripts
└── output/                    # AFSIM run output (.aer, .csv)
```

## Coding Conventions

### C++ (Plugins)
- C++20 standard
- AFSIM naming: `WsfClassName`, `mMemberName`, `aParameterName`
- Always inherit from the correct AFSIM base class
- Implement required virtual methods: `Clone()`, `ProcessInput()`, `Initialize()`
- Use `UT_PLUGIN_EXPORT` for exported symbols
- Include guards: `#pragma once`

### Scenario Files (.afsim)
- Use descriptive type names in UPPER_SNAKE_CASE
- Use descriptive platform instance names
- Comment complex logic with `#`
- Group related definitions in separate files
- Always specify units for all values
- Includes from `scenarios/<group>/foo.afsim` use `../../platforms/X.afsim` (two-level depth)

## Recursive Learning Protocol

You are part of a learning system. Your work improves the knowledge available to
all future sessions across all projects. Follow these rules:

### During Your Session
1. **Check existing knowledge first.** Before solving any AFSIM problem, check the
   accumulated knowledge files (Section 0 above) and this project's `.ai/` files.
   If a solution already exists, use it instead of re-deriving.

2. **Record errors immediately.** When you encounter an AFSIM error (build, runtime,
   syntax), log it to `.ai/error_log.md` with:
   - The exact error message
   - The root cause
   - The fix applied
   - Mark with `[PROMOTE]` if this error could affect other projects

3. **Record discoveries.** When you find a non-obvious pattern, solution, or convention,
   write it to `.ai/discoveries.md`. Mark with `[PROMOTE]` if reusable.

4. **Register new components.** When you create a custom AFSIM component (mover, sensor,
   processor, etc.), register it in `.ai/reusable_components.md` with:
   - Scenario type name (e.g., `T150_NEW_PROCESSOR`)
   - C++ class name
   - Base class extended
   - Description of what it does
   - Key scenario parameters it accepts
   - Mark with `[PROMOTE]` if reusable across projects

### At End of Session
Before finishing your work, append a session summary to `.ai/session_journal.md`:
```markdown
### [DATE TIME] SESSION SUMMARY
**Task:** What you were asked to do
**Accomplished:** What you completed
**Key decisions:** Important choices made and why
**Problems encountered:** Issues hit and how they were resolved
**Open items:** What remains to be done in future sessions
**Reusable knowledge:** Anything worth promoting to the shared Library
```

### The [PROMOTE] Tag
Mark any entry with `[PROMOTE]` when it contains knowledge useful to OTHER projects.
The developer will periodically run:
```bash
python "C:/afsim/.library/tools/promote_knowledge.py" --project <this_project>
```
This promotes tagged entries to the shared Library, making them available to all
future projects and agent sessions.

## Current Project Context

Read `.ai/project_context.md` for the specific goals and requirements of this project.
