# Bill of Materials - Raspberry Pi Pico W Reflow Oven

Complete parts list for building a reflow oven controller with Raspberry Pi Pico W.

## Required Components

| Item | Qty | Description | Approx. Cost (USD) | Notes |
|------|-----|-------------|-------------------|-------|
| **Raspberry Pi Pico W** | 1 | RP2040 microcontroller with WiFi | $6.00 | Must be "W" version |
| **MAX31855 Breakout** | 1 | K-Type thermocouple amplifier | $15.00 | Adafruit or compatible |
| **K-Type Thermocouple** | 1 | Temperature probe (-200 to 1350°C) | $10.00 | Glass braid or stainless steel |
| **Solid State Relay (SSR)** | 1 | 25A, 3-32V DC control, 240V AC load | $8.00 | Must be 3.3V logic compatible |
| **Toaster Oven** | 1 | 1000-1500W heating element | $20-40 | Thrift store or new |
| **Micro USB Cable** | 1 | Power and programming | $3.00 | Data cable (not charge-only) |
| **Jumper Wires** | 10+ | Male-to-female or male-to-male | $5.00 | 22-24 AWG recommended |
| **Power Supply** | 1 | 5V USB power adapter | $5.00 | 1A minimum |
| **Heat-Resistant Wire** | 2m | 18 AWG for heater connection | $3.00 | Silicone insulation |
| **Terminal Block** | 2 | 2-position screw terminals | $2.00 | For easy SSR connection |
| **Enclosure** | 1 | Project box for electronics | $8.00 | Optional but recommended |
| | | | | |
| **Total (Minimum)** | | | **~$85-105** | Without enclosure or tools |

## Optional Components

| Item | Qty | Description | Approx. Cost (USD) | Notes |
|------|-----|-------------|-------------------|-------|
| **LCD Display** | 1 | 2.8" TFT SPI display | $15.00 | For future use on SPI1 |
| **Cooling Fan** | 1 | 12V DC fan for enclosure | $5.00 | Keeps electronics cool |
| **Emergency Stop Button** | 1 | Large red mushroom button | $5.00 | Physical safety switch |
| **Status LEDs** | 3 | Red, Yellow, Green indicators | $2.00 | Visual status feedback |
| **Buzzer** | 1 | Piezo buzzer for alarms | $2.00 | Audio alerts |
| **Thermal Fuse** | 1 | 240°C thermal cutoff | $3.00 | Additional safety |
| **Insulation** | 1 roll | Ceramic fiber blanket | $15.00 | Improve heating efficiency |
| **Breadboard** | 1 | For prototyping connections | $5.00 | Testing before soldering |
| **Perf Board** | 1 | For permanent installation | $3.00 | Cleaner final build |

## Tools Required

| Tool | Notes |
|------|-------|
| **Screwdrivers** | Phillips and flat-head |
| **Wire Strippers** | 18-24 AWG range |
| **Multimeter** | For testing connections |
| **Soldering Iron** | Optional for permanent connections |
| **Drill** | For mounting holes in enclosure |
| **Computer** | For programming Pico W |

## Component Details

### Raspberry Pi Pico W
- **Processor:** RP2040 dual-core ARM Cortex-M0+ @ 133 MHz
- **RAM:** 264 KB SRAM
- **Flash:** 2 MB
- **WiFi:** 2.4 GHz 802.11n
- **GPIO:** 26 multi-function pins
- **Size:** 51mm × 21mm
- **Where to buy:** Official distributors, SparkFun, Adafruit, DigiKey, Mouser

**Important:** Must be Pico **W** (with WiFi). Regular Pico will not work.

### MAX31855 Thermocouple Amplifier
- **Interface:** SPI
- **Accuracy:** ±2°C or ±0.75% (whichever is greater)
- **Resolution:** 0.25°C
- **Temperature Range:** -200°C to +1350°C
- **Supply Voltage:** 3.0V to 3.6V (perfect for Pico W)
- **Where to buy:** Adafruit #269, SparkFun, Amazon

