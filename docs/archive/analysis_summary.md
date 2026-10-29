# T-150 Flight Demonstration Analysis Summary

## Executive Summary

This document summarizes the results of the T-150/TRV-150 Heavy-Lift UAS simulation, focusing on battery consumption analysis and route adherence performance.

---

## 1. Mission Profile

### 1.1 Flight Path Overview

| Phase | Description | Distance | Pattern Type |
|-------|-------------|----------|--------------|
| 1 | Takeoff & Climb | 500m vertical | Vertical ascent |
| 2 | Straight Line to Point A | 2.0 km | Direct flight |
| 3 | Sawtooth to Point B | ~3.15 km path | 8 zigzag segments, 200m amplitude |
| 4 | Square Wave to Point C | ~3.3 km path | 6 cycles, 200m amplitude |
| 5 | Sine Wave to Point D | ~2.7 km path | 2 complete cycles, 200m amplitude |
| 6 | Cycloid Loops to Point E | ~3.5 km path | 4 loops, ~100m radius |
| 7 | Return & Land | ~2.83 km + descent | Direct return with staged descent |

**Total Mission Distance**: ~17.5 km path length (~12 km straight-line)
**Mission Altitude**: 500m AGL
**Total Duration**: ~2000 seconds (~33 minutes)

---

## 2. Battery Consumption Analysis

### 2.1 Energy Model Parameters

| Parameter | Value | Source |
|-----------|-------|--------|
| Battery Capacity | 7000 Wh | Patent/Technical docs |
| Hover Power | 18 kW | Derived from MTOW/thrust |
| Cruise Power | 16-17 kW | Speed dependent |
| Climb Power | 22 kW | Maximum continuous |
| Descent Power | 12 kW | Reduced thrust requirement |

### 2.2 Phase-by-Phase Energy Consumption

| Phase | Energy (Wh) | Duration (sec) | Avg Power (kW) |
|-------|-------------|----------------|----------------|
| 1 - Takeoff/Climb | 611 | 100 | 22.0 |
| 2 - Straight Line | 630 | 133 | 17.0 |
| 3 - Sawtooth | 830 | 175 | 17.1 |
| 4 - Square Wave | 1604 | 330 | 17.5 |
| 5 - Sine Wave | 1031 | 225 | 16.5 |
| 6 - Cycloid Loops | 1369 | 290 | 17.0 |
| 7 - Return/Land | 1449 | 356 | 14.7 |
| **TOTAL** | **7524** | **1609** | **16.8 avg** |

### 2.3 Battery Status

```
Capacity:     7000 Wh
Energy Used:  7524 Wh
Deficit:       524 Wh (7.5% over capacity)
Final SOC:    -7.5% (mission exceeds capacity)
```

**Key Finding**: The mission as designed exceeds the T-150's battery capacity by approximately 7.5%. In real-world operations, this mission would require:
- Reduced pattern complexity
- Shorter distances between waypoints
- Lower cruise speeds
- Or mission segmentation with battery swap

---

## 3. Pattern Efficiency Analysis

### 3.1 Energy per Straight-Line Kilometer

This metric compares actual energy consumed to the straight-line distance traveled, revealing the "overhead" of each pattern type.

| Pattern | Path Distance | Straight-Line | Energy | Wh/km (path) | Wh/km (straight) | Overhead |
|---------|---------------|---------------|--------|--------------|------------------|----------|
| Straight Line | 2.0 km | 2.0 km | 630 Wh | 315 | 315 | Baseline |
| Sawtooth | 3.15 km | 2.0 km | 830 Wh | 264 | 415 | +32% |
| Square Wave | 3.3 km | 2.0 km | 1604 Wh | 486 | 802 | +155% |
| Sine Wave | 2.7 km | 2.0 km | 1031 Wh | 382 | 516 | +64% |
| Cycloid Loops | 3.5 km | 2.0 km | 1369 Wh | 391 | 685 | +117% |

### 3.2 Efficiency Rankings

**Most Efficient to Least Efficient (by straight-line Wh/km):**
1. **Straight Line**: 315 Wh/km (baseline)
2. **Sawtooth**: 415 Wh/km (+32%) - Sharp turns but direct reversals
3. **Sine Wave**: 516 Wh/km (+64%) - Smooth curves minimize acceleration losses
4. **Cycloid Loops**: 685 Wh/km (+117%) - Continuous turning with backtracking
5. **Square Wave**: 802 Wh/km (+155%) - Stop-and-go with 90-degree turns

