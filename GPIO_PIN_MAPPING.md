# GPIO Pin Mapping - Quick Reference

## Updated Pin Configuration (March 17, 2026)

### ✅ New GPIO Pin Assignments

Using GPIO pins **2-5** for easier access and consecutive wiring.

---

## 📌 Pin Connections

### MAX31855 Thermocouple Amplifier

| Pico W Pin | Physical Pin # | MAX31855 Pin | Function |
|------------|----------------|--------------|----------|
| **GPIO 2** | Pin 4 | CLK (or SCK) | SPI Clock Signal |
| **GPIO 3** | Pin 5 | CS | Chip Select |
| **GPIO 4** | Pin 6 | DO (or MISO) | Data Out (Master In) |
| **3.3V** | Pin 36 | VCC (or 3V3) | Power Supply |
| **GND** | Pin 38 (or any GND) | GND | Ground |

### SSR (Solid State Relay) for Heater Control

| Pico W Pin | Physical Pin # | SSR Connection | Function |
|------------|----------------|----------------|----------|
| **GPIO 5** | Pin 7 | Control/Signal | Heater On/Off Control |
| **GND** | Any GND pin | GND (if needed) | Ground Reference |

---

## 🔌 Wiring Diagram (Text)

```
Raspberry Pi Pico W                    MAX31855 Thermocouple Board
┌─────────────────┐                    ┌──────────────────────┐
│                 │                    │                      │
│  GPIO 2 (Pin 4) ├────────────────────┤ CLK                  │
│  GPIO 3 (Pin 5) ├────────────────────┤ CS                   │
│  GPIO 4 (Pin 6) ├────────────────────┤ DO (MISO)            │
│  3.3V  (Pin 36) ├────────────────────┤ VCC                  │
│  GND   (Pin 38) ├────────────────────┤ GND                  │
│                 │                    │                      │
│  GPIO 5 (Pin 7) ├─────────────┐      │ [T+] [T-]            │
│                 │             │      └──────────────────────┘
└─────────────────┘             │              ↑
                                │              │ Thermocouple
                                │              │ K-Type
                                │
                                ↓
                         SSR Control Input
                         (Heater Relay)
```

---

## 📍 Physical Pin Locations on Pico W

### Looking at Pico W with USB port at top:

```
         USB Port
           ___
          |   |
    ┌─────┴───┴─────┐
    │ •             • │  Pin 1 (GP0)   - Not used
    │ •             • │  Pin 2 (GP1)   - Not used
    │ •             • │  Pin 3 (GND)   - Available GND
    │ •  [GP2]      • │  Pin 4 (GP2)   - ✅ MAX31855 CLK
    │ •  [GP3]      • │  Pin 5 (GP3)   - ✅ MAX31855 CS
    │ •  [GP4]      • │  Pin 6 (GP4)   - ✅ MAX31855 DO
    │ •  [GP5]      • │  Pin 7 (GP5)   - ✅ SSR Control
    │ •             • │  Pin 8 (GND)   - Available GND
    │ •             • │  ...
    │ •             • │
    │ •             • │
    │ •             • │
    │ •             • │
    │ •             • │
    │ •             • │
    │ •             • │
    │ •             • │
    │ •             • │
    │ •             • │  Pin 36 (3V3)  - ✅ MAX31855 VCC
    │ •             • │
    │ •             • │  Pin 38 (GND)  - ✅ MAX31855 GND
    │ •             • │  Pin 40 (VBUS) - USB Power
    └───────────────┘
```

**Note**: Pins are grouped together on **one side** of the board (left side) for easy breadboard wiring!

---

## 🎯 Why These Pins?

### Advantages of GPIO 2-5:

1. **Consecutive pins** - Easy to wire on a breadboard
2. **All on one side** - Cleaner wiring layout
3. **Lower pin numbers** - Easier to remember
4. **Not using special functions** - No conflicts with I2C, UART, etc.
5. **Within range 1-15** - As requested

### GPIO Pins Not Used (Available for Expansion):

- GPIO 0, 1: Available
- GPIO 6-15: Available for additional sensors, displays, etc.
- GPIO 16-29: Available

---

## ⚠️ Important Notes

