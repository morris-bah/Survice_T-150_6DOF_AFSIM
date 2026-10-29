#!/usr/bin/env python3
"""
Generate T-150 UAS Plugin Interaction Diagram
Creates a visual block diagram showing how the 5 plugins interact.

Requirements: pip install matplotlib

Usage: python3 generate_plugin_diagram.py
Output: plugin_interaction_diagram.png
"""

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch
import matplotlib.lines as mlines

def create_plugin_diagram():
    # Create figure with larger size for detail
    fig, ax = plt.subplots(1, 1, figsize=(16, 12))
    ax.set_xlim(0, 16)
    ax.set_ylim(0, 12)
    ax.set_aspect('equal')
    ax.axis('off')

    # Colors
    MOVER_COLOR = '#4A90D9'      # Blue for 6DOF mover
    BATTERY_COLOR = '#50C878'    # Green for battery
    PROPULSION_COLOR = '#FFB347' # Orange for propulsion
    VRS_COLOR = '#FF6B6B'        # Red for VRS
    SLUNG_COLOR = '#DDA0DD'      # Purple for slung load
    AUXDATA_COLOR = '#F0E68C'    # Yellow for AuxData
    AFSIM_COLOR = '#E8E8E8'      # Light gray for AFSIM frame
    SCRIPT_COLOR = '#B8D4E3'     # Light blue for scripts

    # Draw AFSIM outer frame
    afsim_frame = FancyBboxPatch((0.3, 0.3), 15.4, 11.4,
                                  boxstyle="round,pad=0.05,rounding_size=0.3",
                                  facecolor=AFSIM_COLOR, edgecolor='#666666',
                                  linewidth=2, alpha=0.3)
    ax.add_patch(afsim_frame)
    ax.text(8, 11.5, 'AFSIM SIMULATION ENGINE', fontsize=14, fontweight='bold',
            ha='center', va='center')

    # Draw 6DOF Mover (top center) - the primary component
    mover_box = FancyBboxPatch((5.5, 8.5), 5, 2.2,
                                boxstyle="round,pad=0.05,rounding_size=0.2",
                                facecolor=MOVER_COLOR, edgecolor='#2C5F8A',
                                linewidth=2)
    ax.add_patch(mover_box)
    ax.text(8, 10.3, 'ROTORCRAFT_6DOF_MOVER', fontsize=11, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(8, 9.7, '(Physics Engine)', fontsize=9, ha='center', va='center', color='white')
    ax.text(8, 9.1, '• Rigid Body 6DOF  • 8 Rotor Models', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(8, 8.7, '• Flight Controllers  • Quaternion Attitude', fontsize=8,
            ha='center', va='center', color='white')

    # Draw AuxData layer (middle)
    auxdata_box = FancyBboxPatch((1, 5.8), 14, 1.4,
                                  boxstyle="round,pad=0.05,rounding_size=0.2",
                                  facecolor=AUXDATA_COLOR, edgecolor='#B8860B',
                                  linewidth=2)
    ax.add_patch(auxdata_box)
    ax.text(8, 6.8, 'PLATFORM AUXDATA (Key-Value Communication Layer)', fontsize=10,
            fontweight='bold', ha='center', va='center')
    ax.text(8, 6.2, '6dof_total_power | 6dof_vel_* | 6dof_pos_* | Platform Altitude | Platform Accel',
            fontsize=8, ha='center', va='center', family='monospace')

    # Draw the 4 processors (bottom row)
    proc_y = 3.0
    proc_height = 2.2
    proc_width = 3.2

    # Battery Processor
    battery_box = FancyBboxPatch((0.8, proc_y), proc_width, proc_height,
                                  boxstyle="round,pad=0.05,rounding_size=0.2",
                                  facecolor=BATTERY_COLOR, edgecolor='#228B22',
                                  linewidth=2)
    ax.add_patch(battery_box)
    ax.text(2.4, proc_y + 1.9, 'BATTERY', fontsize=10, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(2.4, proc_y + 1.5, 'PROCESSOR', fontsize=10, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(2.4, proc_y + 1.0, '• Li-Ion Chemistry', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(2.4, proc_y + 0.6, '• Voltage Sag Model', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(2.4, proc_y + 0.2, '• SOC Tracking', fontsize=8,
            ha='center', va='center', color='white')

    # Propulsion Processor
    prop_box = FancyBboxPatch((4.3, proc_y), proc_width, proc_height,
                               boxstyle="round,pad=0.05,rounding_size=0.2",
                               facecolor=PROPULSION_COLOR, edgecolor='#CC7722',
                               linewidth=2)
    ax.add_patch(prop_box)
    ax.text(5.9, proc_y + 1.9, 'PROPULSION', fontsize=10, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(5.9, proc_y + 1.5, 'PROCESSOR', fontsize=10, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(5.9, proc_y + 1.0, '• Motor Thermal', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(5.9, proc_y + 0.6, '• Ground Effect', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(5.9, proc_y + 0.2, '• Payload Mass', fontsize=8,
            ha='center', va='center', color='white')

    # VRS Processor
    vrs_box = FancyBboxPatch((7.8, proc_y), proc_width, proc_height,
                              boxstyle="round,pad=0.05,rounding_size=0.2",
                              facecolor=VRS_COLOR, edgecolor='#8B0000',
                              linewidth=2)
    ax.add_patch(vrs_box)
    ax.text(9.4, proc_y + 1.9, 'VRS', fontsize=10, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(9.4, proc_y + 1.5, 'PROCESSOR', fontsize=10, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(9.4, proc_y + 1.0, '• Descent Monitor', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(9.4, proc_y + 0.6, '• 5-State Machine', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(9.4, proc_y + 0.2, '• Lift Degradation', fontsize=8,
            ha='center', va='center', color='white')

    # Slung Load Processor
    slung_box = FancyBboxPatch((11.3, proc_y), proc_width, proc_height,
                                boxstyle="round,pad=0.05,rounding_size=0.2",
                                facecolor=SLUNG_COLOR, edgecolor='#8B008B',
                                linewidth=2)
    ax.add_patch(slung_box)
    ax.text(12.9, proc_y + 1.9, 'SLUNG LOAD', fontsize=10, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(12.9, proc_y + 1.5, 'PROCESSOR', fontsize=10, fontweight='bold',
            ha='center', va='center', color='white')
    ax.text(12.9, proc_y + 1.0, '• Pendulum Dynamics', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(12.9, proc_y + 0.6, '• Cable Tension', fontsize=8,
            ha='center', va='center', color='white')
    ax.text(12.9, proc_y + 0.2, '• Resonance Detect', fontsize=8,
            ha='center', va='center', color='white')

    # Output AuxData layer (bottom)
    output_box = FancyBboxPatch((1, 1.0), 14, 1.4,
                                 boxstyle="round,pad=0.05,rounding_size=0.2",
                                 facecolor=AUXDATA_COLOR, edgecolor='#B8860B',
                                 linewidth=2)
    ax.add_patch(output_box)
    ax.text(8, 2.0, 'AUXDATA OUTPUTS (Available to Scenario Scripts)', fontsize=10,
            fontweight='bold', ha='center', va='center')
    ax.text(8, 1.4, 'BATTERY_SOC | PROP_TOTAL_POWER | VRS_MARGIN | SLUNG_TENSION | + 50 more keys',
            fontsize=8, ha='center', va='center', family='monospace')

    # Draw arrows from 6DOF Mover to AuxData
    arrow_style = dict(arrowstyle='->', color='#333333', lw=2,
                       connectionstyle='arc3,rad=0')

    # Main output arrow from 6DOF to AuxData
    ax.annotate('', xy=(8, 7.2), xytext=(8, 8.5),
                arrowprops=dict(arrowstyle='->', color='#333333', lw=2.5))
    ax.text(8.3, 7.85, '6dof_total_power\n6dof_vel_*\n6dof_pos_*', fontsize=7,
            ha='left', va='center', family='monospace')

    # Arrows from AuxData to processors
    ax.annotate('', xy=(2.4, 5.2), xytext=(2.4, 5.8),
                arrowprops=dict(arrowstyle='->', color=BATTERY_COLOR, lw=2))
    ax.annotate('', xy=(5.9, 5.2), xytext=(5.9, 5.8),
                arrowprops=dict(arrowstyle='->', color=PROPULSION_COLOR, lw=2))
    ax.annotate('', xy=(9.4, 5.2), xytext=(9.4, 5.8),
                arrowprops=dict(arrowstyle='->', color=VRS_COLOR, lw=2))
    ax.annotate('', xy=(12.9, 5.2), xytext=(12.9, 5.8),
                arrowprops=dict(arrowstyle='->', color=SLUNG_COLOR, lw=2))

    # Arrows from processors to output AuxData
    ax.annotate('', xy=(2.4, 2.4), xytext=(2.4, 3.0),
                arrowprops=dict(arrowstyle='->', color=BATTERY_COLOR, lw=2))
    ax.annotate('', xy=(5.9, 2.4), xytext=(5.9, 3.0),
                arrowprops=dict(arrowstyle='->', color=PROPULSION_COLOR, lw=2))
    ax.annotate('', xy=(9.4, 2.4), xytext=(9.4, 3.0),
                arrowprops=dict(arrowstyle='->', color=VRS_COLOR, lw=2))
    ax.annotate('', xy=(12.9, 2.4), xytext=(12.9, 3.0),
                arrowprops=dict(arrowstyle='->', color=SLUNG_COLOR, lw=2))

    # Add legend
    legend_elements = [
        mpatches.Patch(facecolor=MOVER_COLOR, edgecolor='#2C5F8A', label='6DOF Mover (Physics)'),
        mpatches.Patch(facecolor=BATTERY_COLOR, edgecolor='#228B22', label='Battery Processor'),
        mpatches.Patch(facecolor=PROPULSION_COLOR, edgecolor='#CC7722', label='Propulsion Processor'),
        mpatches.Patch(facecolor=VRS_COLOR, edgecolor='#8B0000', label='VRS Processor'),
        mpatches.Patch(facecolor=SLUNG_COLOR, edgecolor='#8B008B', label='Slung Load Processor'),
        mpatches.Patch(facecolor=AUXDATA_COLOR, edgecolor='#B8860B', label='AuxData Layer'),
    ]
    ax.legend(handles=legend_elements, loc='upper left', fontsize=9,
              framealpha=0.9, bbox_to_anchor=(0.02, 0.98))

    # Title
    ax.text(8, 11.85, 'T-150 UAS Plugin Interaction Architecture',
            fontsize=16, fontweight='bold', ha='center', va='center')

    # Add data flow labels on the sides
    ax.text(0.5, 6.5, 'INPUTS', fontsize=9, fontweight='bold', rotation=90,
            ha='center', va='center')
    ax.text(0.5, 1.7, 'OUTPUTS', fontsize=9, fontweight='bold', rotation=90,
            ha='center', va='center')

    # Add update rates
    ax.text(15.3, 9.5, '20 Hz', fontsize=8, ha='right', va='center',
            color=MOVER_COLOR, fontweight='bold')
    ax.text(15.3, 4.1, '10 Hz', fontsize=8, ha='right', va='center',
            color='#666666', fontweight='bold')
    ax.text(15.3, 3.6, '(50 Hz for\nSlung Load)', fontsize=7, ha='right', va='center',
            color=SLUNG_COLOR)

    plt.tight_layout()

    # Save the figure
    output_path = '/home/shepard/afsim_projects/project005_t150_uas/docs/plugin_interaction_diagram.png'
    plt.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f'Diagram saved to: {output_path}')

    # Also save as PDF for high quality
    pdf_path = '/home/shepard/afsim_projects/project005_t150_uas/docs/plugin_interaction_diagram.pdf'
    plt.savefig(pdf_path, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f'PDF saved to: {pdf_path}')

    plt.close()
    return output_path


def create_data_flow_diagram():
    """Create a simpler data flow diagram showing just the dependencies"""
    fig, ax = plt.subplots(1, 1, figsize=(12, 8))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 8)
    ax.set_aspect('equal')
    ax.axis('off')

    # Colors
    MOVER_COLOR = '#4A90D9'
    BATTERY_COLOR = '#50C878'
    PROPULSION_COLOR = '#FFB347'
    VRS_COLOR = '#FF6B6B'
    SLUNG_COLOR = '#DDA0DD'

    # Draw 6DOF Mover at top
    mover = plt.Circle((6, 6.5), 0.8, color=MOVER_COLOR, ec='#2C5F8A', lw=2)
    ax.add_patch(mover)
    ax.text(6, 6.5, '6DOF\nMover', ha='center', va='center', fontsize=10,
            fontweight='bold', color='white')

    # Draw processors
    battery = plt.Circle((2.5, 3.5), 0.7, color=BATTERY_COLOR, ec='#228B22', lw=2)
    ax.add_patch(battery)
    ax.text(2.5, 3.5, 'Battery', ha='center', va='center', fontsize=10,
            fontweight='bold', color='white')

    propulsion = plt.Circle((5, 3.5), 0.7, color=PROPULSION_COLOR, ec='#CC7722', lw=2)
    ax.add_patch(propulsion)
    ax.text(5, 3.5, 'Propulsion', ha='center', va='center', fontsize=9,
            fontweight='bold', color='white')

    vrs = plt.Circle((7.5, 3.5), 0.7, color=VRS_COLOR, ec='#8B0000', lw=2)
    ax.add_patch(vrs)
    ax.text(7.5, 3.5, 'VRS', ha='center', va='center', fontsize=10,
            fontweight='bold', color='white')

    slung = plt.Circle((10, 3.5), 0.7, color=SLUNG_COLOR, ec='#8B008B', lw=2)
    ax.add_patch(slung)
    ax.text(10, 3.5, 'Slung\nLoad', ha='center', va='center', fontsize=9,
            fontweight='bold', color='white')

    # Draw arrows with labels
    arrow_props = dict(arrowstyle='->', color='#333333', lw=1.5,
                       connectionstyle='arc3,rad=0.1')

    # 6DOF to Battery (power)
    ax.annotate('', xy=(2.8, 4.1), xytext=(5.4, 5.9),
                arrowprops=dict(arrowstyle='->', color=BATTERY_COLOR, lw=2,
                               connectionstyle='arc3,rad=0.2'))
    ax.text(3.5, 5.3, 'power', fontsize=8, color=BATTERY_COLOR, style='italic')

    # 6DOF to Propulsion (altitude)
    ax.annotate('', xy=(5, 4.2), xytext=(5.7, 5.8),
                arrowprops=dict(arrowstyle='->', color=PROPULSION_COLOR, lw=2,
                               connectionstyle='arc3,rad=0'))
    ax.text(5.5, 5.0, 'altitude', fontsize=8, color=PROPULSION_COLOR, style='italic')

    # 6DOF to VRS (velocity)
    ax.annotate('', xy=(7.5, 4.2), xytext=(6.5, 5.8),
                arrowprops=dict(arrowstyle='->', color=VRS_COLOR, lw=2,
                               connectionstyle='arc3,rad=0'))
    ax.text(7.3, 5.0, 'velocity', fontsize=8, color=VRS_COLOR, style='italic')

    # 6DOF to Slung Load (acceleration)
    ax.annotate('', xy=(9.7, 4.1), xytext=(6.6, 5.9),
                arrowprops=dict(arrowstyle='->', color=SLUNG_COLOR, lw=2,
                               connectionstyle='arc3,rad=-0.2'))
    ax.text(8.8, 5.3, 'accel', fontsize=8, color=SLUNG_COLOR, style='italic')

    # Script output arrow
    ax.annotate('', xy=(6, 1.0), xytext=(6, 2.5),
                arrowprops=dict(arrowstyle='->', color='#333333', lw=2))

    # Script box
    script_box = FancyBboxPatch((3, 0.3), 6, 0.6,
                                 boxstyle="round,pad=0.05,rounding_size=0.1",
                                 facecolor='#B8D4E3', edgecolor='#666666', lw=1)
    ax.add_patch(script_box)
    ax.text(6, 0.6, 'Scenario Scripts (read AuxData)', ha='center', va='center',
            fontsize=9)

    # Output arrows from processors
    for x in [2.5, 5, 7.5, 10]:
        ax.annotate('', xy=(x, 2.0), xytext=(x, 2.8),
                    arrowprops=dict(arrowstyle='->', color='#666666', lw=1.5))

    # AuxData output bar
    auxdata_bar = FancyBboxPatch((1.5, 1.7), 9, 0.4,
                                  boxstyle="round,pad=0.02",
                                  facecolor='#F0E68C', edgecolor='#B8860B', lw=1)
    ax.add_patch(auxdata_bar)
    ax.text(6, 1.9, 'AuxData Outputs', ha='center', va='center', fontsize=8)

    # Title
    ax.text(6, 7.5, 'T-150 Plugin Data Flow', fontsize=14, fontweight='bold',
            ha='center', va='center')

    plt.tight_layout()

    # Save
    output_path = '/home/shepard/afsim_projects/project005_t150_uas/docs/plugin_data_flow.png'
    plt.savefig(output_path, dpi=150, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f'Data flow diagram saved to: {output_path}')
    plt.close()
    return output_path


if __name__ == '__main__':
    print("Generating T-150 UAS Plugin Interaction Diagrams...")
    print("=" * 50)
    create_plugin_diagram()
    create_data_flow_diagram()
    print("=" * 50)
    print("Done!")