### 3.3 Key Insights

1. **Square Wave Inefficiency**: The +155% overhead is due to:
   - Frequent speed changes (10 m/s vs 18 m/s for sawtooth)
   - 90-degree turns requiring full deceleration
   - Longer time at slower speeds

2. **Sine Wave Advantage**: Smooth curves allow:
   - Maintained forward momentum
   - Gradual heading changes
   - Lower acceleration/deceleration losses

3. **Sawtooth Efficiency**: Despite sharp reversals:
   - Higher sustained speed (18 m/s)
   - Shorter path segments
   - Quick direction changes

---

## 4. Route Adherence Analysis

### 4.1 Navigation Performance

The T-150 simulation demonstrated the following route-following characteristics:

| Behavior | Observation |
|----------|-------------|
| **Waypoint Capture** | Successful capture of all 67+ waypoints |
| **Altitude Holding** | Maintained 500m AGL throughout cruise phases |
| **Speed Control** | Achieved commanded speeds with minor overshoots |
| **Turn Performance** | Some overshoot on tight turns, self-correcting |
| **Takeoff** | Clean vertical climb from ground to altitude |
| **Landing** | Successful staged descent with minor overshoot correction |

### 4.2 Observed Behaviors

1. **Momentum Effects**: At higher speeds, the rotorcraft overshoots waypoints before correcting. This is realistic for a 123 kg platform.

2. **Deceleration Timing**: The tuned deceleration gains (1.8) provide adequate but not perfect approach to hover points.

3. **Pattern Fidelity**:
   - Sawtooth: Sharp corners slightly rounded
   - Square Wave: 90-degree corners achievable at 10 m/s
   - Sine Wave: Smooth curves well-matched
   - Cycloid: Loop shapes preserved with minor deviations

### 4.3 Landing Sequence

The staged descent approach works well:
1. Approach at cruise altitude (500m)
2. Begin descent at intermediate waypoint (100m)
3. Final touchdown with `stop` command

Some overshoot observed on final approach, requiring self-correction - this is expected behavior for switch_on_passing mode.

---

## 5. Validation Against T-150 Specifications

### 5.1 Performance Envelope Compliance

| Parameter | Spec Value | Simulated | Status |
|-----------|------------|-----------|--------|
| Max Cruise Speed | 30 m/s | 18 m/s max used | Within limits |
| Max Climb Rate | 5 m/s | 5 m/s | At limit |
| Max Descent Rate | 3-4 m/s | 3 m/s | Within limits |
| Operational Altitude | 500m AGL | 500m AGL | As designed |
| Flight Time | 36 min | ~33 min | Comparable |

### 5.2 Energy Consumption Realism

The simulated power draw aligns with estimates:
- **Hover**: 18 kW estimated vs ~15-20 kW typical for similar platforms
- **Cruise**: 17 kW estimated vs manufacturer's implied range/energy
- **Climb**: 22 kW estimated (maximum continuous)

---

## 6. Recommendations

### 6.1 For This Mission Profile
- Reduce one pattern segment to bring energy within capacity
- Or increase cruise speeds to reduce mission time (and energy)
- Consider battery capacity margins for safety

### 6.2 For Future Simulations
- Implement real-time energy tracking via C++ plugin
- Add wind effects to test energy consumption variance
- Model battery voltage sag at high current draws
- Include thermal effects on performance

### 6.3 For Real-World Operations
- Plan missions to 80% battery capacity (5600 Wh usable)
- Account for reserve requirements (typically 20%)
- Consider environmental factors (wind, temperature)

---

## 7. Conclusion

The T-150 simulation successfully demonstrates:
1. **Realistic flight dynamics** using WSF_FULL_ROTORCRAFT_MOVER
2. **Energy consumption tracking** revealing pattern efficiency differences
3. **Route adherence** with expected momentum-based behaviors
4. **Mission planning insights** showing this profile exceeds battery capacity

The most significant finding is the dramatic efficiency difference between patterns, with square wave consuming 2.5x more energy per kilometer than straight-line flight. This has direct implications for mission planning and route optimization for tactical resupply operations.
