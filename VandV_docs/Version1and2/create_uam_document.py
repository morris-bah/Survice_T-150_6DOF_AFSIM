#!/usr/bin/env python3
"""
T-150 UAS User and Analyst Manual (UAM) Generator
Following NGIC/TMAP V&V template format
"""

from docx import Document
from docx.shared import Inches, Pt, Cm
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn
from docx.oxml import OxmlElement
import datetime

def set_cell_shading(cell, fill_color):
    """Set cell background color"""
    shading_elm = OxmlElement('w:shd')
    shading_elm.set(qn('w:fill'), fill_color)
    cell._tc.get_or_add_tcPr().append(shading_elm)

def add_callout_box(doc, title, content, color='E7F3FF'):
    """Add a callout/note box"""
    table = doc.add_table(rows=1, cols=1)
    table.style = 'Table Grid'
    cell = table.rows[0].cells[0]
    set_cell_shading(cell, color)

    p = cell.paragraphs[0]
    title_run = p.add_run(f'{title}\n')
    title_run.bold = True
    p.add_run(content)

    doc.add_paragraph()  # Spacing after box

def create_uam_document():
    doc = Document()

    # Set document margins
    sections = doc.sections
    for section in sections:
        section.top_margin = Cm(2.54)
        section.bottom_margin = Cm(2.54)
        section.left_margin = Cm(2.54)
        section.right_margin = Cm(2.54)

    # =========================================================================
    # TITLE PAGE
    # =========================================================================
    title_para = doc.add_paragraph()
    title_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    title_run = title_para.add_run("\n\n\n\nUSER AND ANALYST MANUAL\n\n")
    title_run.bold = True
    title_run.font.size = Pt(24)

    subtitle = doc.add_paragraph()
    subtitle.alignment = WD_ALIGN_PARAGRAPH.CENTER
    sub_run = subtitle.add_run("T-150/TRV-150 Heavy-Lift UAS Simulation\n")
    sub_run.bold = True
    sub_run.font.size = Pt(18)

    subtitle2 = doc.add_paragraph()
    subtitle2.alignment = WD_ALIGN_PARAGRAPH.CENTER
    sub_run2 = subtitle2.add_run("AFSIM Project 005\n\n")
    sub_run2.font.size = Pt(14)

    version_para = doc.add_paragraph()
    version_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    version_para.add_run(f"\n\nVersion 1.0\n{datetime.date.today().strftime('%d %B %Y')}\n\n")

    class_para = doc.add_paragraph()
    class_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    class_run = class_para.add_run("UNCLASSIFIED // FOR OFFICIAL USE ONLY")
    class_run.bold = True

    doc.add_page_break()

    # =========================================================================
    # REVISION HISTORY
    # =========================================================================
    doc.add_heading('Revision History', level=1)

    rev_table = doc.add_table(rows=3, cols=4)
    rev_table.style = 'Table Grid'

    header_cells = rev_table.rows[0].cells
    headers = ['Version', 'Date', 'Author', 'Description']
    for i, header in enumerate(headers):
        header_cells[i].text = header
        set_cell_shading(header_cells[i], 'D9E2F3')
        header_cells[i].paragraphs[0].runs[0].bold = True

    rev_data = [
        ['1.0', datetime.date.today().strftime('%Y-%m-%d'), 'Project Team', 'Initial release'],
        ['0.9', '2025-12-01', 'Project Team', 'Draft for review'],
    ]
    for row_idx, row_data in enumerate(rev_data):
        row = rev_table.rows[row_idx + 1]
        for col_idx, cell_data in enumerate(row_data):
            row.cells[col_idx].text = cell_data

    doc.add_page_break()

    # =========================================================================
    # TABLE OF CONTENTS
    # =========================================================================
    doc.add_heading('Table of Contents', level=1)
    toc_para = doc.add_paragraph()
    toc_para.add_run('(Auto-generate in Word: References > Table of Contents)\n\n')
    toc_para.add_run('1. Introduction\n')
    toc_para.add_run('2. Threat System Description\n')
    toc_para.add_run('3. Model Description\n')
    toc_para.add_run('4. Threat Model Components\n')
    toc_para.add_run('5. Basic Setup and General Guide\n')
    toc_para.add_run('6. Analytic Outputs\n')
    toc_para.add_run('7. Model Fidelity Log\n')
    toc_para.add_run('8. Sources and References\n')
    toc_para.add_run('9. Acronyms and Abbreviations\n')

    doc.add_page_break()

    # =========================================================================
    # SECTION 1: INTRODUCTION
    # =========================================================================
    doc.add_heading('1. Introduction', level=1)

    doc.add_heading('1.1 Purpose', level=2)
    doc.add_paragraph(
        'This User and Analyst Manual (UAM) provides comprehensive guidance for operating, '
        'modifying, and extending the T-150/TRV-150 Heavy-Lift UAS simulation. The document '
        'is intended for simulation analysts, developers, and researchers who need to:'
    )
    purpose_list = [
        'Run simulations with various mission profiles and configurations',
        'Build and install custom processor plugins',
        'Modify platform parameters and flight characteristics',
        'Integrate the simulation with other AFSIM scenarios',
        'Extend the model with new capabilities',
    ]
    for item in purpose_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('1.2 Scope', level=2)
    doc.add_paragraph(
        'This manual covers the complete T-150 UAS simulation including:'
    )
    scope_list = [
        'Platform definition with WSF_FULL_ROTORCRAFT_MOVER flight dynamics',
        'Battery processor with Li-Ion voltage sag model',
        'Propulsion processor with thermal, ground effect, and payload models',
        'VRS (Vortex Ring State) detection processor',
        'Slung load dynamics processor',
        'Scenario construction and mission planning',
        'Plugin development and build process',
    ]
    for item in scope_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('1.3 Prerequisites', level=2)
    doc.add_paragraph('Before using this simulation, ensure you have:')
    prereq_list = [
        'AFSIM 2.2505 or later installed at /opt/afsim2.2505/',
        'C++ compiler (GCC 9+ or compatible) for plugin development',
        'CMake 3.16+ for building plugins',
        'Basic familiarity with AFSIM scenario scripting',
        'Linux environment (WSL2 or native Linux)',
    ]
    for item in prereq_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_page_break()

    # =========================================================================
    # SECTION 2: THREAT SYSTEM DESCRIPTION
    # =========================================================================
    doc.add_heading('2. Threat System Description', level=1)

    doc.add_heading('2.1 Platform Overview', level=2)
    doc.add_paragraph(
        'The T-150/TRV-150 is a heavy-lift unmanned aerial system designed for logistics '
        'and cargo delivery missions. Key characteristics include:'
    )

    char_table = doc.add_table(rows=12, cols=2)
    char_table.style = 'Table Grid'

    char_table.rows[0].cells[0].text = 'Characteristic'
    char_table.rows[0].cells[1].text = 'Value'
    set_cell_shading(char_table.rows[0].cells[0], 'D9E2F3')
    set_cell_shading(char_table.rows[0].cells[1], 'D9E2F3')
    char_table.rows[0].cells[0].paragraphs[0].runs[0].bold = True
    char_table.rows[0].cells[1].paragraphs[0].runs[0].bold = True

    chars = [
        ('Configuration', 'Coaxial Octocopter (8 rotors, 4 arms)'),
        ('MTOW', '123 kg (271 lbs)'),
        ('Max Payload', '68 kg (150 lbs) internal, 55 kg slung'),
        ('Cruise Speed', '30 m/s (108 km/h)'),
        ('Max Range', '70 km'),
        ('Flight Time', '36 minutes (nominal)'),
        ('Service Ceiling', '3000 m'),
        ('Battery', '13S Li-Ion, 7 kWh capacity'),
        ('Motors', '8x T-Motor U15II KV100'),
        ('Propellers', '30" (0.76m) diameter'),
        ('Hover Power', '~18 kW'),
    ]
    for i, (char, val) in enumerate(chars):
        char_table.rows[i + 1].cells[0].text = char
        char_table.rows[i + 1].cells[1].text = val

    doc.add_heading('2.2 Concept of Operations (CONOPS)', level=2)
    doc.add_paragraph(
        'The T-150 is designed for autonomous cargo delivery missions with the following '
        'typical mission profile:'
    )

    mission_list = [
        'Takeoff: Vertical climb to cruise altitude (typically 100-500m AGL)',
        'Cruise: Point-to-point or waypoint-following flight at 30 m/s',
        'Station-keeping: Hover capability for precision operations',
        'Payload delivery: Land or slung load release',
        'Return: Autonomous return to launch point',
    ]
    for item in mission_list:
        doc.add_paragraph(item, style='List Number')

    doc.add_heading('2.3 Operational Limitations', level=2)

    add_callout_box(doc, 'CRITICAL FLIGHT ENVELOPE LIMITS',
        'VRS onset: Descent rate > 4 m/s with forward speed < 5 m/s\n'
        'Maximum bank angle: 45 deg (operational), 60 deg (demonstrated)\n'
        'Battery cutoff voltage: 39V (3.0V/cell) - mission abort required',
        'FFDDDD')

    limits_table = doc.add_table(rows=7, cols=3)
    limits_table.style = 'Table Grid'

    limits_table.rows[0].cells[0].text = 'Parameter'
    limits_table.rows[0].cells[1].text = 'Operational'
    limits_table.rows[0].cells[2].text = 'Do Not Exceed'
    for cell in limits_table.rows[0].cells:
        set_cell_shading(cell, 'FCE4D6')
        cell.paragraphs[0].runs[0].bold = True

    limits = [
        ('Max Airspeed', '30 m/s', '40 m/s'),
        ('Max Climb Rate', '5 m/s', '8 m/s'),
        ('Max Descent Rate', '3 m/s', '4 m/s (VRS limit)'),
        ('Max Bank Angle', '35 deg', '60 deg'),
        ('Motor Temperature', '100 C', '120 C'),
        ('Battery Voltage', '> 42V', '> 39V'),
    ]
    for i, row_data in enumerate(limits):
        for j, cell_data in enumerate(row_data):
            limits_table.rows[i + 1].cells[j].text = cell_data

    doc.add_page_break()

    # =========================================================================
    # SECTION 3: MODEL DESCRIPTION
    # =========================================================================
    doc.add_heading('3. Model Description', level=1)

    doc.add_heading('3.1 Model Overview', level=2)
    doc.add_paragraph(
        'The T-150 simulation consists of a platform definition using AFSIM\'s '
        'WSF_FULL_ROTORCRAFT_MOVER for flight dynamics, enhanced by four custom C++ '
        'processor plugins that model engineering subsystems.'
    )

    # Block diagram
    doc.add_heading('3.1.1 System Block Diagram', level=3)
    block_text = doc.add_paragraph()
    block_text.style = 'No Spacing'
    block_run = block_text.add_run(
'''
+-------------------------------------------------------------------+
|                    T-150 UAS Simulation Model                      |
+-------------------------------------------------------------------+
|                                                                    |
|  +---------------------+    +-----------------------------+        |
|  | Platform Definition |    | Custom Processors           |        |
|  | (t150_platform)     |    | (C++ Plugins)               |        |
|  +---------------------+    +-----------------------------+        |
|  |                     |    |                             |        |
|  | - T150_MOVER        |    | - T150_BATTERY_PROCESSOR    |        |
|  | - T150_RADAR_SIG    |    | - T150_PROPULSION_PROCESSOR |        |
|  |                     |    | - T150_VRS_PROCESSOR        |        |
|  +---------------------+    | - T150_SLUNG_LOAD_PROCESSOR |        |
|           |                 +-----------------------------+        |
|           v                              |                         |
|  +---------------------+                 |                         |
|  | WSF_FULL_ROTORCRAFT |<----------------+                         |
|  | _MOVER              |   (AuxData Communication)                 |
|  +---------------------+                                           |
|           |                                                        |
|           v                                                        |
|  +---------------------+                                           |
|  | Scenario Definition |                                           |
|  | (Route, Execute,    |                                           |
|  |  Event Recording)   |                                           |
|  +---------------------+                                           |
|                                                                    |
+-------------------------------------------------------------------+
'''
    )
    block_run.font.name = 'Courier New'
    block_run.font.size = Pt(8)

    doc.add_heading('3.2 Intended Use', level=2)
    doc.add_paragraph('This simulation is intended for:')
    use_list = [
        'Mission planning and route optimization studies',
        'Battery endurance and energy consumption analysis',
        'Flight envelope exploration and limitation studies',
        'Payload delivery mission analysis',
        'Slung load operations assessment',
        'VRS encounter scenario development',
        'Sensor/payload integration studies',
    ]
    for item in use_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('3.3 Key Performance Indicators (KPIs)', level=2)

    kpi_table = doc.add_table(rows=9, cols=3)
    kpi_table.style = 'Table Grid'

    kpi_table.rows[0].cells[0].text = 'KPI'
    kpi_table.rows[0].cells[1].text = 'AuxData Key'
    kpi_table.rows[0].cells[2].text = 'Description'
    for cell in kpi_table.rows[0].cells:
        set_cell_shading(cell, 'E2EFDA')
        cell.paragraphs[0].runs[0].bold = True

    kpis = [
        ('Battery SOC', 'BATTERY_SOC', 'State of charge (0-1)'),
        ('Terminal Voltage', 'BATTERY_VOLTAGE', 'Under-load voltage (V)'),
        ('Power Consumption', 'PROP_TOTAL_POWER', 'Total power draw (W)'),
        ('Motor Temperature', 'PROP_MAX_TEMP', 'Hottest motor temp (C)'),
        ('VRS Margin', 'VRS_MARGIN', 'VRS safety margin (0-1)'),
        ('Thrust Margin', 'PROP_THRUST_MARGIN', 'Available thrust (0-1)'),
        ('Ground Effect', 'PROP_GROUND_EFFECT', 'GE thrust factor'),
        ('Cable Tension', 'SLUNG_CABLE_TENSION', 'Slung load tension (N)'),
    ]
    for i, row_data in enumerate(kpis):
        for j, cell_data in enumerate(row_data):
            kpi_table.rows[i + 1].cells[j].text = cell_data

    doc.add_page_break()

    # =========================================================================
    # SECTION 4: THREAT MODEL COMPONENTS
    # =========================================================================
    doc.add_heading('4. Threat Model Components', level=1)

    doc.add_heading('4.1 Platform Definition Files', level=2)

    doc.add_heading('4.1.1 t150_mover.afsim', level=3)
    doc.add_paragraph(
        'Defines the WSF_FULL_ROTORCRAFT_MOVER with T-150 flight characteristics:'
    )

    mover_code = doc.add_paragraph()
    mover_code.style = 'No Spacing'
    mover_run = mover_code.add_run(
'''mover_type T150_MOVER WSF_FULL_ROTORCRAFT_MOVER
   rotorcraft_kinematics
      performance_limits
         available_airspeed   30 m/s     // Cruise speed
         maximum_rate_of_ascent 5 m/s    // Operational climb
         maximum_rate_of_descent 3 m/s   // Safe descent (VRS margin)
      end_performance_limits

      global_limits
         maximum_airspeed     40 m/s     // Never exceed
         maximum_rate_of_ascent 8 m/s    // Physical limit
         maximum_rate_of_descent 4 m/s   // VRS onset threshold
      end_global_limits

      acceleration_table               // Deceleration profile
         speed    0 m/s   deceleration 0.5 m/s^2
         speed   10 m/s   deceleration 2.5 m/s^2
         speed   30 m/s   deceleration 3.0 m/s^2
      end_acceleration_table
   end_rotorcraft_kinematics
end_mover_type'''
    )
    mover_run.font.name = 'Courier New'
    mover_run.font.size = Pt(9)

    doc.add_heading('4.1.2 t150_platform.afsim', level=3)
    doc.add_paragraph(
        'Defines the platform type combining mover and processors:'
    )

    platform_code = doc.add_paragraph()
    platform_code.style = 'No Spacing'
    platform_run = platform_code.add_run(
'''platform_type T150_UAS WSF_PLATFORM
   mover T150_MOVER

   processor BATTERY_MON T150_BATTERY_PROCESSOR
      battery_capacity_wh    7000.0
      initial_soc            1.0
      nominal_voltage        46.8
      internal_resistance    0.025
      use_dynamic_power      true
   end_processor

   processor PROP_MON T150_PROPULSION_PROCESSOR
      motor_count            8
      max_power_per_motor    3500
      empty_mass             68.0
      payload_mass           40.0
   end_processor

   processor VRS_MON T150_VRS_PROCESSOR
      vrs_descent_threshold  4.0
      vrs_speed_threshold    5.0
   end_processor
end_platform_type'''
    )
    platform_run.font.name = 'Courier New'
    platform_run.font.size = Pt(9)

    doc.add_heading('4.2 Plugin Source Code', level=2)
    doc.add_paragraph('The simulation includes four C++ processor plugins:')

    plugin_files = doc.add_table(rows=5, cols=2)
    plugin_files.style = 'Table Grid'

    plugin_files.rows[0].cells[0].text = 'Plugin Directory'
    plugin_files.rows[0].cells[1].text = 'Source Files'
    set_cell_shading(plugin_files.rows[0].cells[0], 'D9E2F3')
    set_cell_shading(plugin_files.rows[0].cells[1], 'D9E2F3')
    plugin_files.rows[0].cells[0].paragraphs[0].runs[0].bold = True
    plugin_files.rows[0].cells[1].paragraphs[0].runs[0].bold = True

    files = [
        ('plugins/t150_battery/', 'BatteryProcessor.cpp, BatteryProcessor.hpp'),
        ('plugins/t150_propulsion/', 'PropulsionProcessor.cpp, PropulsionProcessor.hpp'),
        ('plugins/t150_vrs/', 'VRSProcessor.cpp, VRSProcessor.hpp'),
        ('plugins/t150_slung_load/', 'SlungLoadProcessor.cpp, SlungLoadProcessor.hpp'),
    ]
    for i, (dir, srcs) in enumerate(files):
        plugin_files.rows[i + 1].cells[0].text = dir
        plugin_files.rows[i + 1].cells[1].text = srcs

    doc.add_page_break()

    # =========================================================================
    # SECTION 5: BASIC SETUP AND GENERAL GUIDE
    # =========================================================================
    doc.add_heading('5. Basic Setup and General Guide', level=1)

    doc.add_heading('5.1 Project Directory Structure', level=2)

    struct_text = doc.add_paragraph()
    struct_text.style = 'No Spacing'
    struct_run = struct_text.add_run(
'''project005_t150_uas/
├── README.md                    # Project overview
├── platforms/
│   ├── t150_mover.afsim         # Rotorcraft mover definition
│   └── t150_platform.afsim      # Platform type definition
├── scenarios/
│   └── t150_flight_demo.afsim   # Example mission scenario
├── plugins/
│   ├── CMakeLists.txt           # Top-level CMake
│   ├── build.sh                 # Build script
│   ├── install_plugins.sh       # Install to AFSIM
│   ├── t150_battery/            # Battery plugin source
│   ├── t150_propulsion/         # Propulsion plugin source
│   ├── t150_vrs/                # VRS plugin source
│   └── t150_slung_load/         # Slung load plugin source
├── output/                      # Simulation output files
├── tools/                       # Python analysis scripts
├── docs/                        # Documentation
└── VandV_docs/                  # V&V documentation
'''
    )
    struct_run.font.name = 'Courier New'
    struct_run.font.size = Pt(9)

    doc.add_heading('5.2 Building Plugins', level=2)

    add_callout_box(doc, 'IMPORTANT: Plugin Installation',
        'Plugins MUST be installed to the AFSIM system directory '
        '(/opt/afsim2.2505/lib/wsf_plugins/) to take effect. '
        'The local build directory is only used for compilation.',
        'FFF3CD')

    doc.add_paragraph('Step 1: Navigate to plugins directory and build')
    build_code = doc.add_paragraph()
    build_code.style = 'No Spacing'
    build_run = build_code.add_run(
'''cd /home/shepard/afsim_projects/project005_t150_uas/plugins
./build.sh

# Expected output:
# -- Configuring done
# -- Generating done
# -- Build files written to: .../plugins/build
# [100%] Built target libt150_battery.so
# [100%] Built target libt150_propulsion.so
# [100%] Built target libt150_vrs.so
# [100%] Built target libt150_slung_load.so'''
    )
    build_run.font.name = 'Courier New'
    build_run.font.size = Pt(9)

    doc.add_paragraph()
    doc.add_paragraph('Step 2: Install plugins (requires sudo)')
    install_code = doc.add_paragraph()
    install_code.style = 'No Spacing'
    install_run = install_code.add_run(
'''./install_plugins.sh

# This copies:
#   build/lib/libt150_battery.so -> /opt/afsim2.2505/lib/wsf_plugins/
#   build/lib/libt150_propulsion.so -> /opt/afsim2.2505/lib/wsf_plugins/
#   build/lib/libt150_vrs.so -> /opt/afsim2.2505/lib/wsf_plugins/
#   build/lib/libt150_slung_load.so -> /opt/afsim2.2505/lib/wsf_plugins/'''
    )
    install_run.font.name = 'Courier New'
    install_run.font.size = Pt(9)

    doc.add_heading('5.3 Running Simulations', level=2)

    doc.add_heading('5.3.1 Command Line Execution', level=3)
    run_code = doc.add_paragraph()
    run_code.style = 'No Spacing'
    run_run = run_code.add_run(
'''# Navigate to project directory
cd /home/shepard/afsim_projects/project005_t150_uas

# Run the simulation
/opt/afsim2.2505/bin/mission scenarios/t150_flight_demo.afsim

# Output files will be created in output/ directory'''
    )
    run_run.font.name = 'Courier New'
    run_run.font.size = Pt(9)

    doc.add_heading('5.3.2 Visualization with Mystic', level=3)
    mystic_code = doc.add_paragraph()
    mystic_code.style = 'No Spacing'
    mystic_run = mystic_code.add_run(
'''# After running simulation, view in Mystic
DISPLAY=:0 /opt/afsim2.2505/bin/mystic output/t150_flight_demo.aer

# For headless systems (WSL2), use X server like VcXsrv'''
    )
    mystic_run.font.name = 'Courier New'
    mystic_run.font.size = Pt(9)

    doc.add_heading('5.4 Creating Custom Scenarios', level=2)

    doc.add_paragraph('A minimal scenario requires:')

    scenario_code = doc.add_paragraph()
    scenario_code.style = 'No Spacing'
    scenario_run = scenario_code.add_run(
'''// my_scenario.afsim
include ../platforms/t150_platform.afsim

simulation
   time 0 sec
   time_step 0.1 sec
   end_time 600 sec
end_simulation

platform t150_drone T150_UAS
   route
      // Starting position
      position 52.9 deg -1.1 deg  altitude 0 m  speed 0 m/s

      // Climb to altitude
      position 52.9 deg -1.1 deg  altitude 100 m  speed 0 m/s

      // Fly to destination
      position 52.91 deg -1.1 deg  altitude 100 m  speed 30 m/s

      // Hover at destination
      position 52.91 deg -1.1 deg  altitude 100 m  speed 0 m/s

      // Return to start
      position 52.9 deg -1.1 deg  altitude 100 m  speed 30 m/s

      end_route loiter
   end_route
end_platform

// Enable event recording
event_pipe
   file output/my_scenario.aer
   platform_track_messages on
end_event_pipe'''
    )
    scenario_run.font.name = 'Courier New'
    scenario_run.font.size = Pt(9)

    doc.add_heading('5.5 Modifying Platform Parameters', level=2)

    doc.add_paragraph(
        'To customize the platform for different configurations, modify the processor '
        'parameters in your scenario or platform definition file:'
    )

    modify_code = doc.add_paragraph()
    modify_code.style = 'No Spacing'
    modify_run = modify_code.add_run(
'''// Example: Heavy payload configuration
platform_type T150_HEAVY_LOAD T150_UAS
   processor PROP_MON T150_PROPULSION_PROCESSOR
      motor_count            8
      max_power_per_motor    3500
      empty_mass             68.0
      payload_mass           55.0        // Maximum payload
      max_payload_mass       55.0
   end_processor

   processor BATTERY_MON T150_BATTERY_PROCESSOR
      battery_capacity_wh    7000.0
      hover_power_w          22000.0     // Higher for heavy load
      use_dynamic_power      true
   end_processor
end_platform_type

// Example: Slung load configuration
platform_type T150_SLUNG T150_UAS
   processor SLUNG_MON T150_SLUNG_LOAD_PROCESSOR
      load_mass              40.0        // 40 kg slung load
      cable_length           10.0        // 10 meter cable
      cable_damping          0.15
      max_cable_tension      6000.0      // Rated cable strength
   end_processor
end_platform_type'''
    )
    modify_run.font.name = 'Courier New'
    modify_run.font.size = Pt(9)

    doc.add_page_break()

    doc.add_heading('5.6 Plugin Development Guide', level=2)

    doc.add_heading('5.6.1 Plugin Structure', level=3)
    doc.add_paragraph(
        'Each plugin follows the AFSIM WsfProcessor pattern:'
    )

    plugin_struct = doc.add_paragraph()
    plugin_struct.style = 'No Spacing'
    plugin_struct_run = plugin_struct.add_run(
'''// MyProcessor.hpp
class MyProcessor : public WsfProcessor
{
public:
    MyProcessor(const WsfScenario& aScenario);
    static WsfStringId GetMyProcessorClassId();

    // Required overrides
    WsfProcessor* Clone() const override;
    bool ProcessInput(UtInput& aInput) override;
    bool Initialize(double aSimTime) override;
    void Update(double aSimTime) override;
    const char* GetScriptClassName() const override;

private:
    // Configuration parameters
    double mMyParameter;

    // Runtime state
    double mLastUpdateTime;
};'''
    )
    plugin_struct_run.font.name = 'Courier New'
    plugin_struct_run.font.size = Pt(9)

    doc.add_heading('5.6.2 Registering the Plugin', level=3)

    register_code = doc.add_paragraph()
    register_code.style = 'No Spacing'
    register_run = register_code.add_run(
'''// In MyProcessor.cpp
#include "WsfProcessorClassInfo.hpp"
#include "MyProcessor.hpp"

// Register plugin with AFSIM
WSF_PROCESSOR_REGISTER(MyProcessor, "MY_PROCESSOR",
    "Description of my processor");'''
    )
    register_run.font.name = 'Courier New'
    register_run.font.size = Pt(9)

    doc.add_heading('5.6.3 CMakeLists.txt Template', level=3)

    cmake_code = doc.add_paragraph()
    cmake_code.style = 'No Spacing'
    cmake_run = cmake_code.add_run(
'''cmake_minimum_required(VERSION 3.16)
project(my_plugin)

# Find AFSIM
set(AFSIM_ROOT "/opt/afsim2.2505")
include(${AFSIM_ROOT}/share/cmake/afsim.cmake)

# Add plugin library
afsim_add_plugin(my_plugin
    source/MyProcessor.cpp
    source/MyProcessor.hpp
)'''
    )
    cmake_run.font.name = 'Courier New'
    cmake_run.font.size = Pt(9)

    doc.add_heading('5.7 Accessing AuxData from Scripts', level=2)

    doc.add_paragraph(
        'Scenario scripts can read AuxData values published by processors:'
    )

    auxdata_code = doc.add_paragraph()
    auxdata_code.style = 'No Spacing'
    auxdata_run = auxdata_code.add_run(
'''// In scenario execute block
execute at_time 60 sec
   // Read battery state
   double soc = t150_drone.BATTERY_SOC
   double voltage = t150_drone.BATTERY_VOLTAGE

   // Check for warnings
   if (soc < 0.2)
      write_log "Low battery warning: " soc*100 "%"
   end_if

   // Read propulsion state
   double power = t150_drone.PROP_TOTAL_POWER
   double max_temp = t150_drone.PROP_MAX_TEMP

   // Read VRS state
   double vrs_margin = t150_drone.VRS_MARGIN
   if (vrs_margin < 0.5)
      write_log "VRS warning: margin = " vrs_margin
   end_if
end_execute'''
    )
    auxdata_run.font.name = 'Courier New'
    auxdata_run.font.size = Pt(9)

    doc.add_page_break()

    # =========================================================================
    # SECTION 6: ANALYTIC OUTPUTS
    # =========================================================================
    doc.add_heading('6. Analytic Outputs', level=1)

    doc.add_heading('6.1 Output Files', level=2)

    output_table = doc.add_table(rows=5, cols=3)
    output_table.style = 'Table Grid'

    output_table.rows[0].cells[0].text = 'File Type'
    output_table.rows[0].cells[1].text = 'Extension'
    output_table.rows[0].cells[2].text = 'Contents'
    for cell in output_table.rows[0].cells:
        set_cell_shading(cell, 'D9E2F3')
        cell.paragraphs[0].runs[0].bold = True

    outputs = [
        ('Event Record', '.aer', 'Platform tracks, events, for Mystic visualization'),
        ('Log File', '.log', 'Console output, warnings, processor messages'),
        ('Track Data', '.csv', 'Position/velocity data (if configured)'),
        ('Script Output', '.txt', 'Custom script output via write_file'),
    ]
    for i, row_data in enumerate(outputs):
        for j, cell_data in enumerate(row_data):
            output_table.rows[i + 1].cells[j].text = cell_data

    doc.add_heading('6.2 Available Telemetry Data', level=2)

    doc.add_paragraph(
        'The following data can be logged or monitored during simulation:'
    )

    doc.add_heading('6.2.1 Platform State', level=3)
    state_list = [
        'Position: Latitude, Longitude, Altitude (MSL/AGL)',
        'Velocity: Ground speed, Vertical speed, Heading',
        'Attitude: Roll, Pitch, Yaw (if available from mover)',
        'Route status: Current waypoint, distance to waypoint',
    ]
    for item in state_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('6.2.2 Subsystem State (via AuxData)', level=3)

    subsys_table = doc.add_table(rows=17, cols=2)
    subsys_table.style = 'Table Grid'

    subsys_table.rows[0].cells[0].text = 'Category'
    subsys_table.rows[0].cells[1].text = 'Available Parameters'
    set_cell_shading(subsys_table.rows[0].cells[0], 'E2EFDA')
    set_cell_shading(subsys_table.rows[0].cells[1], 'E2EFDA')
    subsys_table.rows[0].cells[0].paragraphs[0].runs[0].bold = True
    subsys_table.rows[0].cells[1].paragraphs[0].runs[0].bold = True

    subsys = [
        ('Battery', 'SOC, Voltage, OCV, Current, Power, Voltage Sag'),
        ('', 'Low Battery Flag, Voltage Low Flag'),
        ('Propulsion', 'Total Power, Total Thrust, Motor Count'),
        ('', 'Max/Avg Temperature, System Efficiency'),
        ('', 'Ground Effect Factor, Altitude AGL'),
        ('Payload', 'Total Mass, Payload Mass, Thrust Margin'),
        ('', 'CG Offset X/Y/Z, Overloaded Flag'),
        ('VRS', 'VRS State (0-4), VRS Margin'),
        ('', 'Descent Rate, Forward Speed'),
        ('', 'Lift Factor, Control Factor'),
        ('', 'Caution/Onset/Critical Flags'),
        ('Slung Load', 'Load State, Load Mass, Cable Length'),
        ('', 'Cable Tension, Swing X/Y/Magnitude'),
        ('', 'Force X/Y/Z, Position X/Y/Z'),
        ('', 'Natural Frequency, Resonance Flag'),
    ]
    for i, (cat, params) in enumerate(subsys):
        subsys_table.rows[i + 1].cells[0].text = cat
        subsys_table.rows[i + 1].cells[1].text = params

    doc.add_heading('6.3 Analysis Scripts', level=2)

    doc.add_paragraph(
        'Python analysis scripts are provided in the tools/ directory for post-processing:'
    )

    scripts_list = [
        'plot_battery.py - Battery SOC, voltage, current over time',
        'plot_thermal.py - Motor temperature profiles',
        'plot_trajectory.py - 3D flight path visualization',
        'analyze_energy.py - Energy consumption by flight phase',
    ]
    for item in scripts_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_page_break()

    # =========================================================================
    # SECTION 7: MODEL FIDELITY LOG
    # =========================================================================
    doc.add_heading('7. Model Fidelity Log', level=1)

    doc.add_heading('7.1 Fidelity Assessment', level=2)

    fidelity_table = doc.add_table(rows=8, cols=4)
    fidelity_table.style = 'Table Grid'

    fidelity_table.rows[0].cells[0].text = 'Component'
    fidelity_table.rows[0].cells[1].text = 'Fidelity'
    fidelity_table.rows[0].cells[2].text = 'Basis'
    fidelity_table.rows[0].cells[3].text = 'Limitations'
    for cell in fidelity_table.rows[0].cells:
        set_cell_shading(cell, 'D9E2F3')
        cell.paragraphs[0].runs[0].bold = True

    fidelity = [
        ('Flight Dynamics', 'Medium', 'Table-driven kinematic', 'Not 6-DOF physics'),
        ('Battery Model', 'Medium-High', 'Li-Ion polynomial OCV', 'No temperature effects'),
        ('Motor Thermal', 'Medium', 'First-order thermal', 'Simplified cooling model'),
        ('Ground Effect', 'Medium', 'Cheeseman-Bennett', 'Flat terrain assumed'),
        ('VRS Detection', 'Medium', 'Empirical thresholds', 'Simplified state machine'),
        ('Slung Load', 'Medium', 'Spherical pendulum', 'Small angle approx at high swing'),
        ('Payload/CG', 'Low-Medium', 'Static mass offset', 'No dynamic CG effects'),
    ]
    for i, row_data in enumerate(fidelity):
        for j, cell_data in enumerate(row_data):
            fidelity_table.rows[i + 1].cells[j].text = cell_data

    doc.add_heading('7.2 Known Limitations', level=2)

    limitations = [
        ('No Wind Model', 'Atmospheric wind effects not modeled; affects energy and trajectory'),
        ('Flat Terrain', 'Ground effect uses MSL altitude; no terrain database'),
        ('Static Payload', 'Payload mass fixed at scenario start; no dynamic loading'),
        ('No Sensor Noise', 'All measurements are perfect; no sensor error models'),
        ('Simplified Acoustics', 'No acoustic signature model'),
        ('Single Platform', 'No swarm or multi-vehicle coordination'),
    ]

    for title, desc in limitations:
        p = doc.add_paragraph()
        p.add_run(f'{title}: ').bold = True
        p.add_run(desc)

    doc.add_heading('7.3 Enhancement Roadmap', level=2)

    roadmap = [
        ('Phase 5', 'Pending', 'Wind effects integration'),
        ('Phase 6', 'Pending', 'BEMT rotor aerodynamics'),
        ('Phase 7', 'Pending', 'Sensor noise injection'),
        ('Phase 8', 'Pending', 'Multi-vehicle operations'),
    ]

    roadmap_table = doc.add_table(rows=5, cols=3)
    roadmap_table.style = 'Table Grid'

    roadmap_table.rows[0].cells[0].text = 'Phase'
    roadmap_table.rows[0].cells[1].text = 'Status'
    roadmap_table.rows[0].cells[2].text = 'Description'
    for cell in roadmap_table.rows[0].cells:
        set_cell_shading(cell, 'E2EFDA')
        cell.paragraphs[0].runs[0].bold = True

    for i, row_data in enumerate(roadmap):
        for j, cell_data in enumerate(row_data):
            roadmap_table.rows[i + 1].cells[j].text = cell_data

    doc.add_page_break()

    # =========================================================================
    # SECTION 8: SOURCES
    # =========================================================================
    doc.add_heading('8. Sources and References', level=1)

    sources = [
        ('AAIB Accident Investigation', 'Air Accidents Investigation Branch report on TRV-150 incident at South Scarle, Nottinghamshire, UK, 2023'),
        ('US Patent US20240262499A1', 'Battery topology and power management for heavy-lift UAS'),
        ('FAA Aircraft Registry', 'TRV-150 registration and configuration data'),
        ('T-Motor U15II Specifications', 'Motor performance data: KV100, 3.5kW max power'),
        ('AFSIM Documentation', 'AFSIM 2.2505 framework documentation'),
        ('WSF_FULL_ROTORCRAFT_MOVER', 'AFSIM rotorcraft mover documentation'),
        ('Cheeseman & Bennett (1955)', 'Ground effect approximation'),
        ('Helicopter VRS Theory', 'Vortex Ring State conditions and recovery'),
    ]

    for i, (title, desc) in enumerate(sources, 1):
        p = doc.add_paragraph()
        p.add_run(f'[{i}] {title}: ').bold = True
        p.add_run(desc)

    doc.add_page_break()

    # =========================================================================
    # SECTION 9: ACRONYMS
    # =========================================================================
    doc.add_heading('9. Acronyms and Abbreviations', level=1)

    acronym_table = doc.add_table(rows=26, cols=2)
    acronym_table.style = 'Table Grid'

    acronym_table.rows[0].cells[0].text = 'Acronym'
    acronym_table.rows[0].cells[1].text = 'Definition'
    set_cell_shading(acronym_table.rows[0].cells[0], 'D9E2F3')
    set_cell_shading(acronym_table.rows[0].cells[1], 'D9E2F3')
    acronym_table.rows[0].cells[0].paragraphs[0].runs[0].bold = True
    acronym_table.rows[0].cells[1].paragraphs[0].runs[0].bold = True

    acronyms = [
        ('AFSIM', 'Advanced Framework for Simulation, Integration, and Modeling'),
        ('AGL', 'Above Ground Level'),
        ('BEMT', 'Blade Element Momentum Theory'),
        ('CG', 'Center of Gravity'),
        ('CONOPS', 'Concept of Operations'),
        ('DOF', 'Degrees of Freedom'),
        ('ESC', 'Electronic Speed Controller'),
        ('GE', 'Ground Effect'),
        ('IGE', 'In Ground Effect'),
        ('KPI', 'Key Performance Indicator'),
        ('KV', 'Motor velocity constant (RPM per Volt)'),
        ('MSL', 'Mean Sea Level'),
        ('MTOW', 'Maximum Takeoff Weight'),
        ('NED', 'North-East-Down (coordinate frame)'),
        ('OCV', 'Open-Circuit Voltage'),
        ('OGE', 'Out of Ground Effect'),
        ('PID', 'Proportional-Integral-Derivative'),
        ('RPM', 'Revolutions Per Minute'),
        ('SOC', 'State of Charge'),
        ('UAM', 'User and Analyst Manual'),
        ('UAS', 'Unmanned Aerial System'),
        ('V&V', 'Verification and Validation'),
        ('VRS', 'Vortex Ring State'),
        ('WSF', 'Warfighter Simulation Framework'),
        ('Wh', 'Watt-hour'),
    ]

    for i, (acr, defn) in enumerate(acronyms):
        acronym_table.rows[i + 1].cells[0].text = acr
        acronym_table.rows[i + 1].cells[1].text = defn

    # Save document
    output_path = '/home/shepard/afsim_projects/project005_t150_uas/VandV_docs/T150_UAS_UAM_v1.0.docx'
    doc.save(output_path)
    print(f'UAM document created: {output_path}')
    return output_path

if __name__ == '__main__':
    create_uam_document()
