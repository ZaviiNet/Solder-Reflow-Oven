# Reflow Temperature Standards and Guidelines

## Industry Standards Reference

### JEDEC J-STD-020 Compliance

This document outlines the technical basis for the preset temperature profiles based on industry standards.

## Temperature Ranges by Solder Type

### Lead-Free Solder (SAC305)

**Standard Range**: 240°C to 250°C peak
**Maximum**: 260°C (short duration only)
**Recommended**: 245°C for safety margin

```
Temperature Profile (JEDEC J-STD-020):
┌─────────────────────────────────────────┐
│ Preheat:  150°C - 180°C (60-180s)       │
│ Soak:     180°C - 200°C (60-120s)       │
│ Reflow:   245°C - 250°C (30-60s) ⚠️     │
│ Peak:     260°C MAX (10s absolute max)  │
└─────────────────────────────────────────┘
```

**Critical Constraint**: Time above 220°C should be minimized (typically 40-60 seconds total)

### Leaded Solder (Sn63/Pb37)

**Standard Range**: 210°C to 230°C peak
**Recommended**: 220°C typical

```
Temperature Profile:
┌─────────────────────────────────────────┐
│ Preheat:  120°C - 150°C (60-120s)       │
│ Soak:     150°C - 180°C (60-120s)       │
│ Reflow:   210°C - 230°C (30-60s)        │
│ Peak:     230°C typical                 │
└─────────────────────────────────────────┘
```

## Plastic Component Ratings

### SMD Plastic Connectors/Headers

**Rating**: Most surface-mount plastic connectors are designed to survive temperatures up to **260°C** per JEDEC J-STD-020 standards.

**Material Types**:
- **High-Temp Nylon**: Rated to 260°C (MSL 3 compliant)
- **Glass-Filled Plastic**: Rated to 260°C
- **Standard Nylon**: Rated to 245°C
- **Cheap/Non-Rated Plastics**: May fail at 200-220°C ⚠️

### Critical Time Constraints

**Duration at Peak Temperature**:
- **30-60 seconds maximum** at peak temperature
- Exceeding this causes:
  - Browning/discoloration
  - Deformation/warping
  - Mechanical stress
  - Pin misalignment

**Time Above Liquidus** (SAC305 ~217°C):
- Should not exceed 60-90 seconds total
- Our default reflow time: 40 seconds ✓

## Preset Mapping to Standards

### Updated Preset Justifications

| Preset Name | Peak Temp | Justification | Best For |
|-------------|-----------|---------------|----------|
| **Lead-Free Standard** | 245°C | JEDEC compliant, standard SMD components rated 260°C | Normal production with rated components |
| **Lead-Free Safe** | 235°C | Conservative approach, 10°C below JEDEC peak | Standard plastic headers (40-pin, etc.) |
| **Leaded Solder** | 220°C | Center of Sn63/Pb37 range (210-230°C) | Traditional leaded solder paste |
| **Sensitive Components** | 210°C | For non-rated or cheap plastics | Budget connectors, unknown rating |
| **High Mass Boards** | 250°C | Maximum JEDEC lead-free peak | Thick copper, large ground planes |

## User's Original Problem - Analysis

### Issue Reported
- PCB with 40-pin header
- Solder paste started melting at 160°C ✓ (correct behavior)
- Reflow peak was 230°C (standard lead-free)
- Result: Header melted

### Root Cause Analysis

**Possible Causes**:
1. **Non-rated plastic**: Header not rated for 260°C reflow
2. **Time at temperature**: Possibly exceeded 60 seconds at peak
3. **Cheap material**: Budget header using low-temp plastic
4. **IR hotspot**: Uneven heating with IR element too close

### Recommended Solutions

**Option 1: Use Lead-Free Safe Preset** (235°C)
- 25°C below JEDEC maximum (260°C)
- Still adequate for SAC305 solder (melts at 217°C)
- Provides 18°C above liquidus point
- Duration: 40 seconds at peak ✓

**Option 2: Use Sensitive Components Preset** (210°C)
- For non-rated or cheap plastic headers
- Still 10°C margin above leaded solder range
- Works with low-temp lead-free pastes
- Most conservative approach

**Option 3: Switch to Leaded Solder** (220°C)
- If regulations permit
- Much lower thermal stress
- Header will survive easily
- Traditional hobby approach

## Temperature vs. Time Trade-offs

### Higher Temperature, Shorter Time
```
250°C for 30s:
  Pros: Better wetting, less oxidation
  Cons: More thermal stress, risk to plastics
```

### Lower Temperature, Longer Time
```
235°C for 45s:
  Pros: Less thermal stress, safer for plastics
  Cons: Possible incomplete reflow, more oxidation
```

