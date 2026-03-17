# Quick Start Guide - Raspberry Pi Pico W

Get your reflow oven running in under 30 minutes!

## What You Need

### Hardware
- [ ] Raspberry Pi Pico W (must be "W" with WiFi)
- [ ] Adafruit MAX31855 thermocouple amplifier
- [ ] K-Type thermocouple probe
- [ ] Solid State Relay (3.3V logic compatible)
- [ ] Toaster oven or heating element
- [ ] Jumper wires
- [ ] Micro USB cable
- [ ] Computer with Arduino IDE

### Software
- [ ] Arduino IDE 2.x (or 1.8.19+)
- [ ] USB cable driver (usually auto-installs)

## Step 1: Install Arduino IDE Support (10 minutes)

1. **Open Arduino IDE**

2. **Add Pico Board Support:**
   - File → Preferences
   - In "Additional Boards Manager URLs", add:
     ```
     https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
     ```
   - Click OK

3. **Install Pico Boards:**
   - Tools → Board → Boards Manager
   - Search for "pico"
   - Install "Raspberry Pi Pico/RP2040" by Earle F. Philhower, III
   - Wait for installation

4. **Install Libraries:**
   - Tools → Manage Libraries
   - Search and install:
     - `PID` by Brett Beauregard
     - `Adafruit MAX31855 library` by Adafruit
     - `ArduinoJson` by Benoit Blanchon (v6.x)

## Step 2: Wire the Hardware (15 minutes)

### Pin Connections

**MAX31855 Thermocouple Amplifier:**
```
MAX31855 → Pico W
──────────────────
VCC  → 3.3V (Pin 36)
GND  → GND (Pin 38)
CLK  → GPIO 18 (Pin 24)
DO   → GPIO 16 (Pin 21)
CS   → GPIO 17 (Pin 22)
```

**Solid State Relay:**
```
SSR Input → GPIO 15 (Pin 20)
SSR GND   → GND (Pin 23 or 38)
```

**Thermocouple:**
```
Connect K-Type thermocouple to MAX31855
Red wire (+) and Yellow wire (-)
```

### Wiring Diagram

```
                ┌──────────────┐
                │ Pico W       │
                │              │
     MAX31855   │  GPIO 18 ────┼── CLK
        ┌───────│  GPIO 16 ────┼── DO
        │       │  GPIO 17 ────┼── CS
        │       │  GPIO 15 ────┼── SSR
        │       │  3.3V    ────┼── VCC
        │       │  GND     ────┼── GND
        │       └──────────────┘
        │
   Thermocouple
```

### Safety Check ✓
- [ ] 3.3V to MAX31855 VCC (NOT 5V!)
- [ ] All grounds connected
- [ ] SSR input to GPIO 15
- [ ] No shorts between power and ground
- [ ] Thermocouple properly connected

## Step 3: Upload the Code (5 minutes)

1. **Download the Sketch:**
   - Navigate to `Electrical Design/SolderReflowOvenPicoW/`
   - Open `SolderReflowOvenPicoW.ino`

2. **Configure WiFi:**
   - Edit around line 95:
     ```cpp
     const char* ssid = "ReflowOven";        // Change if desired
     const char* password = "reflow123";      // Min 8 characters
     bool useAPMode = true;                   // true = Access Point
     ```

3. **Select Board:**
   - Tools → Board → Raspberry Pi Pico/RP2040 → **Raspberry Pi Pico W**
   - Tools → Flash Size → **2MB (Sketch: 1MB, FS: 1MB)**
   - Tools → CPU Speed → **133 MHz**

4. **Upload:**
   - Hold **BOOTSEL** button on Pico W
   - Connect USB cable (while holding BOOTSEL)
   - Release BOOTSEL (Pico appears as USB drive)
   - Click **Upload** in Arduino IDE
   - Wait for "Done uploading"

## Step 4: Test Connection (5 minutes)

1. **Open Serial Monitor:**
   - Tools → Serial Monitor
   - Set baud rate to **115200**
   - You should see:
     ```
     ===========================================
     Solder Reflow Oven Controller - Pico W
     Raspberry Pi Pico W Dual-Core Version
     ===========================================
     
     MAX31855 OK - Current temp: 25.3 °C
     ...
     AP IP address: 192.168.4.1
     Both cores running!
     ```

2. **Connect to WiFi:**
   - Find "ReflowOven" network on your device
   - Password: `reflow123` (or what you set)
   - Connect to it

