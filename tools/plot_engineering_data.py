#!/usr/bin/env python3
"""
T-150 UAS Engineering Data Plotter

This script reads the simulation log and plots engineering data.
It parses the AFSIM output looking for battery, thermal, and VRS events.

Usage:
    python3 plot_engineering_data.py output/02_capability_demo.log

Or run the simulation and pipe to this script:
    /opt/afsim2.2505/bin/mission scenarios/demos/02_capability_demo.afsim 2>&1 | python3 tools/plot_engineering_data.py -
"""

import sys
import re
import matplotlib
matplotlib.use('Agg')  # Use non-GUI backend for headless environments
import matplotlib.pyplot as plt
from collections import defaultdict

def parse_log(log_content):
    """Parse AFSIM log for engineering data"""
    data = {
        'time': [],
        'soc': [],
        'vrs_margin': [],
        'motor_temp': [],
        'events': []
    }

    # Pattern for time markers (T = XXX)
    time_pattern = re.compile(r'T = (\d+\.?\d*)')

    # Pattern for battery SOC and Time (multi-line: SOC on one line, Time on next)
    battery_soc_pattern = re.compile(r'SOC: (\d+)%')
    battery_time_pattern = re.compile(r'Time: (\d+)')

    # Pattern for VRS margin (standalone on its own line)
    vrs_margin_pattern = re.compile(r'VRS Margin: (\d+\.?\d*)')

    # Pattern for motor overtemp
    temp_pattern = re.compile(r'Motor (\d+) OVERTEMP.*Temperature: (\d+\.?\d*)')

    current_time = 0.0
    pending_vrs_state = None
    pending_battery_soc = None

    lines = log_content.split('\n')
    for i, line in enumerate(lines):
        # Track time
        time_match = time_pattern.search(line)
        if time_match:
            current_time = float(time_match.group(1))

        # Detect VRS states (multi-line: state on one line, margin on a following line)
        if 'VRS Caution' in line:
            pending_vrs_state = 'Caution'
        elif 'VRS ONSET' in line:
            pending_vrs_state = 'ONSET'
        elif 'VRS DEVELOPED' in line:
            pending_vrs_state = 'DEVELOPED'

        # VRS margin (appears after VRS state line)
        margin_match = vrs_margin_pattern.search(line)
        if margin_match:
            margin = float(margin_match.group(1))
            state = pending_vrs_state or 'Unknown'
            data['events'].append((current_time, f'VRS {state}: {margin:.2f}', 'vrs'))
            data['vrs_margin'].append((current_time, margin))
            pending_vrs_state = None

        # Battery warnings (multi-line format)
        if 'Low battery warning' in line:
            pending_battery_soc = None  # Reset

        soc_match = battery_soc_pattern.search(line)
        if soc_match:
            pending_battery_soc = float(soc_match.group(1))

        time_in_battery = battery_time_pattern.search(line)
        if time_in_battery and pending_battery_soc is not None:
            t = float(time_in_battery.group(1))
            data['events'].append((t, f'Battery: {pending_battery_soc}%', 'battery'))
            pending_battery_soc = None

        # Motor overtemp
        temp_match = temp_pattern.search(line)
        if temp_match:
            motor = temp_match.group(1)
            temp = float(temp_match.group(2))
            data['events'].append((current_time, f'Motor {motor}: {temp}C', 'thermal'))
            data['motor_temp'].append((current_time, temp))

    return data

def create_timeline_plot(data, output_file='t150_engineering_timeline.png'):
    """Create a timeline visualization of events"""

    fig, axes = plt.subplots(3, 1, figsize=(14, 10), sharex=True)
    fig.suptitle('T-150 UAS Engineering Data Timeline', fontsize=14, fontweight='bold')

    # Plot 1: VRS Margin over time
    ax1 = axes[0]
    if data['vrs_margin']:
        times, margins = zip(*data['vrs_margin'])
        ax1.scatter(times, margins, c='blue', s=50, alpha=0.7, label='VRS Margin')
        ax1.axhline(y=0.8, color='orange', linestyle='--', label='Caution Threshold')
        ax1.axhline(y=0.5, color='red', linestyle='--', label='Onset Threshold')
    ax1.set_ylabel('VRS Margin')
    ax1.set_ylim(0, 1.1)
    ax1.legend(loc='upper right')
    ax1.set_title('VRS Safety Margin (1.0 = Safe, 0.0 = In VRS)')
    ax1.grid(True, alpha=0.3)

    # Plot 2: Events timeline
    ax2 = axes[1]
    event_colors = {'battery': 'green', 'vrs': 'blue', 'thermal': 'red'}
    event_y = {'battery': 1, 'vrs': 2, 'thermal': 3}

    for t, label, event_type in data['events']:
        ax2.scatter(t, event_y[event_type], c=event_colors[event_type], s=100, zorder=5)
        ax2.annotate(label, (t, event_y[event_type]),
                    textcoords="offset points", xytext=(5, 5),
                    fontsize=8, rotation=45)

    ax2.set_ylabel('Event Type')
    ax2.set_yticks([1, 2, 3])
    ax2.set_yticklabels(['Battery', 'VRS', 'Thermal'])
    ax2.set_ylim(0.5, 3.5)
    ax2.set_title('Engineering Events')
    ax2.grid(True, alpha=0.3)

    # Plot 3: Battery SOC (inferred from theory)
    ax3 = axes[2]
    # Theoretical battery drain: 7000 Wh capacity, 18 kW hover
    end_time = 1500
    time_points = list(range(0, end_time, 10))
    soc_points = [max(0, 100 - (t * 18000 / 7000 / 36)) for t in time_points]  # % per second

    ax3.plot(time_points, soc_points, 'g-', linewidth=2, label='Battery SOC')
    ax3.axhline(y=20, color='red', linestyle='--', label='Low Battery Warning (20%)')
    ax3.set_ylabel('SOC (%)')
    ax3.set_xlabel('Time (seconds)')
    ax3.set_ylim(0, 105)
    ax3.legend(loc='upper right')
    ax3.set_title('Battery State of Charge')
    ax3.grid(True, alpha=0.3)

    # Add vertical line for low battery time (1120s predicted)
    for ax in axes:
        ax.axvline(x=1120, color='red', linestyle=':', alpha=0.5)

    plt.tight_layout()
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"Plot saved to: {output_file}")
    # plt.show()  # Commented out for headless environments

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 plot_engineering_data.py <log_file_or_-_for_stdin>")
        print("\nExample:")
        print("  /opt/afsim2.2505/bin/mission scenarios/demos/02_capability_demo.afsim 2>&1 | python3 tools/plot_engineering_data.py -")
        sys.exit(1)

    # Read input
    if sys.argv[1] == '-':
        log_content = sys.stdin.read()
    else:
        with open(sys.argv[1], 'r') as f:
            log_content = f.read()

    print("Parsing log data...")
    data = parse_log(log_content)

    print(f"Found {len(data['events'])} events")
    print(f"Found {len(data['vrs_margin'])} VRS margin readings")

    # Print event summary
    print("\n=== Event Summary ===")
    for t, label, event_type in sorted(data['events'], key=lambda x: x[0]):
        print(f"  T={t:7.1f}s: [{event_type:8s}] {label}")

    # Create plot
    output_file = 'output/t150_engineering_timeline.png'
    create_timeline_plot(data, output_file)

if __name__ == '__main__':
    main()