**Recommended**: **245°C for 40s** (our "Lead-Free Standard" preset)
- Balances good wetting with component safety
- JEDEC compliant
- Works with rated SMD components

## Warning Signs of Thermal Damage

### Visual Indicators

1. **Browning**: Plastic turns brown/yellow
   - Cause: Exceeded rated temperature or time
   - Solution: Reduce peak temp by 10-15°C

2. **Deformation**: Warping or melting
   - Cause: Significantly exceeded rating
   - Solution: Use "Sensitive Components" preset

3. **Pin Misalignment**: Pins no longer straight
   - Cause: Plastic softened at peak temp
   - Solution: Reduce time at peak, use lower temp preset

4. **Cracking**: Stress cracks in plastic
   - Cause: Thermal shock or extended time
   - Solution: Slower ramp rates, lower peak

## Best Practices

### For Unknown Component Ratings

1. **Start conservative**: Use "Sensitive Components" preset (210°C)
2. **Visual inspection**: Check for browning after first reflow
3. **Measure joint quality**: Verify solder wetting is adequate
4. **Iterate up**: If joints are poor, increase by 5-10°C
5. **Document**: Save successful profile as custom preset

### For Known Rated Components

1. **Check datasheet**: Verify MSL rating and max reflow temp
2. **Use standard preset**: "Lead-Free Standard" (245°C) for rated parts
3. **Monitor time**: Keep peak duration to 40-50 seconds
4. **Verify profile**: Use thermocouple on actual board

### For Mixed Assemblies

1. **Identify weakest link**: Lowest-rated component sets the limit
2. **Use conservative preset**: Base on lowest rating
3. **Consider two-pass**: High-temp SMD first, low-temp headers second
4. **Alternative**: Hand-solder sensitive parts after reflow

## Troubleshooting Guide

### Problem: Solder Not Fully Reflowed

**Symptoms**: Dull joints, incomplete wetting
**Possible Causes**:
- Peak temperature too low (<235°C for lead-free)
- Time at peak too short (<30s)
- Poor thermal transfer (board not level, cold spots)

**Solutions**:
- Increase peak temp by 5-10°C
- Extend reflow time to 45-50s
- Verify board is level and well-supported

### Problem: Components Brown or Deform

**Symptoms**: Discoloration, warping, melting
**Possible Causes**:
- Peak temperature too high (>250°C)
- Time at peak too long (>60s)
- Component not rated for reflow temperature

**Solutions**:
- Reduce peak temp to 235°C or lower
- Reduce reflow time to 35-40s
- Use "Sensitive Components" or "Leaded Solder" preset
- Verify component reflow rating in datasheet

### Problem: Inconsistent Results

**Symptoms**: Some boards good, some have damage
**Possible Causes**:
- Temperature calibration drift
- Uneven heating in oven
- Varying component suppliers

**Solutions**:
- Calibrate thermocouple
- Run PID auto-tune
- Check oven heating element placement
- Qualify components from each supplier

## Safety Margins

### Recommended Margins from Maximum Ratings

| Component Rating | Use Peak Temp | Safety Margin |
|------------------|---------------|---------------|
| 260°C (JEDEC MSL 3) | 245°C | 15°C ✓ |
| 250°C (Standard) | 235°C | 15°C ✓ |
| 240°C (Budget) | 225°C | 15°C ✓ |
| Unknown/Suspect | 210°C | Conservative |

**Why 15°C margin?**
- Allows for oven temperature variation (±5°C)
- Accounts for thermocouple placement errors
- Provides buffer for component batch variation
- Prevents damage from minor PID overshoot

## References

- **JEDEC J-STD-020E**: "Moisture/Reflow Sensitivity Classification for Nonhermetic Solid State Surface Mount Devices"
- **IPC-7530**: "Temperature Profiling for Mass Soldering (Reflow and Wave) Processes"
- **SAC305 Alloy Datasheet**: Sn96.5/Ag3.0/Cu0.5 (Tm = 217°C)
- **Sn63/Pb37 Alloy**: Eutectic composition (Tm = 183°C)

## Conclusion

The preset system is designed with industry standards and safety margins in mind:

✅ **Lead-Free Standard (245°C)**: JEDEC compliant, 15°C below maximum
✅ **Lead-Free Safe (235°C)**: Conservative for standard plastic headers
✅ **Sensitive Components (210°C)**: For non-rated or cheap plastics
✅ **Leaded Solder (220°C)**: Center of industry range
✅ **High Mass Boards (250°C)**: Maximum recommended peak

**For your 40-pin header issue**: Start with **"Lead-Free Safe" (235°C)** or **"Sensitive Components" (210°C)** preset.

---

**Document Version**: 1.0
**Last Updated**: March 18, 2026
**Standards Referenced**: JEDEC J-STD-020E, IPC-7530

