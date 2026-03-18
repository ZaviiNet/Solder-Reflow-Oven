# Preset Profiles - Quick Start Guide

## What's New?

Your reflow oven controller now has **smart preset profiles** to prevent component damage!

## The Problem You Experienced

**Before**: One-size-fits-all lead-free profile at high temperature → melted plastic 40-pin header 😢

**Analysis**:
- Your solder paste melted at 160°C ✓ (correct - SAC305 starts melting ~217°C)
- Peak was likely 230-250°C (standard lead-free)
- **Problem**: Either component wasn't rated for 260°C OR time at peak exceeded 60 seconds
- **Result**: Plastic deformed/melted

**Now**: Multiple presets based on JEDEC standards → perfect reflows! ✅

## Quick Start

### 1. Access Web Interface
- Connect to WiFi: `ReflowOven-Setup` or your configured network
- Open browser: `http://192.168.4.1` (AP mode) or your device IP

### 2. Select a Preset

In the **"Reflow Profile Configuration"** section:

```
📋 Profile Preset: [▼ Dropdown Menu]
```

**For your 40-pin header boards**, select:
- **"Lead-Free Safe"** - Peak temp 235°C (conservative, safe for rated plastic headers)
- **"Sensitive Components"** - Peak temp 210°C (if header is non-rated or cheap plastic)

Other options:
- **"Lead-Free Standard"** - 245°C (JEDEC compliant, for rated components)
- **"Leaded Solder"** - 220°C (traditional leaded solder paste)
- **"High Mass Boards"** - 250°C (thick PCBs, maximum JEDEC peak)

### Why 235°C for Standard Headers?

Based on JEDEC J-STD-020 standards:
- Most SMD plastic connectors rated to **260°C**
- Safety margin: **15°C below rated max** = 245°C
- Conservative approach: **25°C below max** = **235°C** ← Safe choice!
- Time at peak: **40 seconds** (within 30-60s safe range)

### Why 210°C for Unknown Headers?

For non-rated or budget components:
- Unknown plastic material tolerance
- May not be rated for high-temp reflow
- 210°C provides large safety margin
- Still adequate for low-temp lead-free pastes

### 3. Save Your Own

After finding the perfect settings:
1. Enter name: e.g., "My 40-pin Boards"
2. Click **Save** button
3. Add description: "Custom low-temp for headers"
4. Done! Now available in dropdown

## Temperature Recommendations

| Component Type | Component Rating | Recommended Preset | Peak Temp |
|----------------|------------------|-------------------|-----------|
| Rated SMD Connectors | 260°C (JEDEC MSL 3) | Lead-Free Standard | 245°C |
| Standard Plastic Headers | 250°C typical | Lead-Free Safe | 235°C |
| Unknown/Budget Headers | Unknown | Sensitive Components | 210°C |
| Leaded Solder Paste | N/A | Leaded Solder | 220°C |
| High-temp Components Only | 260°C+ | High Mass Boards | 250°C |

### Time at Temperature Guidelines

⚠️ **Critical**: Time at peak temperature should not exceed **30-60 seconds**
- **Too long**: Browning, deformation, melting
- **Too short**: Incomplete wetting, poor joints
- **Our presets**: 35-50 seconds (safe range)

## Visual Guide

### Before Loading Preset:
```
Preheat Temp (°C): [150]
Soak Temp (°C):    [180]
Reflow Temp (°C):  [245] ← May be too hot for non-rated parts! 🔥
Reflow Time (sec): [40 ]
```

### After Loading "Lead-Free Safe":
```
Preheat Temp (°C): [145]
Soak Temp (°C):    [175]
Reflow Temp (°C):  [235] ← Safer for standard headers! ✓
Reflow Time (sec): [40 ] ← Within 30-60s safe range ✓
```

### Or "Sensitive Components" for Unknown Headers:
```
Preheat Temp (°C): [130]
Soak Temp (°C):    [160]
Reflow Temp (°C):  [210] ← Very conservative! ✓✓
Reflow Time (sec): [35 ] ← Shorter duration ✓
```

## Pro Tips

✅ **DO**:
- Use "Lead-Free Low-Temp" for boards with plastic parts
- Test with a sacrificial board first
- Save successful profiles as presets
- Monitor the temperature graph during reflow

❌ **DON'T**:
- Use "High Mass Boards" preset on small, thin PCBs
- Exceed component manufacturer's max temperature specs
- Forget to save your custom settings as a preset

## API Quick Reference

Load a preset programmatically:
```bash
curl -X POST http://192.168.4.1/api/presets/load \
  -H "Content-Type: application/json" \
  -d '{"name": "Lead-Free Low-Temp"}'
```

Get all presets:
```bash
curl http://192.168.4.1/api/presets
```

## Next Steps

1. **Test the "Lead-Free Low-Temp" preset** on your header boards
2. **Fine-tune if needed** (maybe 205°C works better?)
3. **Save as custom preset** "40-Pin Headers - 205C"
4. **Share your findings** with the community!

## Need Help?

- 📖 Full documentation: `PRESET_PROFILES_FEATURE.md`
- 🐛 Issues: Check serial console for errors
- 💡 Questions: See main `README.md`

---

**Built for**: Solder Reflow Oven Controller v2.0
**Date**: March 2026
**Feature**: Intelligent Profile Presets

