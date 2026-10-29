#!/usr/bin/env python3
"""
T-150 UAS Architecture Description Document (ADD) Generator - Version 2.0
Comprehensive documentation including 6DOF mover, validation results, and installation guide.
Following NGIC/TMAP V&V template format.
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

def add_code_block(doc, code_text, font_size=8):
    """Add a monospace code block"""
    para = doc.add_paragraph()
    para.style = 'No Spacing'
    run = para.add_run(code_text)
    run.font.name = 'Courier New'
    run.font.size = Pt(font_size)
    return para

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

def create_add_document():
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
        ['2.0', datetime.date.today().strftime('%Y-%m-%d'), 'Project Team', 'Major update: 6DOF mover, code documentation, validation results'],
        ['1.0', '2025-12-19', 'Project Team', 'Initial release'],
        ['0.9', '2025-12-01', 'Project Team', 'Draft for review'],
    ]
    add_table_with_header(doc, ['Version', 'Date', 'Author', 'Description'], rev_data)

    doc.add_page_break()

    # =========================================================================
    # TABLE OF CONTENTS
    # =========================================================================
    doc.add_heading('Table of Contents', level=1)
    toc_para = doc.add_paragraph()
    toc_content = '''(Auto-generate in Word: References > Table of Contents)

1. Introduction
   1.1 Purpose
   1.2 Scope
   1.3 Threat System Description
   1.4 Simulation Framework
   1.5 Design Standards
2. Model Architecture Description
   2.1 Architecture Overview
   2.2 Plugin Component Descriptions
   2.3 Code Structure and Class Hierarchies
   2.4 Mathematical Models
   2.5 Inter-Processor Communication
   2.6 Design Decisions
3. 6DOF Rotorcraft Mover
   3.1 Physics Engine Overview
   3.2 Rigid Body Dynamics
   3.3 Rotor Aerodynamics
   3.4 Flight Control System
   3.5 AFSIM Integration
4. Validation and Testing
   4.1 Phase 1 Validation Suite
   4.2 Flight Pattern Analysis
   4.3 Bug Fixes and Calibration
5. Installation and Usage
   5.1 Prerequisites
   5.2 Building the Plugins
   5.3 Installing to AFSIM
   5.4 Adapting for Other Aircraft
6. Sources and References
7. Acronyms and Abbreviations
'''
    toc_para.add_run(toc_content)
    doc.add_page_break()

    # =========================================================================
    # SECTION 1: INTRODUCTION
    # =========================================================================
    doc.add_heading('1. Introduction', level=1)

    doc.add_heading('1.1 Purpose', level=2)
    doc.add_paragraph(
        'This Architecture Description Document (ADD) describes the design, implementation, '
        'and mathematical foundations of the T-150/TRV-150 Heavy-Lift UAS simulation developed '
        'using AFSIM (Advanced Framework for Simulation, Integration, and Modeling). This document '
        'provides the technical details necessary for:'
    )
    purpose_list = [
        'Verification and Validation (V&V) of the simulation model',
        'Understanding the algorithms and physics models implemented',
        'Extending or modifying the simulation for other aircraft',
        'Troubleshooting simulation behavior',
    ]
    for item in purpose_list:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('1.2 Scope', level=2)
    doc.add_paragraph('This document covers the following simulation components:')

    scope_data = [
        ('rotorcraft_6dof', 'Mover Plugin', 'Full 6DOF physics-based flight dynamics'),
        ('t150_battery', 'Processor Plugin', 'Li-Ion battery with voltage sag modeling'),
        ('t150_propulsion', 'Processor Plugin', 'Motor power, thermal, RPM, ground effect'),
        ('t150_vrs', 'Processor Plugin', 'Vortex Ring State detection'),
        ('t150_slung_load', 'Processor Plugin', 'Cable-suspended payload dynamics'),
    ]
    add_table_with_header(doc, ['Component', 'Type', 'Description'], scope_data)

    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('Total C++ Code: ').bold = True
    p.add_run('7,736 lines across 26 source files')

    doc.add_heading('1.3 Threat System Description', level=2)
    doc.add_paragraph(
        'The T-150/TRV-150 is a heavy-lift unmanned aerial system manufactured by Malloy Aeronautics '
        'and marketed as the SURVICE TRV-150. It is a coaxial octocopter configuration with 8 rotors '
        'mounted on 4 arms, designed for cargo delivery and logistics missions.'
    )

    doc.add_heading('1.3.1 Platform Specifications', level=3)
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
        ('Propeller Diameter', '30" (0.76m)', 'Technical Documentation'),
        ('Arm Length', '0.5m', 'Technical Documentation'),
    ]
    add_table_with_header(doc, ['Parameter', 'Value', 'Source'], specs)

    doc.add_heading('1.4 Simulation Framework', level=2)
    doc.add_paragraph(
        'The simulation is built on AFSIM version 2.2505, utilizing custom C++ plugins for '
        'flight dynamics and subsystem modeling.'
    )

    fw_data = [
        ('AFSIM', '2.2505.0', 'Core simulation framework'),
        ('Compiler', 'GCC 13.3', 'C++17 standard'),
        ('Build System', 'CMake 3.16+', 'Cross-platform build'),
        ('Target Platforms', 'Linux (primary), Windows', 'Shared libraries (.so/.dll)'),
    ]
    add_table_with_header(doc, ['Component', 'Version', 'Description'], fw_data)

    doc.add_heading('1.4.1 Framework Components', level=3)
    fw_components = [
        ('Rotorcraft6DOF_Mover', 'Custom physics-based mover replacing WSF_FULL_ROTORCRAFT_MOVER'),
        ('WsfProcessor Plugins', 'Custom C++ plugins for battery, propulsion, VRS, slung load'),
        ('AuxData Interface', 'Inter-processor communication via key-value store'),
        ('Script Interface', 'Scenario scripts can read/write all AuxData values'),
    ]
    add_table_with_header(doc, ['Component', 'Description'], fw_components)

    doc.add_heading('1.5 Design Standards', level=2)
    doc.add_paragraph('The simulation adheres to the following design principles:')
    standards = [
        'Physics-Based Modeling: All dynamics based on first-principles physics (Newton-Euler, momentum theory)',
        'Processor Independence: Each processor functions independently with fallback values',
        'Loose Coupling: Processors communicate via AuxData, not direct references',
        'Script Accessibility: All key values accessible from scenario scripts',
        'Numerical Stability: Semi-implicit Euler integration, quadratic solutions for stability',
        'Real-Time Capable: Plugins operate at configurable update intervals (10-50 Hz)',
    ]
    for item in standards:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_page_break()

    # =========================================================================
    # SECTION 2: MODEL ARCHITECTURE DESCRIPTION
    # =========================================================================
    doc.add_heading('2. Model Architecture Description', level=1)

    doc.add_heading('2.1 Architecture Overview', level=2)
    doc.add_paragraph(
        'The T-150 UAS simulation extends AFSIM with six custom plugins that model '
        'the engineering subsystems of the aircraft.'
    )

    # Architecture diagram
    doc.add_heading('2.1.1 System Block Diagram', level=3)
    arch_diagram = '''
+------------------------------------------------------------------+
|                       AFSIM Platform                              |
|                                                                   |
|  +------------------------------------------------------------+  |
|  |              Rotorcraft6DOF_Mover (Physics Engine)          |  |
|  |                                                              |  |
|  |  +--------------+  +---------------+  +-----------------+   |  |
|  |  | RigidBody6DOF|  | RotorModel[8] |  | FlightController|   |  |
|  |  | - Position   |  | - Thrust      |  | - Altitude PID  |   |  |
|  |  | - Velocity   |  | - Torque      |  | - Position PD   |   |  |
|  |  | - Quaternion |  | - RPM         |  | - Attitude PD   |   |  |
|  |  +--------------+  +---------------+  +-----------------+   |  |
|  +-----------------------------+------------------------------+  |
|                                |                                  |
|                                | 6dof_total_power (AuxData)       |
|                                v                                  |
|  +----------------+   +----------------+   +------------------+   |
|  | Propulsion     |   | Battery        |   | VRS              |   |
|  | Processor      |   | Processor      |   | Processor        |   |
|  |                |   |                |   |                  |   |
|  | - Motor states |-->| - SOC tracking |   | - Descent rate   |   |
|  | - Thermal model|   | - Voltage sag  |   | - VRS margin     |   |
|  | - Ground effect|   | - Energy usage |   | - State machine  |   |
|  +-------+--------+   +--------+-------+   +--------+---------+   |
|          |                     |                    |             |
|          v                     v                    v             |
|  +----------------------------------------------------------+    |
|  |                  Platform AuxData                         |    |
|  |  PROP_*, BATTERY_*, VRS_*, SLUNG_*, 6dof_*                |    |
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
    add_code_block(doc, arch_diagram, font_size=7)

    doc.add_heading('2.2 Plugin Component Descriptions', level=2)

    doc.add_heading('2.2.1 Rotorcraft 6DOF Mover', level=3)
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('File: ').bold = True
    p.add_run('plugins/rotorcraft_6dof/source/Rotorcraft6DOF_Mover.cpp')
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('Lines of Code: ').bold = True
    p.add_run('2,939')

    doc.add_paragraph(
        'The 6DOF mover provides physics-based flight dynamics using quaternion-based '
        'attitude representation (no gimbal lock), Newton-Euler equations of motion, '
        'momentum theory rotor aerodynamics, cascaded flight controllers, and semi-implicit '
        'Euler integration for numerical stability.'
    )

    mover_classes = [
        ('Rotorcraft6DOF_Mover', 'Rotorcraft6DOF_Mover.cpp', 'Main mover class, AFSIM integration'),
        ('RigidBody6DOF', 'RigidBody6DOF.hpp', 'Rigid body state and integration'),
        ('RotorModel', 'Rotorcraft6DOF_Mover.hpp', 'Individual rotor thrust/torque'),
    ]
    add_table_with_header(doc, ['Class', 'File', 'Purpose'], mover_classes)

    doc.add_heading('2.2.2 Battery Processor', level=3)
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('File: ').bold = True
    p.add_run('plugins/t150_battery/source/BatteryProcessor.cpp')
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('Lines of Code: ').bold = True
    p.add_run('1,139')

    doc.add_paragraph(
        'The Battery Processor models a 13S Li-Ion battery pack with polynomial OCV curve, '
        'SOC-dependent internal resistance, quadratic current solution for voltage sag, '
        'and low battery/voltage sag warnings.'
    )

    batt_params = [
        ('battery_capacity_wh', '7000.0', 'Wh', 'Total battery capacity'),
        ('initial_soc', '1.0', '0-1', 'Initial state of charge'),
        ('hover_power_w', '18000.0', 'W', 'Fallback hover power'),
        ('motor_efficiency', '0.85', '0-1', 'Motor/ESC efficiency'),
        ('auxiliary_power_w', '4000.0', 'W', 'Avionics, cooling, etc.'),
        ('low_battery_warning', '0.20', '0-1', 'SOC threshold for warning'),
        ('nominal_voltage', '46.8', 'V', '13S x 3.6V nominal'),
        ('full_charge_voltage', '54.6', 'V', '13S x 4.2V full'),
        ('cutoff_voltage', '39.0', 'V', '13S x 3.0V cutoff'),
        ('internal_resistance', '0.025', 'Ohms', 'Pack internal resistance'),
        ('use_dynamic_power', 'true', 'bool', 'Read power from 6DOF mover'),
        ('power_source_key', '"6dof_total_power"', 'string', 'AuxData key for power input'),
    ]
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], batt_params, 'E2EFDA')

    doc.add_heading('2.2.3 Propulsion Processor', level=3)
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('File: ').bold = True
    p.add_run('plugins/t150_propulsion/source/PropulsionProcessor.cpp')
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('Lines of Code: ').bold = True
    p.add_run('1,281')

    doc.add_paragraph(
        'The Propulsion Processor models the 8-motor coaxial propulsion system with motor '
        'thermal dynamics, ground effect thrust augmentation, payload mass tracking, and '
        'motor RPM/efficiency calculations.'
    )

    prop_params = [
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
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], prop_params, 'E2EFDA')

    doc.add_page_break()

    doc.add_heading('2.2.4 VRS Processor', level=3)
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('File: ').bold = True
    p.add_run('plugins/t150_vrs/source/VRSProcessor.cpp')
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('Lines of Code: ').bold = True
    p.add_run('697')

    doc.add_paragraph(
        'The VRS Processor detects Vortex Ring State conditions by monitoring descent rate '
        'and forward speed. It implements a 5-state machine and computes lift/control degradation factors.'
    )

    vrs_states = [
        ('CLEAR', 'margin >= 1.0', '1.0', '1.0'),
        ('CAUTION', 'margin < 0.8', '0.95', '1.0'),
        ('ONSET', 'margin < 0.5', '0.85', '0.95'),
        ('DEVELOPED', 'margin < 0.2 for 2s', '0.40', '0.50'),
        ('RECOVERY', 'margin increasing', 'Improving', 'Improving'),
    ]
    add_table_with_header(doc, ['State', 'Condition', 'Lift Factor', 'Control Factor'], vrs_states, 'FCE4D6')

    doc.add_heading('2.2.5 Slung Load Processor', level=3)
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('File: ').bold = True
    p.add_run('plugins/t150_slung_load/source/SlungLoadProcessor.cpp')
    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('Lines of Code: ').bold = True
    p.add_run('825')

    doc.add_paragraph(
        'The Slung Load Processor models cable-suspended payload dynamics using a spherical '
        'pendulum with 2 rotational DOF, semi-implicit Euler integration at 50 Hz, cable tension '
        'calculation with centripetal acceleration, and resonance detection.'
    )

    slung_params = [
        ('load_mass', '0.0', 'kg', 'Suspended load mass'),
        ('cable_length', '5.0', 'm', 'Cable length'),
        ('cable_damping', '0.1', '1/s', 'Damping coefficient'),
        ('attach_point_z', '0.5', 'm', 'Attach point below CG'),
        ('max_cable_tension', '5000.0', 'N', 'Maximum cable tension'),
        ('resonance_threshold', '20.0', 'deg', 'Swing angle for warning'),
        ('update_interval', '0.02', 's', '50 Hz dynamics update'),
    ]
    add_table_with_header(doc, ['Parameter', 'Default', 'Units', 'Description'], slung_params, 'E2EFDA')

    doc.add_heading('2.3 Code Structure and Class Hierarchies', level=2)

    doc.add_heading('2.3.1 6DOF Mover Class Hierarchy', level=3)
    hierarchy_code = '''// AFSIM Base Classes
WsfMover
    |__ WsfRouteMover           // Route-following base class
            |__ Rotorcraft6DOF_Mover   // Our custom implementation

// Internal Classes (defined in our plugin)
class RigidBody6DOF {
    // State vectors
    Vec3 mPosition;     // NED position [m]
    Vec3 mVelocity;     // NED velocity [m/s]
    Quaternion mAttitude;   // Body-to-NED rotation
    Vec3 mAngularVel;   // Body angular velocity [rad/s]

    // Physical properties
    double mMass;
    Mat3x3 mInertia;

    // Methods
    void Integrate(double dt, Vec3 force, Vec3 moment);
    Vec3 BodyToNED(Vec3 bodyVec);
    Vec3 NEDToBody(Vec3 nedVec);
};

class RotorModel {
    double mRPM;        // Current RPM
    double mThrust;     // Thrust output [N]
    double mTorque;     // Reaction torque [Nm]
    Vec3 mPosition;     // Position relative to CG
    int mDirection;     // +1 = CW, -1 = CCW

    void Update(double throttle, double dt);
};'''
    add_code_block(doc, hierarchy_code, 9)

    doc.add_heading('2.3.2 Plugin Registration Pattern', level=3)
    doc.add_paragraph(
        'Each plugin must register its types with AFSIM. Critical: Use AddCoreType() for movers, '
        'not Add(). The Add() method is for script-defined types and will not properly initialize '
        'C++-defined movers.'
    )

    registration_code = '''// In PluginRegistration.cpp
#include "afsim/script/WsfMoverTypes.hpp"
#include "Rotorcraft6DOF_Mover.hpp"
#include "Rotorcraft6DOF_Registration.hpp"

// Called when plugin loads
extern "C" void RegisterPlugin()
{
    // For movers - use AddCoreType for proper registration
    WsfMoverTypes::Get().AddCoreType<Rotorcraft6DOF_Mover>(
        "Rotorcraft6DOF_Mover",           // Type name
        Rotorcraft6DOF_Registration,       // Registration function
        nullptr,                           // Grammar file (optional)
        "rotorcraft_6dof"                  // Plugin name
    );
}

// For processors - use AddType
WsfProcessorTypes::Get().AddType<BatteryProcessor>(
    "T150_BATTERY_PROCESSOR",
    BatteryProcessor_Registration
);'''
    add_code_block(doc, registration_code, 9)

    doc.add_page_break()

    doc.add_heading('2.4 Mathematical Models', level=2)

    doc.add_heading('2.4.1 Rigid Body Dynamics', level=3)
    doc.add_paragraph('Translational Dynamics (Newton\'s Second Law):')
    trans_eq = '''F_total = F_gravity + F_thrust + F_drag
a = F_total / m
v_new = v_old + a * dt
p_new = p_old + v_new * dt   (Semi-implicit Euler)'''
    add_code_block(doc, trans_eq, 10)

    doc.add_paragraph()
    doc.add_paragraph('Rotational Dynamics (Euler\'s Equation):')
    rot_eq = '''M_total = M_rotors + M_gyroscopic
I * alpha = M_total - omega x (I * omega)
omega_new = omega_old + alpha * dt
q_new = q_old + 0.5 * q_old * [0, omega_new] * dt   (Quaternion integration)'''
    add_code_block(doc, rot_eq, 10)

    doc.add_heading('2.4.2 Rotor Aerodynamics (Momentum Theory)', level=3)
    doc.add_paragraph('Thrust Calculation:')
    thrust_eq = 'T = Ct * rho * n^2 * D^4'
    p = doc.add_paragraph()
    p.add_run(thrust_eq).italic = True

    doc.add_paragraph('Where:')
    thrust_vars = [
        'T = Thrust [N]',
        'Ct = Thrust coefficient (0.10 for this configuration)',
        'rho = Air density (1.225 kg/m^3 at sea level)',
        'n = Rotational speed [rev/s]',
        'D = Propeller diameter [m]',
    ]
    for item in thrust_vars:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_paragraph()
    doc.add_paragraph('Power Calculation:')
    power_eq = 'P = Q * omega = Cq * rho * n^3 * D^5 * 2*pi'
    p = doc.add_paragraph()
    p.add_run(power_eq).italic = True

    doc.add_paragraph()
    doc.add_paragraph('Coaxial Efficiency: Lower rotor operates in upper rotor wake:')
    coax_eq = '''eta_coaxial = 0.85   (15% efficiency loss for lower rotors)
T_total = T_upper + eta_coaxial * T_lower'''
    add_code_block(doc, coax_eq, 10)

    doc.add_heading('2.4.3 Battery Voltage Model', level=3)
    doc.add_paragraph('Open-Circuit Voltage (OCV) polynomial approximation of Li-Ion discharge curve:')
    ocv_eq = '''V_cell = 3.0 + 1.2*SOC - 0.5*SOC^2 + 0.5*SOC^3
V_pack = V_cell * N_cells   (N_cells = 13 for 13S)'''
    add_code_block(doc, ocv_eq, 10)

    doc.add_paragraph()
    doc.add_paragraph('Quadratic Current Solution for voltage sag under constant power load:')
    quad_eq = '''P = V_terminal * I
V_terminal = OCV - I * R

Substituting: P = (OCV - I*R) * I
Rearranging: R*I^2 - OCV*I + P = 0

Solving (quadratic formula):
I = (OCV - sqrt(OCV^2 - 4*R*P)) / (2*R)
V_terminal = OCV - I * R'''
    add_code_block(doc, quad_eq, 10)

    doc.add_paragraph(
        'Why Quadratic? Iterative solutions can diverge when voltage drops cause current spikes. '
        'The quadratic solution is algebraically exact and numerically stable.'
    )

    doc.add_heading('2.4.4 Ground Effect Model (Cheeseman-Bennett)', level=3)
    ge_eq = '''T_IGE / T_OGE = 1 / (1 - (R / 4h)^2)

Where:
  T_IGE = Thrust in ground effect
  T_OGE = Thrust out of ground effect (hover)
  R = Rotor radius [m]
  h = Rotor height above ground [m]'''
    add_code_block(doc, ge_eq, 10)

    doc.add_heading('2.4.5 Spherical Pendulum Dynamics', level=3)
    pend_eq = '''theta_x'' = -(g/L)*sin(theta_x) - (a_x/L)*cos(theta_x) - c*theta_x'
theta_y'' = -(g/L)*sin(theta_y) - (a_y/L)*cos(theta_y) - c*theta_y'

Cable Tension:
T = m * (g*cos(theta) - a_z + L*omega^2)

Natural Frequency:
f = (1/2*pi) * sqrt(g/L)
For L = 5m: f = 0.22 Hz (4.5 second period)'''
    add_code_block(doc, pend_eq, 10)

    doc.add_page_break()

    doc.add_heading('2.5 Inter-Processor Communication', level=2)
    doc.add_paragraph(
        'Processors communicate through the platform\'s AuxData container, a typed key-value '
        'store accessible from both C++ and scenario scripts.'
    )

    doc.add_heading('2.5.1 6DOF Mover Outputs', level=3)
    mover_outputs = [
        ('6dof_pos_n', 'double', 'm', 'North position in NED frame'),
        ('6dof_pos_e', 'double', 'm', 'East position in NED frame'),
        ('6dof_pos_d', 'double', 'm', 'Down position in NED frame'),
        ('6dof_vel_n', 'double', 'm/s', 'North velocity'),
        ('6dof_vel_e', 'double', 'm/s', 'East velocity'),
        ('6dof_vel_d', 'double', 'm/s', 'Down velocity'),
        ('6dof_roll_deg', 'double', 'deg', 'Roll angle'),
        ('6dof_pitch_deg', 'double', 'deg', 'Pitch angle'),
        ('6dof_yaw_deg', 'double', 'deg', 'Yaw/heading angle'),
        ('6dof_total_thrust', 'double', 'N', 'Total thrust from all rotors'),
        ('6dof_total_power', 'double', 'W', 'Total mechanical power'),
        ('6dof_throttle_cmd', 'double', '0-1', 'Throttle command'),
    ]
    add_table_with_header(doc, ['Key', 'Type', 'Units', 'Description'], mover_outputs)

    doc.add_heading('2.5.2 Battery Processor Outputs', level=3)
    batt_outputs = [
        ('BATTERY_SOC', 'double', '0-1', 'State of charge'),
        ('BATTERY_POWER_W', 'double', 'W', 'Current power consumption'),
        ('BATTERY_ENERGY_WH', 'double', 'Wh', 'Remaining energy'),
        ('BATTERY_VOLTAGE', 'double', 'V', 'Terminal voltage under load'),
        ('BATTERY_OCV', 'double', 'V', 'Open-circuit voltage'),
        ('BATTERY_VOLTAGE_SAG', 'double', 'V', 'IR voltage drop'),
        ('BATTERY_CURRENT_A', 'double', 'A', 'Current draw'),
        ('BATTERY_LOW', 'bool', '-', 'Low battery warning flag'),
        ('BATTERY_VOLTAGE_LOW', 'bool', '-', 'Voltage sag warning flag'),
    ]
    add_table_with_header(doc, ['Key', 'Type', 'Units', 'Description'], batt_outputs)

    doc.add_heading('2.6 Design Decisions', level=2)

    doc.add_heading('2.6.1 Why Custom 6DOF Mover Instead of WSF_FULL_ROTORCRAFT_MOVER', level=3)
    mover_comparison = [
        ('Dynamics', 'Table-driven kinematic', 'Physics-based dynamic'),
        ('Attitude', 'Euler angles', 'Quaternions (no gimbal lock)'),
        ('Power', 'Static tables', 'Real-time computation'),
        ('Transients', 'Simplified', 'Full acceleration modeling'),
        ('Customization', 'Limited', 'Fully configurable'),
    ]
    add_table_with_header(doc, ['Aspect', 'WSF_FULL_ROTORCRAFT_MOVER', 'Rotorcraft6DOF_Mover'], mover_comparison)

    doc.add_paragraph()
    doc.add_paragraph('The custom mover was necessary because:')
    reasons = [
        'Power consumption depends on actual flight dynamics, not table lookups',
        'Battery modeling requires accurate transient power during maneuvers',
        'Research applications need modifiable physics parameters',
    ]
    for item in reasons:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_heading('2.6.2 Why Semi-Implicit Euler Integration', level=3)
    doc.add_paragraph(
        'The 6DOF mover and slung load use semi-implicit (symplectic) Euler:'
    )
    euler_code = '''velocity_new = velocity_old + acceleration * dt;  // Update velocity first
position_new = position_old + velocity_new * dt;  // Use NEW velocity'''
    add_code_block(doc, euler_code, 10)

    doc.add_paragraph()
    doc.add_paragraph('Benefits over explicit Euler:')
    euler_benefits = [
        'Energy-conserving for oscillatory systems (pendulum, rotational dynamics)',
        'Stable for stiff systems without requiring tiny timesteps',
        'Simple to implement and debug',
    ]
    for item in euler_benefits:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_page_break()

    # =========================================================================
    # SECTION 3: 6DOF ROTORCRAFT MOVER
    # =========================================================================
    doc.add_heading('3. 6DOF Rotorcraft Mover', level=1)

    doc.add_heading('3.1 Physics Engine Overview', level=2)
    doc.add_paragraph(
        'The 6DOF mover implements a complete physics-based simulation of rotorcraft flight dynamics.'
    )

    doc.add_paragraph('Update Cycle (per timestep):')
    update_steps = [
        'Read target position from AFSIM route',
        'Run position controller -> desired velocity',
        'Run velocity controller -> desired tilt angles',
        'Run attitude controller -> motor commands',
        'Compute rotor thrust and torque',
        'Apply forces and moments to rigid body',
        'Integrate rigid body dynamics',
        'Sync state back to AFSIM platform',
    ]
    for i, step in enumerate(update_steps, 1):
        doc.add_paragraph(f'{i}. {step}')

    doc.add_heading('3.2 Rigid Body State', level=2)
    state_vars = [
        ('mPosition', 'Vec3', 'NED position [m]'),
        ('mVelocity', 'Vec3', 'NED velocity [m/s]'),
        ('mAttitude', 'Quaternion', 'Body-to-NED rotation'),
        ('mAngularVel', 'Vec3', 'Body angular velocity [rad/s]'),
    ]
    add_table_with_header(doc, ['Variable', 'Type', 'Description'], state_vars)

    doc.add_paragraph()
    doc.add_paragraph('Quaternion Convention:')
    quat_code = '''// q = [w, x, y, z] = [cos(theta/2), sin(theta/2)*axis]
// Rotation from body to NED: v_ned = q * v_body * q^(-1)'''
    add_code_block(doc, quat_code, 10)

    doc.add_heading('3.3 Rotor Configuration', level=2)
    doc.add_paragraph('Coaxial Octocopter Layout (view from above):')
    rotor_layout = '''        Front
          ^
          |
    R1    |    R2        Upper rotors (CW, CCW alternating)
     \\    |    /
      \\   |   /
       \\  |  /
        \\ | /
   R8----[CG]----R3
        / | \\
       /  |  \\
      /   |   \\
     /    |    \\
    R7    |    R6        Lower rotors (CCW, CW alternating)
          |
          v
        Rear

    R4 and R5 at rear positions'''
    add_code_block(doc, rotor_layout, 9)

    rotor_params = [
        ('0', '(+0.5, 0, 0)', 'CW', 'Upper'),
        ('1', '(+0.5, 0, 0)', 'CCW', 'Lower'),
        ('2', '(0, +0.5, 0)', 'CCW', 'Upper'),
        ('3', '(0, +0.5, 0)', 'CW', 'Lower'),
        ('4', '(-0.5, 0, 0)', 'CW', 'Upper'),
        ('5', '(-0.5, 0, 0)', 'CCW', 'Lower'),
        ('6', '(0, -0.5, 0)', 'CCW', 'Upper'),
        ('7', '(0, -0.5, 0)', 'CW', 'Lower'),
    ]
    add_table_with_header(doc, ['Rotor', 'Position [m]', 'Direction', 'Level'], rotor_params)

    doc.add_heading('3.4 Flight Control System', level=2)
    doc.add_paragraph('Controller Cascade:')
    controller_diagram = '''                    +-------------------+
  Waypoint -------->| Position Hold     |-------> Desired Velocity
                    | Kp=1.0, Kd=0.1    |
                    +-------------------+
                             |
                             v
                    +-------------------+
                    | Velocity Control  |-------> Desired Tilt
                    | Kp=3.0            |
                    +-------------------+
                             |
                             v
                    +-------------------+
                    | Attitude Control  |-------> Motor Torques
                    | Kp=8.0, Kd=2.0    |
                    +-------------------+
                             |
                             v
                    +-------------------+
                    | Altitude PID      |-------> Throttle
                    | Kp=0.3, Ki=0.05   |
                    | Kd=1.2            |
                    +-------------------+'''
    add_code_block(doc, controller_diagram, 8)

    doc.add_heading('3.5 AFSIM Integration', level=2)
    doc.add_paragraph('Coordinate Conversion (AFSIM uses geodetic, physics uses NED):')
    coord_code = '''void Rotorcraft6DOF_Mover::GeodeticToNED(
    double lat, double lon, double alt,
    double refLat, double refLon, double refAlt,
    double& north, double& east, double& down)
{
    const double R_EARTH = 6371000.0;  // meters
    const double DEG_TO_RAD = M_PI / 180.0;

    // CRITICAL: AFSIM provides degrees, not radians!
    north = (lat - refLat) * DEG_TO_RAD * R_EARTH;
    east = (lon - refLon) * DEG_TO_RAD * R_EARTH * cos(refLat * DEG_TO_RAD);
    down = -(alt - refAlt);
}'''
    add_code_block(doc, coord_code, 9)

    doc.add_page_break()

    # =========================================================================
    # SECTION 4: VALIDATION AND TESTING
    # =========================================================================
    doc.add_heading('4. Validation and Testing', level=1)

    doc.add_heading('4.1 Phase 1 Validation Suite', level=2)
    doc.add_paragraph('Test Date: January 14-15, 2026')
    doc.add_paragraph('Result: All 10 tests PASSED')

    doc.add_heading('4.1.1 6DOF Physics Tests (Tests 01-05)', level=3)
    physics_tests = [
        ('01', 'Motor Step Response', '30s', 'PASS', 'Throttle dynamics functional'),
        ('02', 'Attitude Recovery', '60s', 'PASS', '90 deg turns stable, no oscillation'),
        ('03', 'Position Hold', '120s', 'PASS', 'Velocity ~1e-17 m/s (essentially zero)'),
        ('04', 'Velocity Step', '60s', 'PASS', 'Achieved 14.49/15.0 m/s target'),
        ('05', 'Extreme Maneuvers', '120s', 'PASS', 'No NaN, numerically stable'),
    ]
    add_table_with_header(doc, ['Test', 'Name', 'Duration', 'Result', 'Key Observation'], physics_tests)

    doc.add_heading('4.1.2 Battery Integration Tests (Tests 06-10)', level=3)
    battery_tests = [
        ('06', 'Voltage Sag', '300s', 'PASS', '30%->0% SOC tracking correct'),
        ('07', 'Ground Effect', '120s', 'PASS', 'Power reduction near ground'),
        ('08', 'Coaxial Efficiency', '60s', 'PASS', '~20 kW mechanical hover power'),
        ('09', 'Energy Conservation', '300s', 'PASS', '< 0.01% energy balance error'),
        ('10', 'Endurance', '858s', 'PASS', '14.3 min flight time'),
    ]
    add_table_with_header(doc, ['Test', 'Name', 'Duration', 'Result', 'Key Observation'], battery_tests)

    doc.add_heading('4.2 Flight Pattern Analysis', level=2)
    doc.add_paragraph('5 flight patterns tested to validate dynamic power model:')

    pattern_results = [
        ('Straight', '17.0 kW', 'Baseline', 'Steady cruise, minimal maneuvering'),
        ('Sawtooth', '16.8 kW', '-1%', 'Gentle zigzag, brief turns'),
        ('Square', '21.7 kW', '+28%', '90 deg turns, moderate banking'),
        ('Cycloid', '24.1 kW', '+42%', 'Continuous loops, sustained banking'),
        ('Sine', '42.3 kW', '+149%', 'Aggressive oscillations, max bank'),
    ]
    add_table_with_header(doc, ['Pattern', 'Avg Power', 'vs Baseline', 'Notes'], pattern_results)

    doc.add_paragraph()
    p = doc.add_paragraph()
    p.add_run('Key Finding: ').bold = True
    p.add_run('Aggressive maneuvering consumes 2.5x more power than straight-line cruise.')

    doc.add_heading('4.3 Bug Fixes and Calibration', level=2)

    doc.add_heading('4.3.1 Motor Efficiency Calibration', level=3)
    doc.add_paragraph(
        'Problem: Initial motor efficiency of 37% resulted in excessive power consumption '
        'and only 6.8 min flight time.'
    )
    doc.add_paragraph('Solution: Updated to 85% efficiency based on typical brushless motor/ESC performance.')

    efficiency_impact = [
        ('Motor efficiency', '37%', '85%', '+48%'),
        ('Average power', '61.6 kW', '28.2 kW', '-54%'),
        ('Flight time', '6.8 min', '14.3 min', '+110%'),
    ]
    add_table_with_header(doc, ['Metric', 'Before', 'After', 'Change'], efficiency_impact)

    doc.add_heading('4.3.2 SOC Initialization Bug', level=3)
    doc.add_paragraph(
        'Problem: When starting with non-100% SOC, mEnergyConsumed was initialized to 0, '
        'causing SOC to jump to ~100% on first update.'
    )
    soc_fix = '''// Before:
mEnergyConsumed = 0.0;

// After:
mEnergyConsumed = (1.0 - mInitialSOC) * mBatteryCapacityWh;'''
    add_code_block(doc, soc_fix, 10)

    doc.add_page_break()

    # =========================================================================
    # SECTION 5: INSTALLATION AND USAGE
    # =========================================================================
    doc.add_heading('5. Installation and Usage', level=1)

    doc.add_heading('5.1 Prerequisites', level=2)
    prereqs = [
        ('AFSIM', '2.2505.x LTS', 'Installed to /opt/afsim2.2505 or similar'),
        ('CMake', '3.16+', 'Cross-platform build system'),
        ('GCC/G++', '11+', 'C++17 support required'),
        ('Linux', 'AlmaLinux 9 / Ubuntu 22.04+', 'Or Windows with Visual Studio'),
    ]
    add_table_with_header(doc, ['Requirement', 'Version', 'Notes'], prereqs)

    doc.add_heading('5.2 Building the Plugins', level=2)
    build_code = '''# Clone the repository
git clone https://github.com/DSGarrett/project005_t150_uas.git
cd project005_t150_uas

# Build using NGIC script (auto-detects AFSIM)
./ngic_afsim_build.sh

# Or for debug build:
./ngic_afsim_build.sh debug'''
    add_code_block(doc, build_code, 10)

    doc.add_paragraph()
    doc.add_paragraph('Build Output:')
    output_code = '''plugins/build/lib/
|-- rotorcraft_6dof.so
|-- t150_battery_processor.so
|-- t150_propulsion.so
|-- t150_vrs.so
|__ t150_slung_load.so'''
    add_code_block(doc, output_code, 10)

    doc.add_heading('5.3 Installing to AFSIM', level=2)
    install_code = '''# Install plugins (requires sudo for system AFSIM)
./ngic_afsim_install.sh

# Verify installation
ls /opt/afsim2.2505/lib/wsf_plugins/*.so'''
    add_code_block(doc, install_code, 10)

    doc.add_paragraph(
        'The install script copies plugins to both /opt/afsim*/lib/wsf_plugins/ (standard) '
        'and /opt/afsim*/bin/mission_plugins/ (alternative).'
    )

    doc.add_heading('5.4 Adapting for Other Aircraft', level=2)
    doc.add_paragraph('To adapt the plugins for a different aircraft:')

    doc.add_heading('Step 1: Create Platform Definition', level=3)
    platform_code = '''platform_type MY_DRONE WSF_PLATFORM
    # Use the 6DOF mover
    mover Rotorcraft6DOF_Mover
        mass 50 kg                    # Your aircraft mass
        inertia_xx 1.5                # Moment of inertia
        inertia_yy 1.5
        inertia_zz 2.5

        rotor_configuration QUAD_X    # Or COAXIAL_OCTO, HEX_X, etc.
        rotor_diameter 0.5 m          # Propeller diameter
        arm_length 0.3 m              # Motor arm length
        max_rpm 8000                  # Maximum motor RPM

        # Controller gains (tune for your aircraft)
        altitude_kp 0.3
        altitude_ki 0.05
        altitude_kd 1.2
    end_mover

    # Battery processor
    processor BATTERY MY_BATTERY_PROCESSOR
        battery_capacity_wh 2000      # Your battery capacity
        nominal_voltage 22.2          # 6S LiPo
        motor_efficiency 0.85
        use_dynamic_power true
    end_processor
end_platform_type'''
    add_code_block(doc, platform_code, 9)

    doc.add_heading('Step 2: Tune Controller Gains', level=3)
    tuning_guide = [
        ('altitude_kp', '0.2', 'Too slow to climb -> increase'),
        ('altitude_ki', '0.02', 'Altitude drifts -> increase'),
        ('altitude_kd', '0.5', 'Oscillates -> increase'),
        ('position_kp', '0.5', 'Slow to reach waypoints -> increase'),
        ('velocity_kp', '2.0', 'Sluggish velocity response -> increase'),
        ('attitude_kp', '5.0', 'Slow attitude correction -> increase'),
        ('attitude_kd', '1.0', 'Attitude oscillation -> increase'),
    ]
    add_table_with_header(doc, ['Gain', 'Start Value', 'Adjust If...'], tuning_guide)

    doc.add_heading('Step 3: Validate', level=3)
    doc.add_paragraph('Run the Phase 1 test scenarios with your aircraft to verify:')
    validation_steps = [
        'Stable hover (position hold test)',
        'Accurate waypoint tracking',
        'Reasonable battery consumption',
    ]
    for item in validation_steps:
        doc.add_paragraph(item, style='List Bullet')

    doc.add_page_break()

    # =========================================================================
    # SECTION 6: SOURCES
    # =========================================================================
    doc.add_heading('6. Sources and References', level=1)

    sources = [
        ('AAIB Accident Investigation', 'Air Accidents Investigation Branch report on TRV-150 incident at South Scarle, Nottinghamshire, UK, 2023'),
        ('US Patent US20240262499A1', 'Battery topology and power management for heavy-lift UAS'),
        ('FAA Aircraft Registry', 'TRV-150 registration and configuration data'),
        ('T-Motor U15II Specifications', 'Motor performance data: KV100, 3.5kW max power'),
        ('AFSIM Documentation', 'AFSIM 2.2505 framework documentation'),
        ('Cheeseman & Bennett (1955)', '"The Effect of the Ground on a Helicopter Rotor in Forward Flight"'),
        ('Johnson, W. (1980)', 'Helicopter Theory, Princeton University Press'),
        ('Prouty, R. (1986)', 'Helicopter Performance, Stability, and Control'),
        ('Leishman, J.G. (2006)', 'Principles of Helicopter Aerodynamics'),
    ]

    for i, (title, desc) in enumerate(sources, 1):
        p = doc.add_paragraph()
        p.add_run(f'[{i}] {title}: ').bold = True
        p.add_run(desc)

    doc.add_page_break()

    # =========================================================================
    # SECTION 7: ACRONYMS
    # =========================================================================
    doc.add_heading('7. Acronyms and Abbreviations', level=1)

    acronyms = [
        ('ADD', 'Architecture Description Document'),
        ('AFSIM', 'Advanced Framework for Simulation, Integration, and Modeling'),
        ('AGL', 'Above Ground Level'),
        ('CG', 'Center of Gravity'),
        ('CW/CCW', 'Clockwise / Counter-Clockwise'),
        ('DOF', 'Degree of Freedom'),
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
    output_path = '/home/shepard/afsim_projects/project005_t150_uas/VandV_docs/T150_UAS_ADD_v2.0.docx'
    doc.save(output_path)
    print(f'ADD v2.0 document created: {output_path}')
    return output_path


if __name__ == '__main__':
    create_add_document()
