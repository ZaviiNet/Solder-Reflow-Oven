# Preset Profiles - Visual UI Guide

## Web Interface - Preset Section

### Location
The preset controls are in the **"Reflow Profile Configuration"** card, at the top before the temperature inputs.

### UI Layout

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃  Reflow Profile Configuration                            ┃
┃━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┃
┃                                                           ┃
┃  ┌─────────────────────┬─────────────────────┐          ┃
┃  │ 📋 Profile Preset   │ 💾 Save Current     │          ┃
┃  │                     │    as Preset        │          ┃
┃  ├─────────────────────┼─────────────────────┤          ┃
┃  │ ▼ [Dropdown]        │ [Name] [Save]       │          ┃
┃  │                     │                     │          ┃
│  │ Options:            │ Enter preset name:  │          ┃
┃  │ • Manual Config     │ ┌──────────────┐    │          ┃
┃  │ • Lead-Free Std     │ │              │    │          ┃
┃  │ • Lead-Free Safe ⭐ │ └──────────────┘    │          ┃
┃  │ • Leaded Solder     │ ┌────────┐          │          ┃
┃  │ • Sensitive Parts   │ │  Save  │          │          ┃
┃  │ • High Mass Boards  │ └────────┘          │          ┃
┃  │                     │                     │          ┃
┃  │ Load a preset or    │ Save your current   │          ┃
┃  │ configure manually  │ settings as preset  │          ┃
┃  └─────────────────────┴─────────────────────┘          ┃
┃                                                           ┃
┃  ┌──────────────────────────────────────────┐            ┃
┃  │ Temperature & Time Configuration         │            ┃
┃  ├──────────────────────────────────────────┤            ┃
┃  │                                           │            ┃
┃  │  Preheat Temp (°C):  [150]               │            ┃
┃  │  Preheat Time (sec): [90 ]               │            ┃
┃  │                                           │            ┃
┃  │  Soak Temp (°C):     [180]               │            ┃
┃  │  Soak Time (sec):    [90 ]               │            ┃
┃  │                                           │            ┃
┃  │  Reflow Temp (°C):   [230]               │            ┃
┃  │  Reflow Time (sec):  [40 ]               │            ┃
┃  │                                           │            ┃
┃  └──────────────────────────────────────────┘            ┃
┃                                                           ┃
┃  ┌────────────────────────┐                              ┃
┃  │  Save Configuration    │                              ┃
┃  └────────────────────────┘                              ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

## Usage Workflows

### Workflow 1: Load a Preset

```
┌──────────────────────────────────────────────────┐
│ 1. Click Dropdown                                │
├──────────────────────────────────────────────────┤
│    📋 Profile Preset: ▼                          │
│                                                  │
│    ┌────────────────────────────────────┐       │
│    │ -- Manual Configuration --         │       │
│    │ Lead-Free Standard - Standard...   │       │
│    │ Lead-Free Safe - Conservative...  │ ◀─ Click!
│    │ Leaded Solder - Traditional...     │       │
│    │ Sensitive Components - Non-rated...│       │
│    │ High Mass Boards - Thick PCBs...   │       │
│    └────────────────────────────────────┘       │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ 2. Confirm Dialog                                │
├──────────────────────────────────────────────────┤
│                                                  │
│  ┌────────────────────────────────────────┐     │
│  │  Load preset "Lead-Free Safe"?         │     │
│  │  This will overwrite your current      │     │
│  │  configuration.                        │     │
│  │                                         │     │
│  │         [ OK ]        [Cancel]         │     │
│  └────────────────────────────────────────┘     │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ 3. Values Auto-Update                            │
├──────────────────────────────────────────────────┤
│    Preheat Temp (°C):  [145] ← Changed!         │
│    Preheat Time (sec): [90 ]                     │
│                                                  │
│    Soak Temp (°C):     [175] ← Changed!         │
│    Soak Time (sec):    [90 ]                     │
│                                                  │
│    Reflow Temp (°C):   [235] ← Changed! (was 245)
│    Reflow Time (sec):  [40 ]                     │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ 4. Success Message                               │
├──────────────────────────────────────────────────┤
│  ┌────────────────────────────────────────┐     │
│  │  ✅ Preset loaded successfully!        │     │
│  │                                         │     │
│  │               [ OK ]                   │     │
│  └────────────────────────────────────────┘     │
└──────────────────────────────────────────────────┘
```

### Workflow 2: Save Custom Preset

