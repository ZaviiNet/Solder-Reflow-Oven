# Reflow Profile Presets Feature

## Overview

The reflow oven controller now includes a **preset profile system** that allows you to save, load, and manage different temperature profiles for various types of boards and components. This prevents issues like melting plastic headers or other heat-sensitive components.

## Why Presets?

Different PCB assemblies have different requirements based on **industry standards (JEDEC J-STD-020)**:

- **Standard lead-free boards** can handle 245-250°C (components rated to 260°C)
- **Boards with plastic headers** need conservative temperatures (235°C) with rated components
- **Non-rated or cheap plastics** may fail even at 210-220°C - need "Sensitive Components" preset
- **Leaded solder** requires different temperature profiles (210-230°C range)
- **High-mass boards** need longer soak times and may use full 250°C peak

### Critical Temperature Constraints

⚠️ **Time at Peak Temperature**:
- **Maximum 30-60 seconds** at peak temperature
- Exceeding causes browning, deformation, or melting
- Our presets use 35-50 seconds (safe range)

⚠️ **Component Ratings**:
- **JEDEC MSL 3**: Rated to 260°C (most SMD plastic connectors)
- **Standard plastics**: Rated to 245-250°C
- **Budget/Unknown**: May fail at 200-220°C
- **Safety margin**: Use 15°C below rated maximum

⚠️ **Solder Paste Constraints**:
- **SAC305 (lead-free)**: Melts at 217°C, needs 235-250°C peak
- **Sn63/Pb37 (leaded)**: Melts at 183°C, needs 210-230°C peak
- **Time above liquidus**: Should not exceed 60-90 seconds total

## Default Presets

The system comes with 5 built-in presets based on **JEDEC J-STD-020** standards:

### 1. Lead-Free Standard
- **Preheat**: 150°C for 90s
- **Soak**: 180°C for 90s
- **Reflow**: 245°C for 40s
- **Description**: SAC305 lead-free, 245C peak (JEDEC compliant)
- **Use for**: Normal PCBs with JEDEC-rated SMD components (260°C rating)
- **Standards**: JEDEC J-STD-020 compliant, 15°C safety margin

### 2. Lead-Free Safe
- **Preheat**: 145°C for 90s
- **Soak**: 175°C for 90s
- **Reflow**: 235°C for 40s
- **Description**: Conservative lead-free for standard plastic headers
- **Use for**: Boards with standard plastic connectors, 40-pin headers ⭐ **RECOMMENDED FOR YOUR CASE**
- **Standards**: 25°C below JEDEC maximum, safe for most rated components

### 3. Leaded Solder
- **Preheat**: 120°C for 90s
- **Soak**: 150°C for 90s
- **Reflow**: 220°C for 40s
- **Description**: Sn63/Pb37 leaded solder (210-230C range)
- **Use for**: Hobbyist projects using traditional leaded solder paste
- **Standards**: Center of industry range for eutectic solder

### 4. Sensitive Components
- **Preheat**: 130°C for 90s
- **Soak**: 160°C for 90s
- **Reflow**: 210°C for 35s
- **Description**: Non-rated plastics, cheap connectors (<210C)
- **Use for**: Budget headers, unknown component ratings, non-rated plastics
- **Standards**: Conservative approach for uncertain component quality

### 5. High Mass Boards
- **Preheat**: 160°C for 120s
- **Soak**: 190°C for 120s
- **Reflow**: 250°C for 50s
- **Description**: Thick PCBs, large thermal mass (max 250C)
- **Use for**: Multi-layer boards, thick copper (>2oz), large ground planes
- **Standards**: Maximum JEDEC lead-free peak temperature

## Using Presets via Web Interface

### Loading a Preset