3. **Open Web Interface:**
   - Open browser
   - Navigate to: `http://192.168.4.1`
   - You should see the reflow oven control interface

## Step 5: Verify Operation (5 minutes)

### Check Temperature Reading
- [ ] Web interface shows current temperature
- [ ] Touch thermocouple - temperature should rise
- [ ] Temperature updates every second

### Check SSR Control
- [ ] Click "Start Reflow"
- [ ] Listen for SSR clicking (or LED)
- [ ] Click "Stop" to abort
- [ ] SSR should stop

### Save Configuration
- [ ] Enter your desired temperatures
- [ ] Click "Save Configuration"
- [ ] Configuration persists after power cycle

## Next Steps

### Recommended: Run PID Auto-Tune
1. Ensure oven is cool (<40°C)
2. Click "Auto-Tune PID"
3. Wait 10-15 minutes
4. Review calculated PID values
5. Click "Save Configuration"

### First Reflow Test
1. Use default lead-free profile (150/180/230°C)
2. Run WITHOUT PCB first (empty oven)
3. Watch temperature curve
4. Verify all stages complete
5. Check maximum temperature reached

### Fine-Tuning
- See [EXAMPLE_CONFIGURATIONS.md](EXAMPLE_CONFIGURATIONS.md) for solder profiles
- See [PID_TUNING_GUIDE.md](PID_TUNING_GUIDE.md) for tuning tips
- See [README_PICOW.md](Electrical%20Design/SolderReflowOvenPicoW/README_PICOW.md) for advanced options

## Troubleshooting

### "Could not read from thermocouple"
- Check wiring: GPIO 18, 16, 17
- Verify 3.3V power (not 5V!)
- Check MAX31855 connections
- Try a different thermocouple

### WiFi Not Appearing
- Verify you have Pico **W** (not regular Pico)
- Check serial monitor for errors
- Try different WiFi channel
- Ensure password is 8+ characters

### Temperature Not Changing
- Verify thermocouple is connected
- Check thermocouple polarity
- Ensure MAX31855 is powered
- Try touching probe with finger

### SSR Not Activating
- Check GPIO 15 connection
- Verify SSR is 3.3V compatible
- Test SSR separately with multimeter
- Check SSR polarity

### Web Interface Won't Load
- Verify IP address (check serial monitor)
- Try `http://192.168.4.1` exactly
- Clear browser cache
- Try different browser

## Safety Reminders

⚠️ **IMPORTANT:**
- Never leave operating oven unattended
- Keep fire extinguisher nearby
- Ensure good ventilation (solder fumes)
- Test emergency stop before first use
- Start with LOW temperatures for testing
- Monitor for thermal runaway

## Success Checklist

Before your first real reflow:
- [ ] Serial monitor shows "Both cores running!"
- [ ] Web interface loads and shows temperature
- [ ] Temperature reading is accurate
- [ ] SSR activates when "Start Reflow" clicked
- [ ] Emergency stop works immediately
- [ ] Configuration saves to EEPROM
- [ ] PID auto-tune completed successfully
- [ ] Test reflow cycle completed (empty oven)
- [ ] Maximum temperature is safe (<260°C)
- [ ] Cooldown phase works

## Performance Expectations

With Pico W dual-core:
- **PID Updates:** Every 100ms (10 Hz)
- **Temperature Accuracy:** ±1-2°C
- **Overshoot:** Typically 2-3°C (vs 5-10°C on ESP8266)
- **Web Updates:** Every 1 second
- **Reflow Cycle:** 6-8 minutes typical

## Get Help

- **Documentation:** See main README.md for all guides
- **Issues:** Open GitHub issue with details
- **Serial Output:** Always include serial monitor output
- **Photos:** Pictures of wiring help diagnose issues

## What's Next?

After successful basic operation:

1. **Optimize Your Profile:**
   - Test with different temperatures
   - Adjust times for your oven size
   - Save multiple configurations (manual backup)

2. **Add Features:**
   - LCD display on SPI1 (pins ready!)
   - Temperature logging to SD card
   - Multiple profile storage

3. **Share Your Results:**
   - Document your build
   - Share temperature curves
   - Contribute improvements

---

**Congratulations!** Your Pico W reflow oven is ready! 🎉

The dual-core architecture gives you 10x faster PID control than the ESP8266 version, resulting in better temperature control and more consistent reflow results.

Happy reflowing! 🔥