```
┌──────────────────────────────────────────────────┐
│ 1. Manually Configure Temperatures               │
├──────────────────────────────────────────────────┤
│    Preheat Temp (°C):  [145] ← Your values      │
│    Soak Temp (°C):     [175]                     │
│    Reflow Temp (°C):   [205]                     │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ 2. Enter Preset Name                             │
├──────────────────────────────────────────────────┤
│    💾 Save Current as Preset                     │
│                                                  │
│    ┌────────────────────────┐ ┌──────┐          │
│    │ 40-Pin Headers Custom  │ │ Save │          │
│    └────────────────────────┘ └──────┘          │
│           ▲                                      │
│           └─ Type your name here                │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ 3. Add Description (Optional)                    │
├──────────────────────────────────────────────────┤
│  ┌────────────────────────────────────────┐     │
│  │  Enter a description for this preset:  │     │
│  │                                         │     │
│  │  ┌────────────────────────────────┐    │     │
│  │  │ Custom low-temp for headers    │    │     │
│  │  └────────────────────────────────┘    │     │
│  │                                         │     │
│  │         [ OK ]        [Cancel]         │     │
│  └────────────────────────────────────────┘     │
└──────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────┐
│ 4. Saved & Available                             │
├──────────────────────────────────────────────────┤
│  ✅ Preset saved successfully!                   │
│                                                  │
│  Now in dropdown:                                │
│    📋 Profile Preset: ▼                          │
│       • Manual Configuration                     │
│       • Lead-Free Standard                       │
│       • Lead-Free Low-Temp                       │
│       • ... (other defaults)                     │
│       • 40-Pin Headers Custom ← NEW!            │
└──────────────────────────────────────────────────┘
```

## Dropdown Options Detail

```
┌─────────────────────────────────────────────────────┐
│ 📋 Profile Preset                                   │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌────────────────────────────────────────────┐    │
│  │ -- Manual Configuration --                 │    │
│  │━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━│    │
│  │ Lead-Free Standard                         │    │
│  │   SAC305, 245C peak (JEDEC compliant)      │    │
│  │   (150°C → 180°C → 245°C)                  │    │
│  │                                             │    │
│  │ Lead-Free Safe ⭐ RECOMMENDED              │    │
│  │   Conservative for plastic headers         │    │
│  │   (145°C → 175°C → 235°C)                  │    │
│  │                                             │    │
│  │ Leaded Solder                              │    │
│  │   Sn63/Pb37 leaded solder (210-230C)       │    │
│  │   (120°C → 150°C → 220°C)                  │    │
│  │                                             │    │
│  │ Sensitive Components                       │    │
│  │   Non-rated plastics, cheap connectors     │    │
│  │   (130°C → 160°C → 210°C)                  │    │
│  │                                             │    │
│  │ High Mass Boards                           │    │
│  │   Thick PCBs, large thermal mass           │    │
│  │   (160°C → 190°C → 250°C, 50s peak)        │    │
│  └────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────┘
```

## Before & After Comparison

### BEFORE (Manual Configuration):
```
You had to remember/know:
┌─────────────────────────────────────┐
│ "What temp won't melt my header?"   │
│                                     │
│ Too high → 💥 Melted components     │
│ Too low  → ❌ Poor solder joints    │
│                                     │
│ Required research and experience    │
└─────────────────────────────────────┘
```

### AFTER (With Presets):
```
Based on JEDEC J-STD-020 standards:
┌─────────────────────────────────────┐
│ "I have plastic headers"            │
│          ↓                          │
│ Check component datasheet           │
│          ↓                          │
│ Rated 260°C? → "Lead-Free Safe"    │
│ Unknown/cheap? → "Sensitive Parts"  │
│          ↓                          │
│ ✅ JEDEC-compliant settings loaded! │
│                                     │
│ 235°C or 210°C (industry standard)  │
│ 40s peak time (30-60s safe range)   │
└─────────────────────────────────────┘
```

## Mobile View (Responsive)

```
┌──────────────────────┐
│  Reflow Profile      │
│  Configuration       │
├──────────────────────┤
│                      │
│  📋 Profile Preset   │
│  ┌────────────────┐  │
│  │ ▼ Select...   │  │
│  └────────────────┘  │
│                      │
│  💾 Save Current     │
│  ┌────────────────┐  │
│  │ [Name]        │  │
│  ├────────────────┤  │
│  │    Save        │  │
│  └────────────────┘  │
│                      │
│  Preheat Temp (°C)   │
│  ┌────────────────┐  │
│  │     150        │  │
│  └────────────────┘  │
│                      │
│  (etc...)            │
└──────────────────────┘
```

## Color Indicators

The UI uses visual cues:
- **Green** button = Save action
- **Blue** = Dropdown selection
- **📋** = Preset selection
- **💾** = Save preset
- **⭐** = Recommended for heat-sensitive components

## Keyboard Navigation

- **Tab**: Move between fields
- **Enter**: Confirm selection
- **Escape**: Cancel dialog
- **Arrow keys**: Navigate dropdown

## Tips

💡 **Tip 1**: The dropdown shows the full description when you hover over each option

💡 **Tip 2**: After loading a preset, you can still manually fine-tune the values

💡 **Tip 3**: Use descriptive names for custom presets like "Customer X Boards" or "Prototype Rev 2"

💡 **Tip 4**: The preset dropdown persists - reload the page and your presets are still there!

---

**Visual Guide Version**: 1.0
**Last Updated**: March 18, 2026