**Recommended:** Adafruit MAX31855 breakout board (includes all needed components)

### K-Type Thermocouple
- **Type:** K (Chromel-Alumel)
- **Range:** -200°C to 1350°C (adequate for reflow)
- **Form Factor:** 
  - Glass braid insulated (up to 480°C continuous)
  - Stainless steel sheath (more durable)
- **Length:** 1m minimum recommended
- **Connector:** Standard mini thermocouple plug
- **Where to buy:** Adafruit, Omega, Amazon

**Tip:** Get one with a bent tip for better PCB contact.

### Solid State Relay (SSR)
- **Control Voltage:** 3-32V DC (works with 3.3V from Pico)
- **Load Rating:** 25A @ 240V AC minimum
- **Load Type:** Resistive (heater element)
- **Zero-crossing:** Optional (reduces EMI)
- **Mounting:** DIN rail or screw mount
- **Heat sink:** Required for loads >10A
- **Where to buy:** Fotek (genuine!), Crydom, Omron, Amazon

**Warning:** Many cheap SSRs on Amazon are counterfeit. Buy from reputable sellers.

**Recommended models:**
- Fotek SSR-25DA (verify genuine)
- Crydom D2425
- Omron G3MB-202P (low current)

### Toaster Oven Selection
- **Power:** 1000-1500W (higher is better)
- **Size:** Large enough for your PCBs
- **Elements:** Top and bottom heating (preferred)
- **Controls:** Simple on/off (will bypass internal controls)
- **Door:** Easy to modify for thermocouple entry
- **Cost:** $20-40 (often found at thrift stores)

**Good brands:** Black+Decker, Oster, Hamilton Beach

**Modifications needed:**
- Remove control electronics (bypass to direct connection)
- Drill small hole for thermocouple wire
- Add insulation (optional but improves performance)

## Wiring Components

### Jumper Wire Specifications
- **Gauge:** 22-24 AWG for low-voltage signal wires
- **Length:** 
  - 15-20cm for MAX31855 connections
  - 10-15cm for SSR control
- **Type:** 
  - Male-to-Female for Pico W to breakouts
  - Male-to-Male for breadboard prototyping

### Power Wire Specifications
- **Gauge:** 18 AWG minimum for heater connection
- **Insulation:** Silicone (high temperature rated)
- **Length:** As needed to reach oven (typically 0.5-1m)
- **Connectors:** Ring terminals or spade connectors

## Enclosure Recommendations

### Size Requirements
- **Minimum:** 150mm × 100mm × 50mm
- **Material:** Plastic (ABS or polycarbonate) or metal
- **Features:**
  - Ventilation holes (prevent overheating)
  - Cable entry points
  - Mounting points for components
  - Transparent lid (optional - useful for status LEDs)

### Layout Inside Enclosure
```
┌─────────────────────────────────────┐
│  [Emergency Stop]    [LED] [LED]    │
│                                     │
│  ┌──────┐  ┌───────────┐           │
│  │ Pico │  │ MAX31855  │           │
│  │  W   │  │           │           │
│  └──────┘  └───────────┘           │
│                                     │
│  USB Power ─┐   ┌─ SSR Control     │
│             │   │                  │
│  TC Input ──┘   └─ AC Power Out    │
└─────────────────────────────────────┘
```

## Cost Breakdown

### Budget Build (Minimal)
- Pico W: $6
- MAX31855: $15
- Thermocouple: $10
- SSR: $8
- Wires/connectors: $5
- Used toaster oven: $20
- USB cable/power: $5
- **Total: ~$70**

### Standard Build
- All above plus:
- New toaster oven: $40
- Enclosure: $8
- Terminal blocks: $2
- Insulation: $15
- **Total: ~$105**

### Deluxe Build
- All above plus:
- LCD display: $15
- Cooling fan: $5
- Emergency button: $5
- LEDs and buzzer: $4
- Perf board: $3
- **Total: ~$137**

## Where to Buy

