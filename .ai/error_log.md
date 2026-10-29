# Error Log

> Errors encountered during development and their solutions.
> Items marked [PROMOTE] are candidates for the shared Library's error catalog.
>
> Agents: Log errors here with the exact error message, root cause, and fix.
> This helps future sessions avoid the same mistakes.

---

### 2026-10-20 - Wizard custom rotorcraft grammar

Wizard reported `Reference to non-existent rule "int", while processing
WSF_ROTORCRAFT_6DOF_MOVER.` Reproduced all four errors by loading the installed
grammar with `WsfParseDefinitions`, the same API Wizard uses. The four waypoint
index references used `<int>` instead of AFSIM's `<integer>`. Corrected the
source grammar and reinstalled it. The native parser then initialized the
complete installed grammar successfully.

### 2026-10-20 - Windows Conan/CMake integration

- Conan 2.32: `TypeError: Conf.get() got an unexpected keyword argument 'required'`.
  Replaced the unsupported argument with an explicit missing-setting check and
  `ConanInvalidConfiguration`.
- MSVC: `error C3861: 'setenv': identifier not found`. Use `_putenv_s` on Windows,
  preserve `setenv` elsewhere, and report failures.
- MSVC: `error C2065: 'M_PI': undeclared identifier`. Use C++20
  `std::numbers::pi`, preserving the existing formulas.
- MSVC: `error LNK2019` for imported rotorcraft mover vtables. The custom class
  incorrectly used `WSF_EXPORT`; it now uses its own generated
  `ROTORCRAFT_6DOF_EXPORT`. Disabled redundant automatic exports on that target.
- Battery grammar install rule referenced a file absent from the source tree.
  Match the existing optional weapon-grammar pattern so installation succeeds.
- CTest initially failed with `Run the windows-install build preset: missing
  C:/code/AFSIM/AFSIM/mission_plugins/rotorcraft_6dof.dll`. This was intentional:
  it verified that a Release build does not install. The separate install target
  then deployed all DLLs and the test passed.