1. Open the web interface (http://192.168.4.1 or your device IP)
2. Navigate to the "Reflow Profile Configuration" section
3. Click the **"Profile Preset"** dropdown
4. Select the desired preset (e.g., "Lead-Free Low-Temp - For heat-sensitive components")
5. Confirm the prompt to load the preset
6. The temperature and time values will automatically update
7. The preset is automatically saved as your current configuration

### Creating a Custom Preset

1. Manually configure the temperature and time values
2. Click "Save Configuration" to test your profile
3. Run a test reflow to verify the profile works
4. Once satisfied, enter a name in the **"Save Current as Preset"** field
5. Click the **Save** button
6. Enter a description when prompted
7. Your preset is now saved and available in the dropdown

### Example: Saving a Preset for 40-Pin Headers

```
Name: "SMD with 40-pin Headers"
Description: "Lower reflow temp to prevent header melting"

Settings:
- Preheat: 140°C for 90s
- Soak: 170°C for 90s
- Reflow: 200°C for 40s
```

## API Endpoints

For advanced users and automation:

### Get All Presets
```
GET /api/presets
```

Returns JSON array of all presets:
```json
[
  {
    "name": "Lead-Free Standard",
    "description": "Standard SAC305 lead-free profile",
    "preheatTemp": 150,
    "soakTemp": 180,
    "reflowTemp": 230,
    "cooldownTemp": 25,
    "preheatTime": 90000,
    "soakTime": 90000,
    "reflowTime": 40000,
    "cooldownTime": 60000
  }
]
```

### Load a Preset
```
POST /api/presets/load
Content-Type: application/json

{
  "name": "Lead-Free Low-Temp"
}
```

### Save Current Settings as Preset
```
POST /api/presets/save
Content-Type: application/json

{
  "name": "My Custom Profile",
  "description": "Custom settings for my boards"
}
```

### Delete a Preset
```
POST /api/presets/delete
Content-Type: application/json

{
  "name": "My Custom Profile"
}
```

## Storage

Presets are stored in `/presets.json` on the device's LittleFS flash filesystem. They persist across reboots and are independent of the current working configuration.

## Technical Details

### File Structure

**presets.h** / **presets.cpp**
- Preset management system
- Default preset definitions
- Flash storage interface

**Preset Data Structure**
```cpp
struct ReflowProfile {
  char name[32];
  double preheatTemp;
  double soakTemp;
  double reflowTemp;
  double cooldownTemp;
  unsigned long preheatTime;
  unsigned long soakTime;
  unsigned long reflowTime;
  unsigned long cooldownTime;
  char description[64];
};
```

### Maximum Presets

The system supports up to **10 presets** (configurable via `MAX_PRESETS` in `presets.h`).

## Best Practices

1. **Test profiles before production**: Always run a test board first
2. **Document your presets**: Use descriptive names and detailed descriptions
3. **Start conservative**: Begin with lower temperatures and adjust up if needed
4. **Monitor the first cycle**: Watch the temperature curve on your first reflow
5. **Consider thermal mass**: Larger boards may need longer soak times

## Troubleshooting

### Preset Not Loading
- Check that the preset name is spelled correctly
- Verify the presets file exists in LittleFS
- Check serial console for error messages

### Preset Lost After Reboot
- Ensure you clicked "Save" when creating the preset
- Check available flash space
- Verify LittleFS is mounted correctly

### Temperature Still Too High
- Try the "Low Temperature" preset
- Create a custom preset with lower values
- Check your solder paste specifications

## Example Use Cases

### Case 1: Board with Plastic USB Connector
**Problem**: Standard 230°C profile melted the plastic USB connector housing

**Solution**: Use "Lead-Free Low-Temp" preset (210°C peak)

### Case 2: 4-Layer Board with Large Ground Plane
**Problem**: Standard profile didn't get the board hot enough; poor solder joints

**Solution**: Use "High Mass Boards" preset with longer soak times

### Case 3: Mixed Assembly (SMD + Through-Hole Headers)
**Problem**: 40-pin header melted at 230°C

**Solution**: Create custom preset:
- Preheat: 140°C for 90s
- Soak: 165°C for 90s
- Reflow: 200°C for 40s
- Save as "Mixed Assembly Headers"

## Future Enhancements

Planned features:
- [ ] Import/export presets as JSON files
- [ ] Ramp rate control in presets
- [ ] Temperature curve visualization per preset
- [ ] Preset sharing/community library
- [ ] Automatic preset recommendation based on board characteristics

## Version History

- **v1.0** (2025-03-18): Initial preset system implementation
  - 5 default presets
  - Web UI integration
  - Flash storage
  - REST API

## Contributing

To add more default presets, edit `src/presets.cpp` and add entries to the `defaultPresets[]` array.