### Online Retailers (US)
- **Adafruit** - Official distributor, excellent support
- **SparkFun** - Good selection, tutorials
- **DigiKey** - Huge inventory, reliable
- **Mouser** - Professional components
- **Amazon** - Fast shipping, check reviews carefully

### International
- **Pimoroni** (UK) - Official Pico distributor
- **AliExpress** (China) - Budget option, longer shipping
- **RS Components** (Global) - Professional supplier
- **Element14** (Global) - Large distributor

### Used/Salvage
- **Thrift stores** - Toaster ovens, enclosures
- **eBay** - Used SSRs (verify condition)
- **Local electronics shops** - Wire, connectors
- **Garage sales** - Ovens, tools

## Bill of Materials Template

Copy this for your shopping list:

```
[ ] Raspberry Pi Pico W - $6
[ ] MAX31855 Breakout - $15
[ ] K-Type Thermocouple - $10
[ ] SSR 25A (3.3V logic) - $8
[ ] Toaster Oven 1200W - $30
[ ] Micro USB Cable - $3
[ ] Jumper Wires (pack) - $5
[ ] USB Power Adapter 5V - $5
[ ] Heat-Resistant Wire - $3
[ ] Terminal Blocks (2) - $2
[ ] Enclosure - $8
[ ] (Optional) LCD Display - $15
[ ] (Optional) Cooling Fan - $5

Total: $___
```

## Upgrade from ESP8266

If you're upgrading from existing ESP8266 build:

**Reuse:**
- ✅ MAX31855 (same)
- ✅ Thermocouple (same)
- ✅ SSR (same)
- ✅ Toaster oven (same)
- ✅ Enclosure (same)
- ✅ Most wiring (same)

**New:**
- ❌ Raspberry Pi Pico W ($6)
- ❌ Possibly different jumper wires

**Cost to upgrade:** ~$6-10 (just the Pico W and maybe new wires)

## Safety Equipment (Recommended)

| Item | Purpose | Cost |
|------|---------|------|
| Fire Extinguisher | Electrical fire safety | $20-40 |
| Safety Glasses | Eye protection | $5-10 |
| Heat-Resistant Gloves | Handling hot components | $10-15 |
| First Aid Kit | Emergency preparedness | $15-25 |
| Smoke Detector | Early fire warning | $15-30 |

## Testing Equipment

| Item | Purpose | Cost |
|------|---------|------|
| Multimeter | Verify connections, voltages | $15-50 |
| Infrared Thermometer | Check oven temperatures | $15-30 |
| Thermal Camera | Advanced temp monitoring | $200+ |

## Recurring Supplies

| Item | Replacement Interval | Cost |
|------|---------------------|------|
| Thermocouple | 1-2 years (if damaged) | $10 |
| Solder Paste | Per project | $10-30 |
| Test PCBs | As needed | Varies |

## Total Investment Summary

| Build Type | One-Time Cost | Notes |
|------------|--------------|-------|
| **Minimal** | ~$70 | Used oven, no enclosure |
| **Standard** | ~$105 | New oven, basic enclosure |
| **Deluxe** | ~$137 | Display, safety features |
| **Upgrade from ESP8266** | ~$10 | Just swap microcontroller |

## Money-Saving Tips

1. **Buy used toaster oven** - Save $10-20
2. **Salvage enclosure** - Use old project box
3. **Generic MAX31855** - Compatible clones work (verify reviews)
4. **Bulk wire purchase** - Share with friends
5. **Skip optional items** - Add LCD display later
6. **3D print enclosure** - If you have access to printer

## Return on Investment

**Cost of alternatives:**
- Professional reflow oven: $500-2000
- Reflow service: $0.50-2.00 per PCB
- Hot air station: $50-300 (less consistent)

**Break-even:** 
- 50-100 PCBs vs service
- Immediate vs buying new oven

**Plus benefits:**
- Learn electronics
- Customize to your needs
- Upgradeable platform
- Fun project!

---

**Ready to build?** Check all items off your list and see the [QUICK_START.md](QUICK_START.md) guide!
