#!/usr/bin/env python3
"""
T-150 UAS Architecture Description Document (ADD) Generator
Following NGIC/TMAP V&V template format
"""

from docx import Document
from docx.shared import Inches, Pt, Cm
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.enum.style import WD_STYLE_TYPE
from docx.oxml.ns import qn
from docx.oxml import OxmlElement
import datetime

def set_cell_shading(cell, fill_color):
    """Set cell background color"""
    shading_elm = OxmlElement('w:shd')
    shading_elm.set(qn('w:fill'), fill_color)
    cell._tc.get_or_add_tcPr().append(shading_elm)

def add_heading_with_style(doc, text, level):
    """Add a heading with proper styling"""
    heading = doc.add_heading(text, level=level)
    return heading

def create_add_document():
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
    title_run = title_para.add_run("\n\n\n\nARCHITECTURE DESCRIPTION DOCUMENT\n\n")
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

    # Version info
    version_para = doc.add_paragraph()
    version_para.alignment = WD_ALIGN_PARAGRAPH.CENTER
    version_para.add_run(f"\n\nVersion 1.0\n{datetime.date.today().strftime('%d %B %Y')}\n\n")

    # Classification placeholder
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

    # Header row
    header_cells = rev_table.rows[0].cells
    headers = ['Version', 'Date', 'Author', 'Description']
    for i, header in enumerate(headers):
        header_cells[i].text = header
        set_cell_shading(header_cells[i], 'D9E2F3')
        header_cells[i].paragraphs[0].runs[0].bold = True

    # Data rows
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
    toc_para.add_run('   1.1 Purpose\n')
    toc_para.add_run('   1.2 Threat System Description\n')
    toc_para.add_run('   1.3 Simulation Framework\n')
    toc_para.add_run('   1.4 Design Standards\n')
    toc_para.add_run('2. Model Architecture Description\n')
    toc_para.add_run('   2.1 Architecture Overview\n')
    toc_para.add_run('   2.2 Plugin Component Descriptions\n')
    toc_para.add_run('   2.3 Mathematical Models\n')
    toc_para.add_run('   2.4 Inter-Processor Communication\n')
    toc_para.add_run('   2.5 Design Decisions\n')
    toc_para.add_run('3. Sources and References\n')
    toc_para.add_run('4. Acronyms and Abbreviations\n')

    doc.add_page_break()

    # =========================================================================
    # SECTION 1: INTRODUCTION
    # =========================================================================
    doc.add_heading('1. Introduction', level=1)

    # 1.1 Purpose
    doc.add_heading('1.1 Purpose', level=2)
    doc.add_paragraph(
        'This Architecture Description Document (ADD) describes the design, implementation, '
        'and mathematical foundations of the T-150/TRV-150 Heavy-Lift UAS simulation developed '
        'using AFSIM (Advanced Framework for Simulation, Integration, and Modeling). This document '
        'provides the technical details necessary for verification and validation (V&V) of the '
        'simulation model, including the algorithms, design choices, and inter-component interfaces.'
    )

    # 1.2 Threat System Description
    doc.add_heading('1.2 Threat System Description', level=2)
    doc.add_paragraph(
        'The T-150/TRV-150 is a heavy-lift unmanned aerial system manufactured by Malloy Aeronautics '
        'and marketed as the SURVICE TRV-150. It is a coaxial octocopter configuration with 8 rotors '
        'mounted on 4 arms, designed for cargo delivery and logistics missions.'
    )

    # Platform specs table
    doc.add_heading('1.2.1 Platform Specifications', level=3)
    spec_table = doc.add_table(rows=14, cols=3)
    spec_table.style = 'Table Grid'

    spec_headers = ['Parameter', 'Value', 'Source']
    for i, header in enumerate(spec_headers):
        spec_table.rows[0].cells[i].text = header
        set_cell_shading(spec_table.rows[0].cells[i], 'D9E2F3')
        spec_table.rows[0].cells[i].paragraphs[0].runs[0].bold = True

    specs = [
        ('Configuration', 'Coaxial Octocopter (8 rotors, 4 arms)', 'Technical Documentation'),
        ('Motors', '8x T-Motor U15II KV100', 'FAA Registry'),
        ('Total Thrust', '240 kg', 'AAIB Report'),
        ('MTOW', '123 kg (271 lbs)', 'AAIB Report'),
        ('Max Payload', '68 kg (150 lbs)', 'Manufacturer'),
        ('Empty Weight', '~55 kg (estimated)', 'Derived'),
        ('Cruise Speed', '30 m/s (108 km/h)', 'Manufacturer'),
        ('Max Airspeed', '40+ m/s', 'AAIB (demonstrated)'),
        ('Max Rate of Climb', '8 m/s', 'Derived'),
        ('Max Rate of Descent', '4 m/s (VRS limit)', 'Aerodynamic Analysis'),
        ('Battery', '13S Li-Ion, 46.8V nominal', 'US Patent'),
        ('Battery Capacity', '~7 kWh (148.75 Ah)', 'US Patent'),
        ('Flight Time', '36 minutes (nominal)', 'Manufacturer'),
    ]

    for i, (param, value, source) in enumerate(specs):
        row = spec_table.rows[i + 1]
        row.cells[0].text = param
        row.cells[1].text = value
        row.cells[2].text = source

    # 1.3 Simulation Framework
    doc.add_heading('1.3 Simulation Framework', level=2)
    doc.add_paragraph(
        'The simulation is built on AFSIM version 2.2505, utilizing the WSF_FULL_ROTORCRAFT_MOVER '
        'for flight dynamics and custom C++ processor plugins for subsystem modeling. The framework '
        'provides table-driven performance characteristics with mode-based operation (DEFAULT, HOVER) '
        'and realistic rotorcraft kinematics.'
    )

    # Framework components table
    fw_table = doc.add_table(rows=5, cols=2)
    fw_table.style = 'Table Grid'

    fw_table.rows[0].cells[0].text = 'Component'
    fw_table.rows[0].cells[1].text = 'Description'
    set_cell_shading(fw_table.rows[0].cells[0], 'D9E2F3')
    set_cell_shading(fw_table.rows[0].cells[1], 'D9E2F3')
    fw_table.rows[0].cells[0].paragraphs[0].runs[0].bold = True
    fw_table.rows[0].cells[1].paragraphs[0].runs[0].bold = True

    fw_data = [
        ('AFSIM 2.2505', 'Core simulation framework with entity-based modeling'),
        ('WSF_FULL_ROTORCRAFT_MOVER', 'Table-driven rotorcraft dynamics with PID control'),
        ('WsfProcessor Plugins', 'Custom C++ plugins for battery, propulsion, VRS, slung load'),
        ('AuxData Interface', 'Inter-processor communication via key-value store'),
    ]
    for i, (comp, desc) in enumerate(fw_data):
        fw_table.rows[i + 1].cells[0].text = comp
        fw_table.rows[i + 1].cells[1].text = desc

    # 1.4 Design Standards
    doc.add_heading('1.4 Design Standards', level=2)
    doc.add_paragraph(
        'The simulation adheres to the following design principles:'
    )
    standards_list = [
        'Processor Independence: Each processor functions independently with fallback values',
        'Loose Coupling: Processors communicate via AuxData, not direct references',
        'Script Accessibility: All key values accessible from scenario scripts',
        'Real-Time Execution: Plugins operate at 100ms update intervals (10 Hz)',
        'Stable Numerics: Quadratic solutions and semi-implicit Euler integration for stability',
    ]
    for standard in standards_list:
        p = doc.add_paragraph(standard, style='List Bullet')

    doc.add_page_break()

    # =========================================================================
    # SECTION 2: MODEL ARCHITECTURE DESCRIPTION
    # =========================================================================
    doc.add_heading('2. Model Architecture Description', level=1)

    # 2.1 Architecture Overview
    doc.add_heading('2.1 Architecture Overview', level=2)
    doc.add_paragraph(
        'The T-150 UAS simulation extends AFSIM with four custom processor plugins that model '
        'the engineering subsystems of the aircraft. These plugins communicate through the '
        "platform's AuxData container, providing loose coupling and script accessibility."
    )

    # Architecture diagram (text representation)
    doc.add_heading('2.1.1 System Block Diagram', level=3)
    arch_text = doc.add_paragraph()
    arch_text.style = 'No Spacing'
    arch_run = arch_text.add_run(
'''
+------------------------------------------------------------------+
|                       AFSIM Platform                              |
|                                                                   |
|  +----------------+   +----------------+   +------------------+   |
|  | Propulsion     |   | Battery        |   | VRS              |   |
|  | Processor      |   | Processor      |   | Processor        |   |
|  |                |   |                |   |                  |   |
|  | - Motor states |-->| - SOC tracking |   | - Descent rate   |   |
|  | - Thermal model|   | - Voltage sag  |   | - VRS margin     |   |
|  | - Ground effect|   | - Energy usage |   | - State machine  |   |
|  | - Payload mass |   |                |   |                  |   |
|  +-------+--------+   +--------+-------+   +--------+---------+   |
|          |                     |                    |             |
|          v                     v                    v             |
|  +----------------------------------------------------------+    |
|  |                  Platform AuxData                         |    |
|  |  PROP_TOTAL_POWER, BATTERY_SOC, VRS_MARGIN, SLUNG_*       |    |
|  +----------------------------------------------------------+    |
|          |                                                        |
|          v                                                        |
|  +----------------------------------------------------------+    |
|  |              Platform Mover (Rotorcraft)                  |    |
|  |          GetVelocityNED(), GetAltitude()                  |    |
|  +----------------------------------------------------------+    |
|                                                                   |
|  +------------------+                                             |
|  | Slung Load       |                                             |
|  | Processor        |                                             |
|  | - Pendulum dyn.  |                                             |
|  | - Cable tension  |                                             |
|  +------------------+                                             |
+------------------------------------------------------------------+
'''
    )
    arch_run.font.name = 'Courier New'
    arch_run.font.size = Pt(8)

    # Plugin summary table
    doc.add_heading('2.1.2 Plugin Summary', level=3)
    plugin_table = doc.add_table(rows=5, cols=3)
    plugin_table.style = 'Table Grid'

    plugin_table.rows[0].cells[0].text = 'Plugin'
    plugin_table.rows[0].cells[1].text = 'Class'
    plugin_table.rows[0].cells[2].text = 'Purpose'
    for cell in plugin_table.rows[0].cells:
        set_cell_shading(cell, 'D9E2F3')
        cell.paragraphs[0].runs[0].bold = True

    plugins = [
        ('t150_battery', 'BatteryProcessor', 'Battery SOC, voltage sag, energy tracking'),
        ('t150_propulsion', 'PropulsionProcessor', 'Motor power, thermal, RPM, thrust, ground effect'),
        ('t150_vrs', 'VRSProcessor', 'Vortex Ring State detection and warning'),
        ('t150_slung_load', 'SlungLoadProcessor', 'Spherical pendulum dynamics for cable loads'),
    ]
    for i, (plugin, cls, purpose) in enumerate(plugins):
        plugin_table.rows[i + 1].cells[0].text = plugin
        plugin_table.rows[i + 1].cells[1].text = cls
        plugin_table.rows[i + 1].cells[2].text = purpose

    doc.add_page_break()

    # 2.2 Plugin Component Descriptions
    doc.add_heading('2.2 Plugin Component Descriptions', level=2)

    # Battery Processor
    doc.add_heading('2.2.1 Battery Processor (T150_BATTERY_PROCESSOR)', level=3)
    doc.add_paragraph(
        'The Battery Processor models a 13S Li-Ion battery pack with realistic voltage sag behavior '
        'under load. It tracks state of charge (SOC), computes terminal voltage using a polynomial '
        'OCV curve, and provides warnings for low battery and voltage sag conditions.'
    )

    doc.add_heading('Configuration Parameters', level=4)
    batt_params = doc.add_table(rows=11, cols=4)
    batt_params.style = 'Table Grid'

    batt_params.rows[0].cells[0].text = 'Parameter'
    batt_params.rows[0].cells[1].text = 'Default'
    batt_params.rows[0].cells[2].text = 'Units'
    batt_params.rows[0].cells[3].text = 'Description'
    for cell in batt_params.rows[0].cells:
        set_cell_shading(cell, 'E2EFDA')
        cell.paragraphs[0].runs[0].bold = True

    batt_data = [
        ('battery_capacity_wh', '7000.0', 'Wh', 'Total battery capacity'),
        ('initial_soc', '1.0', '0-1', 'Initial state of charge'),
        ('hover_power_w', '18000.0', 'W', 'Fallback hover power'),
        ('low_battery_warning', '0.20', '0-1', 'SOC threshold for warning'),
        ('nominal_voltage', '46.8', 'V', '13S x 3.6V nominal'),
        ('full_charge_voltage', '54.6', 'V', '13S x 4.2V full'),
        ('cutoff_voltage', '39.0', 'V', '13S x 3.0V cutoff'),
        ('internal_resistance', '0.025', 'Ohms', 'Pack internal resistance'),
        ('voltage_sag_warning', '42.0', 'V', 'Voltage sag warning threshold'),
        ('use_dynamic_power', 'false', 'bool', 'Read power from propulsion'),
    ]
    for i, row_data in enumerate(batt_data):
        for j, cell_data in enumerate(row_data):
            batt_params.rows[i + 1].cells[j].text = cell_data

    # Propulsion Processor
    doc.add_heading('2.2.2 Propulsion Processor (T150_PROPULSION_PROCESSOR)', level=3)
    doc.add_paragraph(
        'The Propulsion Processor models the 8-motor coaxial propulsion system. It includes '
        'motor thermal dynamics, ground effect augmentation, payload mass tracking, and provides '
        'total power consumption data to the battery processor.'
    )

    doc.add_heading('Configuration Parameters', level=4)
    prop_params = doc.add_table(rows=14, cols=4)
    prop_params.style = 'Table Grid'

    prop_params.rows[0].cells[0].text = 'Parameter'
    prop_params.rows[0].cells[1].text = 'Default'
    prop_params.rows[0].cells[2].text = 'Units'
    prop_params.rows[0].cells[3].text = 'Description'
    for cell in prop_params.rows[0].cells:
        set_cell_shading(cell, 'E2EFDA')
        cell.paragraphs[0].runs[0].bold = True

    prop_data = [
        ('motor_count', '8', '-', 'Number of motors'),
        ('kv_rating', '100.0', 'RPM/V', 'Motor KV rating'),
        ('max_power_per_motor', '3500.0', 'W', 'Maximum power per motor'),
        ('propeller_diameter', '0.76', 'm', '30" propeller diameter'),
        ('coaxial_efficiency', '0.85', '0-1', 'Coaxial interference factor'),
        ('motor_thermal_limit', '120.0', 'C', 'Motor thermal limit'),
        ('motor_thermal_mass', '1500.0', 'J/K', 'Motor thermal mass'),
        ('motor_cooling_rate', '20.0', 'W/K', 'Forced convection cooling'),
        ('motor_time_constant', '0.15', 's', 'RPM response time constant'),
        ('ground_effect_enabled', 'true', 'bool', 'Enable ground effect'),
        ('empty_mass', '68.0', 'kg', 'Empty aircraft mass'),
        ('payload_mass', '0.0', 'kg', 'Current payload mass'),
        ('max_payload_mass', '55.0', 'kg', 'Maximum payload capacity'),
    ]
    for i, row_data in enumerate(prop_data):
        for j, cell_data in enumerate(row_data):
            prop_params.rows[i + 1].cells[j].text = cell_data

    # VRS Processor
    doc.add_heading('2.2.3 VRS Processor (T150_VRS_PROCESSOR)', level=3)
    doc.add_paragraph(
        'The VRS Processor detects Vortex Ring State conditions based on descent rate and '
        'forward speed. It implements a 5-state machine (CLEAR, CAUTION, ONSET, DEVELOPED, '
        'RECOVERY) and computes lift and control degradation factors.'
    )

    doc.add_heading('Configuration Parameters', level=4)
    vrs_params = doc.add_table(rows=9, cols=4)
    vrs_params.style = 'Table Grid'

    vrs_params.rows[0].cells[0].text = 'Parameter'
    vrs_params.rows[0].cells[1].text = 'Default'
    vrs_params.rows[0].cells[2].text = 'Units'
    vrs_params.rows[0].cells[3].text = 'Description'
    for cell in vrs_params.rows[0].cells:
        set_cell_shading(cell, 'E2EFDA')
        cell.paragraphs[0].runs[0].bold = True

    vrs_data = [
        ('vrs_descent_threshold', '4.0', 'm/s', 'Descent rate for VRS onset'),
        ('vrs_speed_threshold', '5.0', 'm/s', 'Forward speed to exit VRS'),
        ('caution_margin', '0.8', '0-1', 'Margin for caution state'),
        ('induced_velocity', '8.0', 'm/s', 'Hover induced velocity'),
        ('coaxial_factor', '0.85', '0-1', 'Coaxial VRS resistance factor'),
        ('vrs_development_time', '2.0', 's', 'Time to develop full VRS'),
        ('vrs_recovery_time', '3.0', 's', 'Time to recover from VRS'),
        ('max_lift_degradation', '0.6', '0-1', 'Maximum lift loss in VRS'),
    ]
    for i, row_data in enumerate(vrs_data):
        for j, cell_data in enumerate(row_data):
            vrs_params.rows[i + 1].cells[j].text = cell_data

    # Slung Load Processor
    doc.add_heading('2.2.4 Slung Load Processor (T150_SLUNG_LOAD_PROCESSOR)', level=3)
    doc.add_paragraph(
        'The Slung Load Processor models cable-suspended payload dynamics using a spherical '
        'pendulum approximation. It computes swing angles, cable tension, and load forces '
        'on the aircraft, with resonance detection and overload protection.'
    )

    doc.add_heading('Configuration Parameters', level=4)
    slung_params = doc.add_table(rows=9, cols=4)
    slung_params.style = 'Table Grid'

    slung_params.rows[0].cells[0].text = 'Parameter'
    slung_params.rows[0].cells[1].text = 'Default'
    slung_params.rows[0].cells[2].text = 'Units'
    slung_params.rows[0].cells[3].text = 'Description'
    for cell in slung_params.rows[0].cells:
        set_cell_shading(cell, 'E2EFDA')
        cell.paragraphs[0].runs[0].bold = True

    slung_data = [
        ('load_mass', '0.0', 'kg', 'Suspended load mass'),
        ('cable_length', '5.0', 'm', 'Cable length'),
        ('cable_damping', '0.1', '1/s', 'Damping coefficient'),
        ('attach_point_z', '0.5', 'm', 'Attach point below CG'),
        ('max_cable_tension', '5000.0', 'N', 'Maximum cable tension'),
        ('resonance_threshold', '20.0', 'deg', 'Swing angle for warning'),
        ('update_interval', '0.02', 's', '50 Hz dynamics update'),
        ('', '', '', ''),
    ]
    for i, row_data in enumerate(slung_data[:-1]):
        for j, cell_data in enumerate(row_data):
            slung_params.rows[i + 1].cells[j].text = cell_data

    doc.add_page_break()

    # 2.3 Mathematical Models
    doc.add_heading('2.3 Mathematical Models', level=2)

    # Battery voltage model
    doc.add_heading('2.3.1 Battery Voltage Sag Model', level=3)
    doc.add_paragraph(
        'The battery models realistic Li-Ion voltage behavior under load using open-circuit '
        'voltage (OCV), internal resistance, and a stable quadratic solution for current draw.'
    )

    doc.add_heading('Open-Circuit Voltage (OCV)', level=4)
    doc.add_paragraph(
        'The OCV varies with SOC following a polynomial approximation of the Li-Ion discharge curve:'
    )
    ocv_formula = doc.add_paragraph()
    ocv_formula.add_run('V_cell = 3.0 + 1.2*SOC - 0.5*SOC').italic = True
    ocv_formula.add_run('2').font.superscript = True
    ocv_formula.add_run(' + 0.5*SOC').italic = True
    ocv_formula.add_run('3').font.superscript = True

    doc.add_paragraph(
        'This produces per-cell voltages of ~4.2V at 100% SOC, ~3.6V at 50% SOC, and ~3.0V at 0% SOC. '
        'The pack voltage is computed by multiplying the cell voltage by the cell count (13 for a 13S pack).'
    )

    doc.add_heading('Quadratic Current Solution', level=4)
    doc.add_paragraph(
        'For a constant power load P with internal resistance R and open-circuit voltage OCV, '
        'the terminal voltage and current are related by:'
    )
    eq1 = doc.add_paragraph()
    eq1.add_run('P = V_terminal * I').italic = True
    eq1.add_run(' and ').italic = False
    eq1.add_run('V_terminal = OCV - I*R').italic = True

    doc.add_paragraph('Substituting and solving the quadratic equation:')
    eq2 = doc.add_paragraph()
    eq2.add_run('I = (OCV - sqrt(OCV').italic = True
    eq2.add_run('2').font.superscript = True
    eq2.add_run(' - 4*R*P)) / (2*R)').italic = True

    doc.add_paragraph(
        'This quadratic solution is stable because it directly solves for current without '
        'the feedback instability that can occur with iterative I = P/V calculations.'
    )

    doc.add_heading('SOC-Dependent Internal Resistance', level=4)
    doc.add_paragraph(
        'Internal resistance follows a U-shaped curve with minimum around 60% SOC:'
    )
    ir_list = [
        'SOC > 60%: R_factor = 1.0 + 0.3*(SOC - 0.6)^2 / 0.16 (peaks ~1.3x at 100%)',
        'SOC 20-60%: R_factor = 1.0 (minimum resistance)',
        'SOC < 20%: R_factor = 1.0 + 2.0*(0.2 - SOC)^2 / 0.04 (peaks ~3x at 0%)',
    ]
    for item in ir_list:
        doc.add_paragraph(item, style='List Bullet')

    # Motor thermal model
    doc.add_heading('2.3.2 Motor Thermal Model', level=3)
    doc.add_paragraph(
        'Each motor accumulates heat from electrical losses and dissipates via forced convection:'
    )

    therm_eq = doc.add_paragraph()
    therm_eq.add_run('dT/dt = (Q_in - Q_out) / C_thermal').italic = True

    doc.add_paragraph('Where:')
    therm_list = [
        'Q_in = P_electrical * (1 - efficiency) [Heat from losses]',
        'Q_out = k_cooling * (T_motor - T_ambient) [Convective cooling]',
        'C_thermal = 1500 J/K [Motor thermal mass]',
        'k_cooling = 20 W/K [Cooling rate from prop wash]',
    ]
    for item in therm_list:
        doc.add_paragraph(item, style='List Bullet')

    # Motor time constant
    doc.add_heading('2.3.3 Motor Response Dynamics', level=3)
    doc.add_paragraph(
        'Motor RPM follows a first-order lag model for actuator dynamics:'
    )
    motor_eq = doc.add_paragraph()
    motor_eq.add_run('alpha = dt / (tau + dt)').italic = True
    doc.add_paragraph()
    rpm_eq = doc.add_paragraph()
    rpm_eq.add_run('RPM_new = RPM_old + alpha * (RPM_target - RPM_old)').italic = True

    doc.add_paragraph('With tau = 150ms time constant for large motors with 30" props.')

    # Ground effect
    doc.add_heading('2.3.4 Ground Effect Model', level=3)
    doc.add_paragraph(
        'Ground effect thrust augmentation uses the Cheeseman-Bennett approximation:'
    )
    ge_eq = doc.add_paragraph()
    ge_eq.add_run('T_IGE / T_OGE = 1 / (1 - (R / 4h)').italic = True
    ge_eq.add_run('2').font.superscript = True
    ge_eq.add_run(')').italic = True

    doc.add_paragraph('Where:')
    ge_list = [
        'T_IGE = Thrust in ground effect',
        'T_OGE = Thrust out of ground effect',
        'R = Rotor radius (0.38m for 30" props)',
        'h = Rotor height above ground',
    ]
    for item in ge_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_paragraph(
        'Ground effect is active below 1 rotor diameter AGL (0.76m) with maximum boost '
        'clamped to 30% (1.3x thrust multiplier).'
    )

    # VRS model
    doc.add_heading('2.3.5 Vortex Ring State Detection', level=3)
    doc.add_paragraph(
        'VRS occurs when high descent rate causes the rotor to descend through its own wake. '
        'The VRS margin combines descent rate and forward speed factors:'
    )

    vrs_eq1 = doc.add_paragraph()
    vrs_eq1.add_run('descent_margin = 1.0 - (descent_rate / threshold)').italic = True

    vrs_eq2 = doc.add_paragraph()
    vrs_eq2.add_run('speed_margin = min(1.0, forward_speed / speed_threshold)').italic = True

    doc.add_paragraph(
        'Forward speed above 5 m/s provides sufficient fresh air inflow to prevent VRS. '
        'Coaxial rotors have inherent VRS resistance (0.85 factor applied to thresholds).'
    )

    doc.add_heading('VRS State Machine', level=4)
    vrs_states = doc.add_table(rows=6, cols=3)
    vrs_states.style = 'Table Grid'

    vrs_states.rows[0].cells[0].text = 'State'
    vrs_states.rows[0].cells[1].text = 'Condition'
    vrs_states.rows[0].cells[2].text = 'Effect'
    for cell in vrs_states.rows[0].cells:
        set_cell_shading(cell, 'FCE4D6')
        cell.paragraphs[0].runs[0].bold = True

    vrs_state_data = [
        ('CLEAR', 'VRS margin >= 1.0', 'No degradation'),
        ('CAUTION', 'margin < 0.8', '5% lift loss'),
        ('ONSET', 'margin < 0.5', '15% lift loss developing'),
        ('DEVELOPED', 'margin < 0.2 for 2s', '60% lift loss, 50% control loss'),
        ('RECOVERY', 'margin increasing', 'Degradation decreasing over 3s'),
    ]
    for i, row_data in enumerate(vrs_state_data):
        for j, cell_data in enumerate(row_data):
            vrs_states.rows[i + 1].cells[j].text = cell_data

    # Slung load model
    doc.add_heading('2.3.6 Spherical Pendulum Dynamics', level=3)
    doc.add_paragraph(
        'The slung load is modeled as a spherical pendulum with two rotational degrees of freedom:'
    )

    pend_eq1 = doc.add_paragraph()
    pend_eq1.add_run("theta_x'' = -(g/L)*sin(theta_x) - (a_x/L)*cos(theta_x) - c*theta_x'").italic = True

    pend_eq2 = doc.add_paragraph()
    pend_eq2.add_run("theta_y'' = -(g/L)*sin(theta_y) - (a_y/L)*cos(theta_y) - c*theta_y'").italic = True

    doc.add_paragraph('Where:')
    pend_list = [
        'theta_x, theta_y = Swing angles in X-Z and Y-Z planes (rad)',
        'g = Gravitational acceleration (9.81 m/s^2)',
        'L = Cable length (m)',
        'a_x, a_y = Aircraft accelerations in X and Y (m/s^2)',
        'c = Damping coefficient (1/s)',
    ]
    for item in pend_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('Cable Tension', level=4)
    tension_eq = doc.add_paragraph()
    tension_eq.add_run('T = m * (g*cos(theta) - a_z + L*omega^2)').italic = True

    doc.add_paragraph('Including centripetal acceleration from swing angular velocity.')

    doc.add_heading('Natural Frequency', level=4)
    freq_eq = doc.add_paragraph()
    freq_eq.add_run('f = (1/2*pi) * sqrt(g/L)').italic = True

    doc.add_paragraph('For a 5m cable: f = 0.22 Hz (4.5 second period).')

    doc.add_heading('Numerical Integration', level=4)
    doc.add_paragraph(
        'Semi-implicit Euler integration is used for stability:'
    )
    euler_list = [
        "1. Update velocity: theta_dot += theta_dot_dot * dt",
        "2. Update position: theta += theta_dot * dt",
        "3. Clamp angles to +/- 70 degrees to prevent numerical blowup",
    ]
    for item in euler_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_page_break()

    # 2.4 Inter-Processor Communication
    doc.add_heading('2.4 Inter-Processor Communication', level=2)
    doc.add_paragraph(
        "Processors communicate through the platform's AuxData container, a key-value store "
        "that provides loose coupling and script accessibility."
    )

    doc.add_heading('Published AuxData Keys', level=3)

    # Split into multiple tables by processor
    doc.add_heading('Propulsion Processor Outputs', level=4)
    prop_aux = doc.add_table(rows=13, cols=3)
    prop_aux.style = 'Table Grid'

    prop_aux.rows[0].cells[0].text = 'Key'
    prop_aux.rows[0].cells[1].text = 'Type'
    prop_aux.rows[0].cells[2].text = 'Description'
    for cell in prop_aux.rows[0].cells:
        set_cell_shading(cell, 'D9E2F3')
        cell.paragraphs[0].runs[0].bold = True

    prop_aux_data = [
        ('PROP_TOTAL_POWER', 'double', 'Total electrical power draw (W)'),
        ('PROP_TOTAL_THRUST', 'double', 'Combined motor thrust (N)'),
        ('PROP_MAX_TEMP', 'double', 'Hottest motor temperature (C)'),
        ('PROP_AVG_TEMP', 'double', 'Average motor temperature (C)'),
        ('PROP_MOTOR_COUNT', 'int', 'Operational motor count'),
        ('PROP_SYSTEM_EFF', 'double', 'Overall system efficiency'),
        ('PROP_GROUND_EFFECT', 'double', 'Ground effect factor (1.0 = none)'),
        ('PROP_ALTITUDE_AGL', 'double', 'Altitude above ground (m)'),
        ('PROP_TOTAL_MASS', 'double', 'Empty + payload mass (kg)'),
        ('PROP_THRUST_MARGIN', 'double', 'Available thrust margin (0-1)'),
        ('PROP_CG_OFFSET_X/Y/Z', 'double', 'Combined CG offset (m)'),
        ('PROP_OVERLOADED', 'bool', 'Payload exceeds capacity'),
    ]
    for i, row_data in enumerate(prop_aux_data):
        for j, cell_data in enumerate(row_data):
            prop_aux.rows[i + 1].cells[j].text = cell_data

    doc.add_heading('Battery Processor Outputs', level=4)
    batt_aux = doc.add_table(rows=9, cols=3)
    batt_aux.style = 'Table Grid'

    batt_aux.rows[0].cells[0].text = 'Key'
    batt_aux.rows[0].cells[1].text = 'Type'
    batt_aux.rows[0].cells[2].text = 'Description'
    for cell in batt_aux.rows[0].cells:
        set_cell_shading(cell, 'D9E2F3')
        cell.paragraphs[0].runs[0].bold = True

    batt_aux_data = [
        ('BATTERY_SOC', 'double', 'State of charge (0-1)'),
        ('BATTERY_POWER_W', 'double', 'Current power consumption (W)'),
        ('BATTERY_ENERGY_WH', 'double', 'Remaining energy (Wh)'),
        ('BATTERY_VOLTAGE', 'double', 'Terminal voltage under load (V)'),
        ('BATTERY_OCV', 'double', 'Open-circuit voltage (V)'),
        ('BATTERY_VOLTAGE_SAG', 'double', 'Voltage drop from IR (V)'),
        ('BATTERY_CURRENT_A', 'double', 'Current draw (A)'),
        ('BATTERY_LOW', 'bool', 'Low battery warning flag'),
    ]
    for i, row_data in enumerate(batt_aux_data):
        for j, cell_data in enumerate(row_data):
            batt_aux.rows[i + 1].cells[j].text = cell_data

    doc.add_heading('VRS Processor Outputs', level=4)
    vrs_aux = doc.add_table(rows=8, cols=3)
    vrs_aux.style = 'Table Grid'

    vrs_aux.rows[0].cells[0].text = 'Key'
    vrs_aux.rows[0].cells[1].text = 'Type'
    vrs_aux.rows[0].cells[2].text = 'Description'
    for cell in vrs_aux.rows[0].cells:
        set_cell_shading(cell, 'D9E2F3')
        cell.paragraphs[0].runs[0].bold = True

    vrs_aux_data = [
        ('VRS_STATE', 'int', 'Current VRS state enum (0-4)'),
        ('VRS_MARGIN', 'double', 'Safety margin (0=VRS, 1=safe)'),
        ('VRS_DESCENT_RATE', 'double', 'Current descent rate (m/s)'),
        ('VRS_LIFT_FACTOR', 'double', 'Lift degradation factor (0-1)'),
        ('VRS_CONTROL_FACTOR', 'double', 'Control authority factor (0-1)'),
        ('VRS_CAUTION', 'bool', 'Caution warning flag'),
        ('VRS_CRITICAL', 'bool', 'Critical VRS warning flag'),
    ]
    for i, row_data in enumerate(vrs_aux_data):
        for j, cell_data in enumerate(row_data):
            vrs_aux.rows[i + 1].cells[j].text = cell_data

    doc.add_page_break()

    # 2.5 Design Decisions
    doc.add_heading('2.5 Design Decisions', level=2)

    doc.add_heading('2.5.1 Why WSF_FULL_ROTORCRAFT_MOVER', level=3)
    doc.add_paragraph(
        'The WSF_FULL_ROTORCRAFT_MOVER was selected over WSF_BASIC_ROTORCRAFT_MOVER for:'
    )
    decision_list = [
        'Table-driven performance varying with altitude and airspeed',
        'Mode-based operation (DEFAULT cruise, HOVER for station-keeping)',
        'More realistic rotorcraft dynamics with PID control',
        'Acceleration tables for realistic deceleration profiles',
    ]
    for item in decision_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('2.5.2 Why Quadratic Voltage Solution', level=3)
    doc.add_paragraph(
        'The quadratic solution for battery current was chosen over iterative I=P/V because:'
    )
    quad_list = [
        'Iterative solutions can diverge when voltage drops cause current spikes',
        'Quadratic directly solves without feedback loops',
        'Handles edge cases (discriminant < 0 means power exceeds capability)',
        'Computationally efficient (one sqrt vs multiple iterations)',
    ]
    for item in quad_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('2.5.3 Why 100ms Update Interval', level=3)
    doc.add_paragraph(
        'The 100ms (10 Hz) update interval balances accuracy and performance:'
    )
    interval_list = [
        'Battery and propulsion dynamics are slow (thermal time constants > 1s)',
        'VRS detection needs reasonable response time but not real-time control',
        'Matches AFSIM typical processor update rates',
        'Slung load uses 20ms (50 Hz) for pendulum dynamics stability',
    ]
    for item in interval_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('2.5.4 Why AuxData for Communication', level=3)
    doc.add_paragraph(
        'The AuxData interface was chosen for inter-processor communication because:'
    )
    aux_list = [
        'Loose coupling: Processors work independently without direct references',
        'Script access: Scenario scripts can read/write values',
        'Debugging: All values visible in AFSIM data inspection tools',
        'Fallback support: Processors can use defaults if peer unavailable',
    ]
    for item in aux_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_page_break()

    # =========================================================================
    # SECTION 3: SOURCES
    # =========================================================================
    doc.add_heading('3. Sources and References', level=1)

    sources = [
        ('AAIB Accident Investigation', 'Air Accidents Investigation Branch report on TRV-150 incident at South Scarle, Nottinghamshire, UK, 2023'),
        ('US Patent US20240262499A1', 'Battery topology and power management for heavy-lift UAS'),
        ('FAA Aircraft Registry', 'TRV-150 registration and configuration data'),
        ('T-Motor U15II Specifications', 'Motor performance data: KV100, 3.5kW max power'),
        ('AFSIM Documentation', 'AFSIM 2.2505 framework documentation, WSF_FULL_ROTORCRAFT_MOVER'),
        ('Cheeseman & Bennett (1955)', 'Ground effect model: "The Effect of the Ground on a Helicopter Rotor"'),
        ('Johnson, W. (1980)', 'Helicopter Theory, Princeton University Press - VRS conditions'),
        ('Prouty, R. (1986)', 'Helicopter Performance, Stability, and Control - rotorcraft dynamics'),
    ]

    for i, (title, desc) in enumerate(sources, 1):
        p = doc.add_paragraph()
        p.add_run(f'[{i}] {title}: ').bold = True
        p.add_run(desc)

    doc.add_page_break()

    # =========================================================================
    # SECTION 4: ACRONYMS
    # =========================================================================
    doc.add_heading('4. Acronyms and Abbreviations', level=1)

    acronym_table = doc.add_table(rows=21, cols=2)
    acronym_table.style = 'Table Grid'

    acronym_table.rows[0].cells[0].text = 'Acronym'
    acronym_table.rows[0].cells[1].text = 'Definition'
    set_cell_shading(acronym_table.rows[0].cells[0], 'D9E2F3')
    set_cell_shading(acronym_table.rows[0].cells[1], 'D9E2F3')
    acronym_table.rows[0].cells[0].paragraphs[0].runs[0].bold = True
    acronym_table.rows[0].cells[1].paragraphs[0].runs[0].bold = True

    acronyms = [
        ('ADD', 'Architecture Description Document'),
        ('AFSIM', 'Advanced Framework for Simulation, Integration, and Modeling'),
        ('AGL', 'Above Ground Level'),
        ('CG', 'Center of Gravity'),
        ('ESC', 'Electronic Speed Controller'),
        ('IGE', 'In Ground Effect'),
        ('IR', 'Internal Resistance'),
        ('KV', 'Motor velocity constant (RPM per Volt)'),
        ('MTOW', 'Maximum Takeoff Weight'),
        ('NED', 'North-East-Down (coordinate frame)'),
        ('OCV', 'Open-Circuit Voltage'),
        ('OGE', 'Out of Ground Effect'),
        ('PID', 'Proportional-Integral-Derivative'),
        ('RPM', 'Revolutions Per Minute'),
        ('SOC', 'State of Charge'),
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
    output_path = '/home/shepard/afsim_projects/project005_t150_uas/VandV_docs/T150_UAS_ADD_v1.0.docx'
    doc.save(output_path)
    print(f'ADD document created: {output_path}')
    return output_path

if __name__ == '__main__':
    create_add_document()
