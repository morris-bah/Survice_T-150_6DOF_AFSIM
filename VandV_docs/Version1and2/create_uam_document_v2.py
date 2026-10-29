#!/usr/bin/env python3
"""
T-150 UAS User and Analyst Manual (UAM) Generator - Version 2.0
Comprehensive user guide with quick start, configuration reference, troubleshooting, and adaptation examples.
Following NGIC/TMAP V&V template format.
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


def add_code_block(doc, code_text, font_size=9):
    """Add a monospace code block"""
    para = doc.add_paragraph()
    para.style = 'No Spacing'
    run = para.add_run(code_text)
    run.font.name = 'Courier New'
    run.font.size = Pt(font_size)
    return para


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


def add_table_with_header(doc, headers, data, header_color='D9E2F3'):
    """Create a table with styled header row"""
    table = doc.add_table(rows=len(data) + 1, cols=len(headers))
    table.style = 'Table Grid'

    # Header row
    for i, header in enumerate(headers):
        table.rows[0].cells[i].text = header
        set_cell_shading(table.rows[0].cells[i], header_color)
        if table.rows[0].cells[i].paragraphs[0].runs:
            table.rows[0].cells[i].paragraphs[0].runs[0].bold = True

    # Data rows
    for row_idx, row_data in enumerate(data):
        for col_idx, cell_data in enumerate(row_data):
            table.rows[row_idx + 1].cells[col_idx].text = str(cell_data)

    return table


def create_uam_document():
    doc = Document()

    # Set document margins
    for section in doc.sections:
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
    version_para.add_run(f"\n\nVersion 2.0\n{datetime.date.today().strftime('%d %B %Y')}\n\n")

    class_para = doc.add_paragraph()
    class_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    class_run = class_para.add_run("UNCLASSIFIED // FOR OFFICIAL USE ONLY")
    class_run.bold = True

    doc.add_page_break()

    # =========================================================================
    # REVISION HISTORY
    # =========================================================================
    doc.add_heading('Revision History', level=1)

    rev_data = [
        ['2.0', datetime.date.today().strftime('%Y-%m-%d'), 'Project Team', 'Major update: Quick start, configuration reference, troubleshooting'],
        ['1.0', '2025-12-19', 'Project Team', 'Initial release'],
        ['0.9', '2025-12-01', 'Project Team', 'Draft for review'],
    ]
    add_table_with_header(doc, ['Version', 'Date', 'Author', 'Description'], rev_data)

    doc.add_page_break()

    # =========================================================================
    # TABLE OF CONTENTS
    # =========================================================================
    doc.add_heading('Table of Contents', level=1)
    toc_content = '''(Auto-generate in Word: References > Table of Contents)

1. Introduction
   1.1 Purpose
   1.2 Scope
   1.3 Prerequisites
2. Quick Start Guide
   2.1 Installation (5 Minutes)
   2.2 Running Your First Simulation
   2.3 Verifying Results
3. Threat System Description
   3.1 Platform Overview
   3.2 Operational Characteristics
   3.3 Flight Envelope Limits
4. Complete Configuration Reference
   4.1 Rotorcraft6DOF_Mover Parameters
   4.2 Battery Processor Parameters
   4.3 Propulsion Processor Parameters
   4.4 VRS Processor Parameters
   4.5 Slung Load Processor Parameters
5. Scenario Development Guide
   5.1 Scenario Structure
   5.2 Route Definition
   5.3 Event Recording
   5.4 Script-Based Control
6. AuxData Reference
   6.1 6DOF Mover Outputs
   6.2 Battery Processor Outputs
   6.3 Propulsion Processor Outputs
   6.4 VRS Processor Outputs
   6.5 Slung Load Processor Outputs
7. Script Examples
   7.1 Battery Monitoring
   7.2 VRS Avoidance
   7.3 Data Logging
8. Troubleshooting
   8.1 Plugin Loading Issues
   8.2 Simulation Behavior Problems
   8.3 Common Errors
9. Adapting for Other Aircraft
   9.1 Quadcopter Example
   9.2 Hexacopter Example
   9.3 Controller Tuning Guide
10. Sources and References
11. Acronyms and Abbreviations
'''
    toc_para = doc.add_paragraph()
    toc_para.add_run(toc_content)
    doc.add_page_break()

    # =========================================================================
    # SECTION 1: INTRODUCTION
    # =========================================================================
    doc.add_heading('1. Introduction', level=1)

    doc.add_heading('1.1 Purpose', level=2)
    doc.add_paragraph(
        'This User and Analyst Manual (UAM) provides comprehensive guidance for operating, '
        'modifying, and extending the T-150/TRV-150 Heavy-Lift UAS simulation. Whether you '
        'are running your first simulation or adapting the plugins for a custom aircraft, '
        'this document will guide you through the process.'
    )

    doc.add_heading('1.2 Scope', level=2)
    doc.add_paragraph('This manual covers:')
    scope_list = [
        'Quick start installation and first simulation',
        'Complete configuration reference for all 5 plugins',
        'Scenario development with routes, events, and scripts',
        'AuxData interface for monitoring and control',
        'Troubleshooting common issues',
        'Adapting plugins for other rotorcraft',
    ]
    for item in scope_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('1.3 Prerequisites', level=2)
    prereqs = [
        ('AFSIM', '2.2505.x LTS', 'Core simulation framework'),
        ('CMake', '3.16+', 'Build system (for plugin development)'),
        ('GCC/G++', '11+', 'Compiler (for plugin development)'),
        ('Linux', 'AlmaLinux 9 / Ubuntu 22.04+', 'Operating system'),
    ]
    add_table_with_header(doc, ['Requirement', 'Version', 'Notes'], prereqs)

    doc.add_page_break()

    # =========================================================================
    # SECTION 2: QUICK START GUIDE
    # =========================================================================
    doc.add_heading('2. Quick Start Guide', level=1)

    add_callout_box(doc, 'TIME TO FIRST SIMULATION: ~5 MINUTES',
        'Follow these steps to get the T-150 simulation running quickly.',
        'E2EFDA')

    doc.add_heading('2.1 Installation (5 Minutes)', level=2)

    doc.add_heading('Step 1: Clone the Repository', level=3)
    clone_code = '''git clone https://github.com/DSGarrett/project005_t150_uas.git
cd project005_t150_uas'''
    add_code_block(doc, clone_code)

    doc.add_heading('Step 2: Build Plugins', level=3)
    build_code = '''./ngic_afsim_build.sh

# Expected output:
# -- Configuring done
# -- Generating done
# [100%] Built target rotorcraft_6dof
# [100%] Built target t150_battery_processor
# ...'''
    add_code_block(doc, build_code)

    doc.add_heading('Step 3: Install Plugins', level=3)
    install_code = '''./ngic_afsim_install.sh

# Copies plugins to /opt/afsim2.2505/lib/wsf_plugins/'''
    add_code_block(doc, install_code)

    doc.add_heading('2.2 Running Your First Simulation', level=2)
    run_code = '''# Run the flight demo scenario
/opt/afsim2.2505/bin/mission scenarios/t150_flight_demo.afsim

# Output will be saved to output/ directory'''
    add_code_block(doc, run_code)

    doc.add_heading('2.3 Verifying Results', level=2)
    verify_code = '''# View in Mystic visualization tool
DISPLAY=:0 /opt/afsim2.2505/bin/mystic output/t150_flight_demo.aer

# Or check the CSV telemetry
head output/t150_flight_demo.csv'''
    add_code_block(doc, verify_code)

    add_callout_box(doc, 'SUCCESS INDICATORS',
        '- Simulation runs without errors\n'
        '- .aer file created for visualization\n'
        '- Battery SOC decreases over time\n'
        '- Aircraft follows waypoints',
        'E7F3FF')

    doc.add_page_break()

    # =========================================================================
    # SECTION 3: THREAT SYSTEM DESCRIPTION
    # =========================================================================
    doc.add_heading('3. Threat System Description', level=1)

    doc.add_heading('3.1 Platform Overview', level=2)
    doc.add_paragraph(
        'The T-150/TRV-150 is a heavy-lift unmanned aerial system manufactured by '
        'Malloy Aeronautics. It features a coaxial octocopter configuration optimized '
        'for cargo delivery missions.'
    )

    specs = [
        ('Configuration', 'Coaxial Octocopter (8 rotors, 4 arms)'),
        ('MTOW', '123 kg (271 lbs)'),
        ('Max Payload', '68 kg (150 lbs) internal, 55 kg slung'),
        ('Cruise Speed', '30 m/s (108 km/h)'),
        ('Max Range', '70 km'),
        ('Flight Time', '36 minutes (nominal)'),
        ('Battery', '13S Li-Ion, 7 kWh capacity'),
        ('Motors', '8x T-Motor U15II KV100'),
        ('Propellers', '30" (0.76m) diameter'),
    ]
    add_table_with_header(doc, ['Characteristic', 'Value'], specs)

    doc.add_heading('3.2 Operational Characteristics', level=2)
    doc.add_paragraph('Typical mission profile:')
    mission_steps = [
        'Takeoff: Vertical climb to cruise altitude (100-500m AGL)',
        'Cruise: Waypoint-following at 30 m/s',
        'Station-keeping: Hover for precision operations',
        'Payload delivery: Land or slung load release',
        'Return: Autonomous return to base',
    ]
    for i, step in enumerate(mission_steps, 1):
        doc.add_paragraph(f'{i}. {step}')

    doc.add_heading('3.3 Flight Envelope Limits', level=2)

    add_callout_box(doc, 'CRITICAL FLIGHT ENVELOPE LIMITS',
        'VRS onset: Descent rate > 4 m/s with forward speed < 5 m/s\n'
        'Maximum bank angle: 45 deg (operational), 60 deg (limit)\n'
        'Battery cutoff: 39V (3.0V/cell) - mission abort required',
        'FFDDDD')

    limits = [
        ('Max Airspeed', '30 m/s', '40 m/s'),
        ('Max Climb Rate', '5 m/s', '8 m/s'),
        ('Max Descent Rate', '3 m/s', '4 m/s (VRS)'),
        ('Max Bank Angle', '35 deg', '60 deg'),
        ('Motor Temperature', '100 C', '120 C'),
        ('Battery Voltage', '> 42V', '> 39V'),
    ]
    add_table_with_header(doc, ['Parameter', 'Operational', 'Do Not Exceed'], limits, 'FCE4D6')

    doc.add_page_break()

    # =========================================================================
    # SECTION 4: COMPLETE CONFIGURATION REFERENCE
    # =========================================================================
    doc.add_heading('4. Complete Configuration Reference', level=1)

    doc.add_heading('4.1 Rotorcraft6DOF_Mover Parameters', level=2)

    doc.add_heading('4.1.1 Physical Properties', level=3)
    phys_params = [
        ('mass', '68.0', 'kg', 'Total aircraft mass including payload'),
        ('inertia_xx', '2.5', 'kg*m^2', 'Roll moment of inertia'),
        ('inertia_yy', '2.5', 'kg*m^2', 'Pitch moment of inertia'),
        ('inertia_zz', '4.0', 'kg*m^2', 'Yaw moment of inertia'),
        ('rotor_count', '8', '-', 'Number of rotors'),
        ('rotor_diameter', '0.76', 'm', 'Propeller diameter'),
        ('arm_length', '0.5', 'm', 'Motor arm length from CG'),
    ]
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], phys_params, 'E2EFDA')

    doc.add_heading('4.1.2 Controller Gains', level=3)
    ctrl_params = [
        ('altitude_kp', '0.3', '-', 'Altitude proportional gain'),
        ('altitude_ki', '0.05', '-', 'Altitude integral gain'),
        ('altitude_kd', '1.2', '-', 'Altitude derivative gain'),
        ('position_kp', '1.0', '-', 'Position proportional gain'),
        ('position_kd', '0.1', '-', 'Position derivative gain'),
        ('velocity_kp', '3.0', '-', 'Velocity proportional gain'),
        ('attitude_kp', '8.0', '-', 'Attitude proportional gain'),
        ('attitude_kd', '2.0', '-', 'Attitude derivative gain'),
        ('max_tilt_angle', '25.0', 'deg', 'Maximum tilt angle'),
    ]
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], ctrl_params, 'E2EFDA')

    doc.add_heading('4.2 Battery Processor Parameters', level=2)
    batt_params = [
        ('battery_capacity_wh', '7000.0', 'Wh', 'Total battery capacity'),
        ('initial_soc', '1.0', '0-1', 'Initial state of charge'),
        ('nominal_voltage', '46.8', 'V', '13S x 3.6V nominal'),
        ('full_charge_voltage', '54.6', 'V', '13S x 4.2V full'),
        ('cutoff_voltage', '39.0', 'V', '13S x 3.0V cutoff'),
        ('internal_resistance', '0.025', 'Ohms', 'Pack internal resistance'),
        ('motor_efficiency', '0.85', '0-1', 'Motor/ESC efficiency'),
        ('auxiliary_power_w', '4000.0', 'W', 'Avionics, cooling, etc.'),
        ('hover_power_w', '18000.0', 'W', 'Fallback hover power'),
        ('low_battery_warning', '0.20', '0-1', 'SOC threshold for warning'),
        ('use_dynamic_power', 'true', 'bool', 'Read power from 6DOF'),
        ('power_source_key', '"6dof_total_power"', 'string', 'AuxData key for power'),
    ]
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], batt_params, 'E2EFDA')

    doc.add_heading('4.3 Propulsion Processor Parameters', level=2)
    prop_params = [
        ('motor_count', '8', '-', 'Number of motors'),
        ('kv_rating', '100.0', 'RPM/V', 'Motor KV rating'),
        ('max_power_per_motor', '3500.0', 'W', 'Max power per motor'),
        ('propeller_diameter', '0.76', 'm', 'Propeller diameter'),
        ('coaxial_efficiency', '0.85', '0-1', 'Coaxial interference factor'),
        ('motor_thermal_limit', '120.0', 'C', 'Thermal limit'),
        ('motor_thermal_mass', '1500.0', 'J/K', 'Motor thermal mass'),
        ('motor_cooling_rate', '20.0', 'W/K', 'Cooling rate'),
        ('motor_time_constant', '0.15', 's', 'RPM response time'),
        ('ground_effect_enabled', 'true', 'bool', 'Enable ground effect'),
        ('empty_mass', '68.0', 'kg', 'Empty aircraft mass'),
        ('payload_mass', '0.0', 'kg', 'Current payload'),
        ('max_payload_mass', '55.0', 'kg', 'Maximum payload'),
    ]
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], prop_params, 'E2EFDA')

    doc.add_page_break()

    doc.add_heading('4.4 VRS Processor Parameters', level=2)
    vrs_params = [
        ('vrs_descent_threshold', '4.0', 'm/s', 'Descent rate for VRS'),
        ('vrs_speed_threshold', '5.0', 'm/s', 'Speed to exit VRS'),
        ('caution_margin', '0.8', '0-1', 'Caution state threshold'),
        ('onset_margin', '0.5', '0-1', 'Onset state threshold'),
        ('developed_margin', '0.2', '0-1', 'Developed VRS threshold'),
        ('induced_velocity', '8.0', 'm/s', 'Hover induced velocity'),
        ('coaxial_factor', '0.85', '0-1', 'Coaxial VRS resistance'),
        ('vrs_development_time', '2.0', 's', 'Time to develop VRS'),
        ('vrs_recovery_time', '3.0', 's', 'Recovery time'),
        ('max_lift_degradation', '0.6', '0-1', 'Max lift loss in VRS'),
    ]
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], vrs_params, 'E2EFDA')

    doc.add_heading('4.5 Slung Load Processor Parameters', level=2)
    slung_params = [
        ('load_mass', '0.0', 'kg', 'Suspended load mass'),
        ('cable_length', '5.0', 'm', 'Cable length'),
        ('cable_damping', '0.1', '1/s', 'Damping coefficient'),
        ('attach_point_z', '0.5', 'm', 'Attach point below CG'),
        ('max_cable_tension', '5000.0', 'N', 'Max cable tension'),
        ('resonance_threshold', '20.0', 'deg', 'Swing angle for warning'),
        ('update_interval', '0.02', 's', '50 Hz dynamics update'),
        ('initial_swing_x', '0.0', 'deg', 'Initial swing angle X'),
        ('initial_swing_y', '0.0', 'deg', 'Initial swing angle Y'),
    ]
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], slung_params, 'E2EFDA')

    doc.add_page_break()

    # =========================================================================
    # SECTION 5: SCENARIO DEVELOPMENT GUIDE
    # =========================================================================
    doc.add_heading('5. Scenario Development Guide', level=1)

    doc.add_heading('5.1 Scenario Structure', level=2)
    doc.add_paragraph('A complete scenario includes these elements:')

    scenario_code = '''// my_mission.afsim

// Include platform definitions
include ../platforms/t150_platform.afsim

// Simulation parameters
simulation
   time 0 sec
   time_step 0.1 sec
   end_time 600 sec
end_simulation

// Platform instance with route
platform t150_drone T150_UAS
   route
      // waypoints here
   end_route
end_platform

// Event recording for visualization
event_pipe
   file output/my_mission.aer
   platform_track_messages on
end_event_pipe'''
    add_code_block(doc, scenario_code)

    doc.add_heading('5.2 Route Definition', level=2)
    doc.add_paragraph('Routes define waypoints with position, altitude, and speed:')

    route_code = '''route
   // Starting position (on ground)
   position 52.9 deg -1.1 deg  altitude 0 m  speed 0 m/s

   // Climb to cruise altitude
   position 52.9 deg -1.1 deg  altitude 100 m  speed 0 m/s

   // Cruise to destination
   position 52.91 deg -1.1 deg  altitude 100 m  speed 30 m/s

   // Hover at destination
   position 52.91 deg -1.1 deg  altitude 100 m  speed 0 m/s

   // Return to start
   position 52.9 deg -1.1 deg  altitude 100 m  speed 30 m/s

   // Land
   position 52.9 deg -1.1 deg  altitude 0 m  speed 0 m/s

   end_route loiter
end_route'''
    add_code_block(doc, route_code)

    doc.add_heading('5.3 Event Recording', level=2)
    event_code = '''// Full event recording
event_pipe
   file output/my_mission.aer
   platform_track_messages on
   all_messages on
end_event_pipe

// CSV telemetry export
csv_logger
   file output/my_mission.csv
   interval 1.0 sec
   columns time, latitude, longitude, altitude, speed
end_csv_logger'''
    add_code_block(doc, event_code)

    doc.add_heading('5.4 Script-Based Control', level=2)
    script_code = '''// Periodic monitoring script
execute at_interval 10 sec
   double soc = t150_drone.BATTERY_SOC
   double vrs = t150_drone.VRS_MARGIN

   // Log warnings
   if (soc < 0.2)
      write_log "LOW BATTERY: " soc*100 "%"
   end_if

   if (vrs < 0.5)
      write_log "VRS WARNING: margin = " vrs
   end_if
end_execute

// Mission abort trigger
execute when (t150_drone.BATTERY_SOC < 0.15)
   write_log "MISSION ABORT - Critical battery"
   t150_drone.SetRoute(return_to_base_route)
end_execute'''
    add_code_block(doc, script_code)

    doc.add_page_break()

    # =========================================================================
    # SECTION 6: AUXDATA REFERENCE
    # =========================================================================
    doc.add_heading('6. AuxData Reference', level=1)

    doc.add_paragraph(
        'AuxData is the inter-processor communication mechanism. All values can be '
        'read from scenario scripts and some can be written to modify behavior.'
    )

    doc.add_heading('6.1 6DOF Mover Outputs', level=2)
    mover_aux = [
        ('6dof_pos_n', 'double', 'm', 'North position'),
        ('6dof_pos_e', 'double', 'm', 'East position'),
        ('6dof_pos_d', 'double', 'm', 'Down position'),
        ('6dof_vel_n', 'double', 'm/s', 'North velocity'),
        ('6dof_vel_e', 'double', 'm/s', 'East velocity'),
        ('6dof_vel_d', 'double', 'm/s', 'Down velocity'),
        ('6dof_roll_deg', 'double', 'deg', 'Roll angle'),
        ('6dof_pitch_deg', 'double', 'deg', 'Pitch angle'),
        ('6dof_yaw_deg', 'double', 'deg', 'Yaw angle'),
        ('6dof_total_thrust', 'double', 'N', 'Total thrust'),
        ('6dof_total_power', 'double', 'W', 'Mechanical power'),
        ('6dof_throttle_cmd', 'double', '0-1', 'Throttle command'),
    ]
    add_table_with_header(doc, ['Key', 'Type', 'Units', 'Description'], mover_aux)

    doc.add_heading('6.2 Battery Processor Outputs', level=2)
    batt_aux = [
        ('BATTERY_SOC', 'double', '0-1', 'State of charge'),
        ('BATTERY_POWER_W', 'double', 'W', 'Power consumption'),
        ('BATTERY_ENERGY_WH', 'double', 'Wh', 'Remaining energy'),
        ('BATTERY_VOLTAGE', 'double', 'V', 'Terminal voltage'),
        ('BATTERY_OCV', 'double', 'V', 'Open-circuit voltage'),
        ('BATTERY_VOLTAGE_SAG', 'double', 'V', 'Voltage drop'),
        ('BATTERY_CURRENT_A', 'double', 'A', 'Current draw'),
        ('BATTERY_LOW', 'bool', '-', 'Low battery flag'),
        ('BATTERY_VOLTAGE_LOW', 'bool', '-', 'Voltage sag flag'),
    ]
    add_table_with_header(doc, ['Key', 'Type', 'Units', 'Description'], batt_aux)

    doc.add_heading('6.3 Propulsion Processor Outputs', level=2)
    prop_aux = [
        ('PROP_TOTAL_POWER', 'double', 'W', 'Electrical power'),
        ('PROP_TOTAL_THRUST', 'double', 'N', 'Combined thrust'),
        ('PROP_MAX_TEMP', 'double', 'C', 'Hottest motor'),
        ('PROP_AVG_TEMP', 'double', 'C', 'Average temperature'),
        ('PROP_GROUND_EFFECT', 'double', '1.0+', 'GE factor'),
        ('PROP_ALTITUDE_AGL', 'double', 'm', 'Altitude AGL'),
        ('PROP_TOTAL_MASS', 'double', 'kg', 'Total mass'),
        ('PROP_OVERLOADED', 'bool', '-', 'Overload flag'),
    ]
    add_table_with_header(doc, ['Key', 'Type', 'Units', 'Description'], prop_aux)

    doc.add_heading('6.4 VRS Processor Outputs', level=2)
    vrs_aux = [
        ('VRS_STATE', 'int', '0-4', 'VRS state enum'),
        ('VRS_MARGIN', 'double', '0-1', 'Safety margin'),
        ('VRS_DESCENT_RATE', 'double', 'm/s', 'Descent rate'),
        ('VRS_LIFT_FACTOR', 'double', '0-1', 'Lift degradation'),
        ('VRS_CONTROL_FACTOR', 'double', '0-1', 'Control authority'),
        ('VRS_CAUTION', 'bool', '-', 'Caution flag'),
        ('VRS_CRITICAL', 'bool', '-', 'Critical flag'),
    ]
    add_table_with_header(doc, ['Key', 'Type', 'Units', 'Description'], vrs_aux)

    doc.add_heading('6.5 Slung Load Processor Outputs', level=2)
    slung_aux = [
        ('SLUNG_ANGLE_X', 'double', 'deg', 'Swing angle X'),
        ('SLUNG_ANGLE_Y', 'double', 'deg', 'Swing angle Y'),
        ('SLUNG_TENSION', 'double', 'N', 'Cable tension'),
        ('SLUNG_LOAD_MASS', 'double', 'kg', 'Load mass'),
        ('SLUNG_RESONANCE', 'bool', '-', 'Resonance flag'),
        ('SLUNG_OVERLOAD', 'bool', '-', 'Overload flag'),
    ]
    add_table_with_header(doc, ['Key', 'Type', 'Units', 'Description'], slung_aux)

    doc.add_page_break()

    # =========================================================================
    # SECTION 7: SCRIPT EXAMPLES
    # =========================================================================
    doc.add_heading('7. Script Examples', level=1)

    doc.add_heading('7.1 Battery Monitoring Script', level=2)
    batt_script = '''// battery_monitor.afsim
// Monitor battery and trigger warnings

execute at_interval 5 sec
   double soc = t150_drone.BATTERY_SOC
   double voltage = t150_drone.BATTERY_VOLTAGE
   double current = t150_drone.BATTERY_CURRENT_A
   double power = t150_drone.BATTERY_POWER_W

   // Calculate remaining flight time
   double energy_remaining = t150_drone.BATTERY_ENERGY_WH
   double time_remaining_min = (energy_remaining / power) * 60

   write_log "Battery: " soc*100 "% | " voltage "V | " current "A"
   write_log "Est. remaining: " time_remaining_min " minutes"

   // Warning levels
   if (soc < 0.10)
      write_log "*** CRITICAL BATTERY - LAND IMMEDIATELY ***"
   else if (soc < 0.20)
      write_log "** LOW BATTERY WARNING **"
   end_if
end_execute'''
    add_code_block(doc, batt_script, 8)

    doc.add_heading('7.2 VRS Avoidance Script', level=2)
    vrs_script = '''// vrs_avoidance.afsim
// Monitor VRS and adjust flight path

execute at_interval 1 sec
   double margin = t150_drone.VRS_MARGIN
   double descent = t150_drone.VRS_DESCENT_RATE
   int state = t150_drone.VRS_STATE

   // VRS states: 0=CLEAR, 1=CAUTION, 2=ONSET, 3=DEVELOPED, 4=RECOVERY
   if (state >= 2)
      write_log "VRS ONSET - Increasing forward speed"
      // Recovery: push nose down to gain airspeed
      // (Would require route modification in practice)
   else if (state == 1)
      write_log "VRS CAUTION - margin: " margin
   end_if
end_execute'''
    add_code_block(doc, vrs_script, 8)

    doc.add_heading('7.3 Data Logging Script', level=2)
    log_script = '''// data_logger.afsim
// Log comprehensive telemetry to file

execute at_interval 1 sec
   // Get all telemetry
   double time = sim_time()
   double lat = t150_drone.latitude
   double lon = t150_drone.longitude
   double alt = t150_drone.altitude
   double soc = t150_drone.BATTERY_SOC
   double power = t150_drone.BATTERY_POWER_W
   double vrs = t150_drone.VRS_MARGIN

   // Write to CSV file
   write_file "output/telemetry.csv" append
      time "," lat "," lon "," alt "," soc "," power "," vrs
   end_write_file
end_execute'''
    add_code_block(doc, log_script, 8)

    doc.add_page_break()

    # =========================================================================
    # SECTION 8: TROUBLESHOOTING
    # =========================================================================
    doc.add_heading('8. Troubleshooting', level=1)

    doc.add_heading('8.1 Plugin Loading Issues', level=2)

    doc.add_heading('Problem: "Unknown processor type" error', level=3)
    doc.add_paragraph('Cause: Plugin not installed to AFSIM directory')
    solve1 = '''# Verify plugin location
ls /opt/afsim2.2505/lib/wsf_plugins/t150*.so

# If missing, install:
./ngic_afsim_install.sh'''
    add_code_block(doc, solve1)

    doc.add_heading('Problem: Plugin loads but parameters not recognized', level=3)
    doc.add_paragraph('Cause: Mismatch between plugin version and scenario')
    solve2 = '''# Rebuild and reinstall plugins
./ngic_afsim_build.sh clean
./ngic_afsim_build.sh
./ngic_afsim_install.sh'''
    add_code_block(doc, solve2)

    doc.add_heading('8.2 Simulation Behavior Problems', level=2)

    doc.add_heading('Problem: Aircraft does not move', level=3)
    doc.add_paragraph('Check: Route definition and initial position')
    check1 = '''# Ensure route has waypoints with different positions
route
   position 52.9 deg -1.1 deg  altitude 0 m    speed 0 m/s
   position 52.9 deg -1.1 deg  altitude 100 m  speed 0 m/s  // MUST DIFFER
end_route'''
    add_code_block(doc, check1)

    doc.add_heading('Problem: Battery depletes too fast', level=3)
    doc.add_paragraph('Check: Motor efficiency and auxiliary power settings')
    check2 = '''processor BATTERY T150_BATTERY_PROCESSOR
   motor_efficiency 0.85      // Should be 0.80-0.90
   auxiliary_power_w 1500     // Reduce if too high
   hover_power_w 18000        // Check matches actual hover power
end_processor'''
    add_code_block(doc, check2)

    doc.add_heading('Problem: Aircraft oscillates or is unstable', level=3)
    doc.add_paragraph('Check: Controller gains - reduce proportional, increase derivative')
    check3 = '''mover Rotorcraft6DOF_Mover
   // Reduce Kp if oscillating
   altitude_kp 0.2       // Reduce from 0.3
   attitude_kp 5.0       // Reduce from 8.0

   // Increase Kd to dampen oscillations
   altitude_kd 1.5       // Increase from 1.2
   attitude_kd 3.0       // Increase from 2.0
end_mover'''
    add_code_block(doc, check3)

    doc.add_heading('8.3 Common Errors', level=2)

    errors = [
        ('NaN in position', 'Numerical instability', 'Reduce time step or check mass/inertia values'),
        ('SOC jumps to 100%', 'SOC init bug (v1.0)', 'Update to v1.0.1+ with fixed initialization'),
        ('No power output', 'use_dynamic_power false', 'Set use_dynamic_power true'),
        ('VRS always CLEAR', 'Thresholds too high', 'Check vrs_descent_threshold (should be 4.0)'),
        ('Slung load explodes', 'Time step too large', 'Reduce scenario time_step to 0.02 sec'),
    ]
    add_table_with_header(doc, ['Error', 'Likely Cause', 'Solution'], errors)

    doc.add_page_break()

    # =========================================================================
    # SECTION 9: ADAPTING FOR OTHER AIRCRAFT
    # =========================================================================
    doc.add_heading('9. Adapting for Other Aircraft', level=1)

    doc.add_heading('9.1 Quadcopter Example', level=2)
    doc.add_paragraph('Configuration for a 5kg racing quadcopter:')

    quad_code = '''platform_type RACING_QUAD WSF_PLATFORM
   mover Rotorcraft6DOF_Mover
      mass 5.0 kg
      inertia_xx 0.05
      inertia_yy 0.05
      inertia_zz 0.08

      rotor_configuration QUAD_X
      rotor_count 4
      rotor_diameter 0.25 m        // 10" props
      arm_length 0.18 m

      // Aggressive tuning for racing
      altitude_kp 0.5
      altitude_ki 0.1
      altitude_kd 0.8
      max_tilt_angle 45.0 deg      // Allow aggressive banking
   end_mover

   processor BATTERY T150_BATTERY_PROCESSOR
      battery_capacity_wh 300       // 4S 5000mAh
      nominal_voltage 14.8
      motor_efficiency 0.88
      auxiliary_power_w 50
   end_processor
end_platform_type'''
    add_code_block(doc, quad_code, 8)

    doc.add_heading('9.2 Hexacopter Example', level=2)
    doc.add_paragraph('Configuration for a 25kg heavy-lift hexacopter:')

    hex_code = '''platform_type HEAVY_HEX WSF_PLATFORM
   mover Rotorcraft6DOF_Mover
      mass 25.0 kg
      inertia_xx 1.2
      inertia_yy 1.2
      inertia_zz 2.0

      rotor_configuration HEX_X
      rotor_count 6
      rotor_diameter 0.55 m        // 22" props
      arm_length 0.4 m

      // Conservative tuning for stability
      altitude_kp 0.25
      altitude_ki 0.03
      altitude_kd 1.0
      max_tilt_angle 20.0 deg
   end_mover

   processor BATTERY T150_BATTERY_PROCESSOR
      battery_capacity_wh 2500      // 6S 20Ah
      nominal_voltage 22.2
      motor_efficiency 0.85
      auxiliary_power_w 200
      hover_power_w 2500
   end_processor

   processor PROP T150_PROPULSION_PROCESSOR
      motor_count 6
      max_power_per_motor 800
      empty_mass 20.0
      payload_mass 5.0
      max_payload_mass 10.0
   end_processor
end_platform_type'''
    add_code_block(doc, hex_code, 8)

    doc.add_heading('9.3 Controller Tuning Guide', level=2)

    tuning_table = [
        ('altitude_kp', '0.2', 'Slow climb', 'Increase', 'Overshoot', 'Decrease'),
        ('altitude_ki', '0.02', 'Altitude drift', 'Increase', 'Slow settling', 'Decrease'),
        ('altitude_kd', '0.5', 'Oscillation', 'Increase', 'Sluggish response', 'Decrease'),
        ('position_kp', '0.5', 'Slow waypoint', 'Increase', 'Position overshoot', 'Decrease'),
        ('velocity_kp', '2.0', 'Sluggish velocity', 'Increase', 'Velocity oscillation', 'Decrease'),
        ('attitude_kp', '5.0', 'Slow leveling', 'Increase', 'Attitude oscillation', 'Decrease'),
        ('attitude_kd', '1.0', 'Oscillation', 'Increase', 'Slow damping', 'Decrease'),
    ]
    add_table_with_header(doc, ['Gain', 'Start', 'If Problem', 'Action', 'If Opposite', 'Action'], tuning_table)

    doc.add_page_break()

    # =========================================================================
    # SECTION 10: SOURCES
    # =========================================================================
    doc.add_heading('10. Sources and References', level=1)

    sources = [
        ('AAIB Accident Investigation', 'TRV-150 incident report, South Scarle, 2023'),
        ('US Patent US20240262499A1', 'Battery topology for heavy-lift UAS'),
        ('FAA Aircraft Registry', 'TRV-150 configuration data'),
        ('T-Motor U15II Specifications', 'Motor performance data'),
        ('AFSIM 2.2505 Documentation', 'Framework documentation'),
        ('Cheeseman & Bennett (1955)', 'Ground effect model'),
        ('Johnson, W. (1980)', 'Helicopter Theory'),
        ('Leishman, J.G. (2006)', 'Principles of Helicopter Aerodynamics'),
    ]

    for i, (title, desc) in enumerate(sources, 1):
        p = doc.add_paragraph()
        p.add_run(f'[{i}] {title}: ').bold = True
        p.add_run(desc)

    doc.add_page_break()

    # =========================================================================
    # SECTION 11: ACRONYMS
    # =========================================================================
    doc.add_heading('11. Acronyms and Abbreviations', level=1)

    acronyms = [
        ('ADD', 'Architecture Description Document'),
        ('AFSIM', 'Advanced Framework for Simulation, Integration, and Modeling'),
        ('AGL', 'Above Ground Level'),
        ('CG', 'Center of Gravity'),
        ('DOF', 'Degree of Freedom'),
        ('ESC', 'Electronic Speed Controller'),
        ('GE', 'Ground Effect'),
        ('IGE', 'In Ground Effect'),
        ('KV', 'Motor velocity constant (RPM per Volt)'),
        ('MTOW', 'Maximum Takeoff Weight'),
        ('NED', 'North-East-Down coordinate frame'),
        ('OCV', 'Open-Circuit Voltage'),
        ('OGE', 'Out of Ground Effect'),
        ('PID', 'Proportional-Integral-Derivative'),
        ('RPM', 'Revolutions Per Minute'),
        ('SOC', 'State of Charge'),
        ('UAM', 'User and Analyst Manual'),
        ('UAS', 'Unmanned Aerial System'),
        ('V&V', 'Verification and Validation'),
        ('VRS', 'Vortex Ring State'),
        ('Wh', 'Watt-hour'),
    ]
    add_table_with_header(doc, ['Acronym', 'Definition'], acronyms)

    # =========================================================================
    # FOOTER
    # =========================================================================
    doc.add_paragraph()
    doc.add_paragraph()
    footer = doc.add_paragraph()
    footer.alignment = WD_ALIGN_PARAGRAPH.CENTER
    footer.add_run('Document generated with assistance from Claude Code').italic = True
    footer.add_run(f'\n{datetime.date.today().strftime("%B %d, %Y")}').italic = True

    # Save document
    output_path = '/home/shepard/afsim_projects/project005_t150_uas/VandV_docs/T150_UAS_UAM_v2.0.docx'
    doc.save(output_path)
    print(f'UAM v2.0 document created: {output_path}')
    return output_path


if __name__ == '__main__':
    create_uam_document()