### Software SPI
This configuration uses **software SPI** (bit-banged), not hardware SPI. This means:
- ✅ Any GPIO pins can be used
- ✅ No need to match hardware SPI pins
- ⚠️ Slightly slower than hardware SPI (not an issue for MAX31855)
- ✅ More flexible pin assignment

### Pin Capabilities
All GPIO 2-5 pins support:
- ✅ Digital input/output
- ✅ PWM (if needed for future features)
- ✅ 3.3V logic level
- ✅ Software SPI

---

## 🔧 Testing Pin Connections

### Using Multimeter (Continuity Mode)

**Test CLK (GPIO 2)**:
1. Touch red probe to Pico W Pin 4 (GPIO 2)
2. Touch black probe to MAX31855 CLK pin
3. Should beep (continuity confirmed)

**Test CS (GPIO 3)**:
1. Touch red probe to Pico W Pin 5 (GPIO 3)
2. Touch black probe to MAX31855 CS pin
3. Should beep

**Test DO (GPIO 4)**:
1. Touch red probe to Pico W Pin 6 (GPIO 4)
2. Touch black probe to MAX31855 DO/MISO pin
3. Should beep

**Test Power (Voltage Mode)**:
1. Set multimeter to DC voltage
2. Red probe to MAX31855 VCC
3. Black probe to MAX31855 GND
4. Should read **3.3V** (±0.2V acceptable)

---

## 📋 Wire Color Suggestions

To keep track of your wiring, use consistent colors:

| Connection | Suggested Color | Your Color |
|------------|-----------------|------------|
| GPIO 2 (CLK) | Yellow | _______ |
| GPIO 3 (CS) | Orange | _______ |
| GPIO 4 (DO) | Blue | _______ |
| GPIO 5 (SSR) | Green | _______ |
| 3.3V | Red | _______ |
| GND | Black | _______ |

**Fill in "Your Color" column** for future reference!

---

## 🔄 Changed from Previous Version

### Old Pin Configuration:
```
GPIO 18 → CLK     (was on far side of board)
GPIO 17 → CS      (was on far side of board)
GPIO 16 → DO      (was on far side of board)
GPIO 15 → SSR     (was on far side of board)
```

### New Pin Configuration:
```
GPIO 2 → CLK      (now on near side - left edge)
GPIO 3 → CS       (now on near side - left edge)
GPIO 4 → DO       (now on near side - left edge)
GPIO 5 → SSR      (now on near side - left edge)
```

**Benefits**:
- ✅ Easier physical access
- ✅ All pins grouped together
- ✅ Simpler breadboard layout
- ✅ Lower pin numbers (easier to remember)
- ✅ Uses requested GPIO 1-15 range

---

## 📖 Code Reference

### In `main.cpp`:
```cpp
// Pin Definitions (Pico W)
#define THERMO_CLK 2    // GPIO2
#define THERMO_CS 3     // GPIO3
#define THERMO_DO 4     // GPIO4 (MISO)
#define SSR_PIN 5       // GPIO5
```

---

## ✅ Verification Checklist

Before powering on, verify:

- [ ] GPIO 2 connected to MAX31855 CLK
- [ ] GPIO 3 connected to MAX31855 CS
- [ ] GPIO 4 connected to MAX31855 DO (MISO)
- [ ] GPIO 5 connected to SSR control input
- [ ] 3.3V connected to MAX31855 VCC
- [ ] GND connected to MAX31855 GND
- [ ] Thermocouple plugged into MAX31855 T+ and T-
- [ ] All connections are secure (wiggle test)
- [ ] No shorts between adjacent pins
- [ ] SSR properly connected to heater circuit

---

## 🆘 If Still Getting Errors

After rewiring with new pins, if you still see thermocouple errors:

1. **Double-check pin numbers** - GPIO 2, 3, 4 (not physical pins 2, 3, 4!)
2. **Verify code uploaded** - Make sure you uploaded the updated firmware
3. **Check wire continuity** - Use multimeter to test each connection
4. **Verify 3.3V power** - Measure voltage at MAX31855 VCC pin
5. **Try swapping wires** - Test with different jumper wires

---

**Updated**: March 17, 2026
**Pins Used**: GPIO 2, 3, 4, 5
**Range**: Within GPIO 1-15 as requested ✅
**Status**: Ready for rewiring and testing

