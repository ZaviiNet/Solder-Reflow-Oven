# ESP8266 NodeMCU 1.0 Setup Guide

This document describes the hardware and software configuration for running the Solder Reflow Oven on a NodeMCU 1.0 ESP12E (ESP8266) with an Adafruit MAX31855 thermocouple amplifier.

## Hardware Changes from Original Design

### Original Setup (Arduino Uno)
- **Microcontroller:** Arduino Uno (ATmega328P)
- **Thermocouple Amplifier:** Adafruit MAX31856
- **Display:** Adafruit HX8357 TFT with resistive touch

### New Setup (ESP8266)
- **Microcontroller:** NodeMCU 1.0 ESP12E (ESP8266)
- **Thermocouple Amplifier:** Adafruit MAX31855 (K-type only)
- **Display:** Adafruit HX8357 TFT with resistive touch

## Pin Mapping

### NodeMCU ESP8266 Pin Assignments

| Function | NodeMCU Pin | GPIO | Arduino Uno (Original) |
|----------|-------------|------|------------------------|
| **MAX31855 Thermocouple** | | | |
| SCK (Clock) | D5 | GPIO14 | D13 (Hardware SPI) |
| CS (Chip Select) | D4 | GPIO2 | D8 |
| DO (MISO/Data Out) | D6 | GPIO12 | D12 (Hardware SPI) |
| **SSR Control** | D8 | GPIO15 | D2 |
| **TFT Display** | | | |
| CS (Chip Select) | D1 | GPIO5 | D10 |
| DC (Data/Command) | D2 | GPIO4 | D9 |
| RST (Reset) | -1 | - | -1 (tied to reset) |
| MOSI (via hardware SPI) | D7 | GPIO13 | D11 |
| MISO (via hardware SPI) | D6 | GPIO12 | D12 |
| SCK (via hardware SPI) | D5 | GPIO14 | D13 |
| **Touch Screen** | | | |
| YP (Y+ analog read) | A0 | ADC | A0 |
| XM (X- analog/digital) | D3 | GPIO0 | A1 |
| YM (Y- digital) | D0 | GPIO16 | D7 |
| XP (X+ digital) | D7 | GPIO13 | D6 |

### Important Notes on Pin Selection

1. **SPI Pins**: The ESP8266 hardware SPI uses:
   - MOSI: GPIO13 (D7)
   - MISO: GPIO12 (D6)
   - SCK: GPIO14 (D5)

2. **Touchscreen Limitation**: The ESP8266 has only ONE ADC input (A0), while the original design uses two (A0 and A1). The touchscreen may have reduced accuracy. Consider these alternatives:
   - Use capacitive touch display (I2C-based)
   - Add external ADC via I2C (e.g., ADS1115)
   - Accept reduced touch accuracy with single ADC

3. **Voltage Levels**: ESP8266 is 3.3V logic. Ensure all connected devices support 3.3V:
   - ✓ MAX31855: 3.3V compatible
   - ✓ HX8357 TFT: 3.3V compatible
   - ✓ SSR: Most accept 3.3V logic (verify your specific model)

4. **GPIO15 (D8) for SSR**: GPIO15 must be LOW during boot. The SSR control code sets it LOW at startup, which is correct.

5. **GPIO0 (D3)**: Used for XM touch pin. Keep this pin floating or HIGH during boot.

6. **GPIO16 (D0)**: Used for YM touch pin. This pin cannot be used for interrupts on ESP8266.

## Software Changes

### Library Differences: MAX31856 vs MAX31855

| Feature | MAX31856 | MAX31855 |
|---------|----------|----------|
| Thermocouple Types | All types (B,E,J,K,N,R,S,T) | K-type ONLY |
| Initialization | `begin()` method required | No initialization needed |
| Temperature Reading | `readThermocoupleTemperature()` | `readCelsius()` |
| Conversion Mode | Configurable (oneshot/continuous) | Continuous only |
| Conversion Check | `conversionComplete()` | Not needed (always ready) |
| Trigger Conversion | `triggerOneShot()` | Not applicable |

### Code Changes Made

