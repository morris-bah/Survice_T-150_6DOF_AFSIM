# T-150 UAS AFSIM Plugin Suite - Diagram Sources

Extracted ASCII art diagrams from LaTeX V&V documentation for conversion to
professional diagrams via eraser.io or similar diagramming tools.

**Date extracted**: 2026-03-06
**Source documents**: ADD v3.0, UAM v3.0, ICD v1.0

## Diagram Inventory

| # | Filename | Source | Description | Suggested Diagram Type |
|---|----------|--------|-------------|----------------------|
| 01 | `01_ADD_architecture_overview.txt` | ADD 3.1 | AFSIM Runtime containing WsfPlatform with Rotorcraft6DOF mover, AuxData store, and 4 processor plugins. Shows blackboard communication pattern. | Block diagram / System architecture |
| 02 | `02_ADD_uml_class_hierarchy.txt` | ADD 3.2 | Full UML class hierarchy: WsfMover -> WsfRouteMover -> Rotorcraft6DOF_Mover (with RigidBody6DOF, RotorModel composition) and WsfProcessor -> 4 processor subclasses. Includes attributes and methods. | UML class diagram |
| 03 | `03_ADD_plugin_registration_sequence.txt` | ADD 3.3 | Two-level AFSIM plugin registration: WsfPluginVersion -> WsfPluginSetup -> RegisterFeature -> RegisterExtension -> ScenarioCreated -> AddCoreType. Standard API contract for all 5 plugins. | Sequence diagram |
| 04 | `04_ADD_update_position_sequence.txt` | ADD 3.7 | Core physics loop: dt computation, air density, navigation (spline/waypoint), control mixing, per-rotor update, force aggregation, rigid body integration, AFSIM sync. | Flowchart / Activity diagram |
| 05 | `05_ADD_spline_navigation_state_machine.txt` | ADD 3.8 | Spline navigation states: APPROACHING -> behavior decision (FLY_THROUGH/SLOW_PASS/STOP_AND_HOVER) -> AT_WAYPOINT -> ROTATING -> DEPARTING -> loop back. | State machine diagram |
| 06 | `06_ADD_battery_lifecycle_sequence.txt` | ADD 4.2 | AFSIM-to-BatteryProcessor lifecycle: ProcessInput (config), Initialize (SOC/OCV), Update loop (power/SOC/voltage per timestep), Destructor (CSV summary). | Sequence diagram |
| 07 | `07_ADD_power_source_resolution.txt` | ADD 4.3 | 4-level fallback chain for battery power source: configured key -> 6dof_total_power -> PROP_TOTAL_POWER -> hover_power_w static fallback. | Flowchart / Decision tree |
| 08 | `08_ADD_motor_state_array.txt` | ADD 5.2 | MotorData struct definition: array of 8 motors with state enum (OFF/STARTING/RUNNING/OVERTEMP/FAILED), rpm, current, temperature, efficiency, thrust, torque, throttle. | Data structure / Class diagram |
| 09 | `09_ADD_propulsion_update_loop.txt` | ADD 5.3 | PropulsionProcessor Update(): velocity/acceleration, required thrust, ground effect, thrust distribution, per-motor update loop (RPM/thermal/health), AuxData publish. | Flowchart / Activity diagram |
| 10 | `10_ADD_motor_state_machine.txt` | ADD 5.4 | Motor state transitions: OFF -> STARTING -> RUNNING -> OVERTEMP -> FAILED (terminal). Temperature-driven degradation path. | State machine diagram |
| 11 | `11_ADD_vrs_state_machine.txt` | ADD 6.2 | VRS state machine: CLEAR -> CAUTION -> ONSET -> DEVELOPED -> RECOVERY -> CLEAR. Margin-driven transitions with development and recovery timing. | State machine diagram |
| 12 | `12_ADD_slung_load_integration_loop.txt` | ADD 7.2 | Pendulum integration: state check, velocity/acceleration, angular acceleration equations, semi-implicit Euler, cable tension, reaction forces, resonance detection. | Flowchart / Activity diagram |
| 13 | `13_ADD_slung_load_state_machine.txt` | ADD 7.3 | Slung load states: DETACHED -> ATTACHED -> SWINGING -> RESONANCE, with ReleaseLoad() -> RELEASED from any attached state. | State machine diagram |
| 14 | `14_ADD_build_directory_structure.txt` | ADD 9.1 | CMake build system directory tree: plugins/ with 5 plugin subdirectories, each containing CMakeLists.txt, source/, grammar/. | Directory tree / File structure |
| 15 | `15_UAM_functional_block_diagram.txt` | UAM 3.1 | Top-level functional block diagram showing 6DOF Mover (with RigidBody6DOF, RotorModel[8], FlightController), 3 processors, Platform AuxData, and Slung Load processor. Data flow with AuxData key names. | Block diagram / System architecture |
| 16 | `16_ICD_plugin_lifecycle.txt` | ICD 2.2 | 5-stage plugin lifecycle: Load -> Version -> Setup -> Scenario -> Registration. Shows AFSIM/plugin interaction at each stage. | Sequence diagram / Timeline |
| 17 | `17_ICD_inter_plugin_data_flow.txt` | ICD 5.1 | Comprehensive data flow diagram with all AuxData keys. Shows producer/consumer relationships between all 5 plugins, including fallback chain and Platform API interfaces. | Data flow diagram |

## Summary by Source Document

- **ADD (Architecture Description Document)**: 14 diagrams (01-14)
  - 2 architecture/block diagrams
  - 1 UML class diagram
  - 3 sequence/flowchart diagrams
  - 3 state machine diagrams
  - 3 activity/flowchart diagrams
  - 1 data structure diagram
  - 1 directory tree

- **UAM (User and Analyst Manual)**: 1 diagram (15)
  - 1 block diagram

- **ICD (Interface Control Document)**: 2 diagrams (16-17)
  - 1 lifecycle sequence
  - 1 data flow diagram

## File Format

Each `.txt` file contains:
1. **Header comments** (`##` lines): Source document, section, description, and context for eraser.io
2. **Raw ASCII art**: The original diagram text extracted from LaTeX verbatim/lstlisting blocks
3. **Eraser.io prompt**: A suggested prompt/description for converting to a professional diagram, including the recommended diagram type

## Usage with eraser.io

1. Open the `.txt` file for the diagram you want to convert
2. Copy the "Suggested eraser.io prompt" section
3. Paste into eraser.io's AI diagram generation
4. Optionally paste the raw ASCII art as additional context
5. Refine the generated diagram as needed
