# T-150 UAS Project - Codex CLI Instructions

Read `.ai/system_instructions.md` for complete AFSIM development guidelines.
Read `.ai/project_context.md` for what this specific project is building.

## Quick Reference

### Before Writing ANY AFSIM Code
1. Check accumulated knowledge first: `"C:/Program Files/Python312/python" "C:/afsim/.library/tools/query_learned.py" "<topic>"`
2. Check this project's `.ai/session_journal.md` and `.ai/error_log.md` for prior work
3. Read the relevant distilled context file from `C:/afsim/.library/ai_context_distilled/`
4. Never guess AFSIM syntax - always reference documentation
5. Search source when needed: `"C:/Program Files/Python312/python" "C:/afsim/.library/tools/search_source.py" "<query>"`

### Recursive Learning (IMPORTANT)
- **Log errors** to `.ai/error_log.md` when you encounter them
- **Log discoveries** to `.ai/discoveries.md` when you find reusable patterns
- **Log new components** to `.ai/reusable_components.md` when you build them
- **Write a session summary** to `.ai/session_journal.md` at end of work
- **Mark with [PROMOTE]** anything useful to other projects

### Key Paths
- AFSIM Install: `C:/afsim/AFSIM-2.2505.2/`
- AFSIM Source: `C:/afsim/afsim2.2505.2_source/src/`
- Knowledge Base: `C:/afsim/.library/`
- Documentation: `C:/afsim/AFSIM-2.2505.2/documentation/html/docs/`
- Demo Scenarios: `C:/afsim/AFSIM-2.2505.2/demos/`

### Project-Specific Notes
This project has a **multi-plugin layout** (`plugins/<name>/`), not the template's
single-plugin `src/` layout. Plugin sources live in `plugins/t150_battery/`,
`plugins/t150_propulsion/`, etc. Build with `./ngic_afsim_build.sh` (WSL/Linux)
and install with `./ngic_afsim_install.sh` (requires sudo).

### Scenario Files (.afsim)
- Always close blocks with `end_<keyword>`
- Always specify units (ft, m, knots, deg, sec)
- Use `include_once` for type definitions
- Reference: `C:/afsim/.library/ai_context_distilled/scenario_syntax.md`
- This project's scenarios are organized under `scenarios/<group>/NN_*.afsim` — see `scenarios/README.md`

### C++ Plugins
- C++20 standard, build with CMake
- Must implement `Clone()`, `ProcessInput()`, `Initialize()`
- Plugin entry: `WsfPluginVersion()` + `WsfPluginSetup()` (extern "C")
- Reference: `C:/afsim/.library/ai_context_distilled/plugin_development.md`

### Search Tools
```bash
# Search source code
"C:/Program Files/Python312/python" "C:/afsim/.library/tools/search_source.py" "WsfSensor"

# Search documentation
"C:/Program Files/Python312/python" "C:/afsim/.library/tools/search_docs.py" "radar"

# Find example scenarios
"C:/Program Files/Python312/python" "C:/afsim/.library/tools/find_example.py" "six_dof"

# Find header files
"C:/Program Files/Python312/python" "C:/afsim/.library/tools/find_header.py" "WsfMover" --show
```

### Build Commands
```bash
# Preferred: use the project's NGIC wrapper scripts (from WSL)
./ngic_afsim_build.sh
./ngic_afsim_install.sh

# Manual per-plugin build:
cd plugins && ./build.sh
```