1. **Pin Definitions**: Updated to use ESP8266 GPIO pin names (D0-D8, A0)
2. **Library Include**: Changed from `Adafruit_MAX31856.h` to `Adafruit_MAX31855.h`
3. **Thermocouple Object**: Using software SPI constructor for MAX31855
4. **Setup Function**: Removed MAX31856-specific initialization
5. **Temperature Reading**: Changed from `readThermocoupleTemperature()` to `readCelsius()`
6. **Removed Conversion Logic**: MAX31855 continuously converts, removed `conversionComplete()` and `triggerOneShot()` calls

## Required Libraries for ESP8266

Install these libraries via Arduino Library Manager:
- **Adafruit MAX31855 library**
- **Adafruit GFX Library**
- **Adafruit HX8357 Library**
- **PID** (by Brett Beauregard)
- **TouchScreen** (by Adafruit)

## Arduino IDE Setup

1. **Install ESP8266 Board Support**:
   - File → Preferences → Additional Board Manager URLs
   - Add: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Tools → Board → Board Manager → Search "ESP8266" → Install

2. **Select Board**:
   - Tools → Board → ESP8266 Boards → NodeMCU 1.0 (ESP-12E Module)

3. **Configure Settings**:
   - CPU Frequency: 80 MHz (or 160 MHz for better performance)
   - Flash Size: 4M (3M SPIFFS)
   - Upload Speed: 115200

4. **Select Port**:
   - Tools → Port → (Your NodeMCU COM port)

## Wiring Diagram

```
NodeMCU ESP8266          MAX31855          Thermocouple
┌─────────────┐         ┌─────────┐        ┌──────────┐
│             │         │         │        │          │
│ D5 (GPIO14) ├────────►│ CLK     │        │          │
│ D4 (GPIO2)  ├────────►│ CS      │        │          │
│ D6 (GPIO12) │◄────────┤ DO      │        │          │
│ 3V3         ├────────►│ VCC     │        │          │
│ GND         ├────────►│ GND     │───────►│ -        │
│             │         │ T+      │◄───────┤ +        │
│             │         │ T-      │◄───────┤          │
└─────────────┘         └─────────┘        └──────────┘

NodeMCU ESP8266          SSR
┌─────────────┐         ┌─────────┐
│             │         │         │
│ D8 (GPIO15) ├────────►│ Input+  │
│ GND         ├────────►│ Input-  │
└─────────────┘         └─────────┘
```

## Known Limitations

1. **Touch Accuracy**: Single ADC may reduce touchscreen precision
2. **K-Type Only**: MAX31855 only supports K-type thermocouples (should be fine for solder reflow)
3. **Pin Sharing Critical Issue**: XP touch pin (D7/GPIO13) shares with SPI MOSI
   - Touch readings will be unreliable during SPI communication with TFT or thermocouple
   - The touch screen will NOT work reliably during active reflow when the display is updating
   - **Recommended Solutions**:
     - Replace resistive touch with I2C capacitive touch display
     - Use physical buttons connected to unused GPIO pins
     - Use rotary encoder with push button for menu navigation
     - Use web interface over WiFi (ESP8266 has built-in WiFi capability)
4. **XM Pin**: Using digital pin D3 for XM instead of analog may affect touch calibration

## Troubleshooting

### Thermocouple Reading Errors
- Check wiring: CLK, CS, DO connections
- Ensure thermocouple is properly connected to MAX31855
- Verify 3.3V power supply is stable
- During reflow, if thermocouple fails, the system will automatically shut down the SSR and display an error

### Touch Screen Not Working or Unreliable
- ESP8266's single ADC may cause reduced accuracy
- **Primary Issue**: Touch readings conflict with SPI during display updates
- Touch may work during setup menu but fail during active reflow
- Verify touch pins are correctly connected
- **Solution**: Consider replacing with capacitive touch, physical buttons, or web interface

### SSR Not Switching
- Verify SSR supports 3.3V logic input
- Check GPIO15 is LOW during boot
- Test with multimeter on SSR control input

### Display Issues
- Ensure TFT is 3.3V compatible
- Check SPI connections
- Verify CS and DC pins are correct

## Future Improvements

1. **RECOMMENDED**: Replace resistive touch with alternative input method (I2C capacitive touch, physical buttons, or web interface)
2. Add web interface for WiFi control (ESP8266 has built-in WiFi)
3. Store reflow profiles in SPIFFS or EEPROM
4. Remote monitoring via MQTT or HTTP
5. Add temperature data logging to SD card or cloud
6. Use I2C ADC (ADS1115) to properly support resistive touch with 2 analog inputs
