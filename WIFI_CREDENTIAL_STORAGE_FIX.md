# WiFi Credential Storage Fix for Raspberry Pi Pico W

## Problem Description

The Raspberry Pi Pico W was forgetting WiFi credentials after power disconnect and requiring WiFi setup to be run again each time.

## Root Cause

The issue was in the EEPROM handling code. The Raspberry Pi Pico W uses **emulated EEPROM** in flash memory, which requires specific initialization and finalization steps:

1. **Missing `EEPROM.begin()`** in `saveConfigToEEPROM()` - The code only called `EEPROM.begin()` during load, not during save
2. **Missing `EEPROM.end()`** - Neither load nor save properly closed the EEPROM session after operations
3. Without proper initialization and finalization, writes to EEPROM were not being committed to flash memory

## The Fix

### Changes Made to `/src/main.cpp`

#### 1. Updated `saveConfigToEEPROM()` function:
```cpp
void saveConfigToEEPROM() {
  EEPROM.begin(EEPROM_SIZE);  // ✅ Must call begin() before writing

  EEPROMConfig config;
  config.magic = EEPROM_MAGIC;
  // ... populate config struct ...

  EEPROM.put(0, config);
  EEPROM.commit();  // Commit changes to flash
  EEPROM.end();     // ✅ End EEPROM session to ensure data is written

  Serial.println("Configuration saved to EEPROM");
  Serial.print("Saved WiFi SSID: ");
  Serial.println(storedSSID);
}
```

#### 2. Updated `loadConfigFromEEPROM()` function:
```cpp
void loadConfigFromEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROMConfig config;
  EEPROM.get(0, config);

  if (config.magic == EEPROM_MAGIC) {
    // ... load all config values ...
    Serial.println("Configuration loaded successfully!");
    // ... print loaded values ...
  } else {
    Serial.println("No valid EEPROM configuration found, using defaults");
  }

  EEPROM.end();  // ✅ End EEPROM session after loading
}
```

## How Pico W EEPROM Emulation Works

The Raspberry Pi Pico W doesn't have actual EEPROM hardware. Instead, it emulates EEPROM in flash memory:

1. **`EEPROM.begin(size)`** - Allocates a buffer in RAM and loads existing data from flash
2. **`EEPROM.put()` / `EEPROM.write()`** - Modifies data in the RAM buffer only
3. **`EEPROM.commit()`** - Writes the RAM buffer to flash memory (critical!)
4. **`EEPROM.end()`** - Finalizes the write and releases resources

**Without `EEPROM.begin()` before writing**: The library doesn't know where to write data
**Without `EEPROM.end()`**: Data may not be properly flushed to flash memory

## Testing the Fix

### 1. Upload the Fixed Code

```bash
cd /home/jack/CLionProjects/Solder-Reflow-Oven-IO
pio run -t upload -e pico
```

### 2. Monitor Serial Output

```bash
pio device monitor
```

You should see during boot:
```
Loading configuration from EEPROM...
Configuration loaded successfully!
Stored WiFi SSID: YourNetworkName
```

### 3. Test WiFi Credential Persistence

1. **Initial Setup**:
   - Power on the Pico W
   - Connect to "ReflowOven-Setup" AP
   - Navigate to http://192.168.4.1/wifi
   - Enter your WiFi credentials and save
   - Device will reboot

2. **First Verification**:
   - Watch serial monitor
   - Should see: "Connecting to WiFi: YourNetworkName"
   - Should connect successfully

3. **Power Cycle Test**:
   - Disconnect USB power completely
   - Wait 5 seconds
   - Reconnect USB power
   - Watch serial monitor
   - Should see: "Stored WiFi SSID: YourNetworkName"
   - Should auto-connect without showing captive portal

4. **Success Indicators**:
   - ✅ No captive portal appears after reboot
   - ✅ WiFi connects automatically
   - ✅ Serial shows "WiFi connected!" with IP address
   - ✅ Can access web interface at the IP address

### 4. Verify EEPROM Content (Optional Debug)

Add this debug code temporarily to `setup()` to see EEPROM contents:

```cpp
// Debug: Print raw EEPROM data
Serial.println("\n=== EEPROM Debug ===");
EEPROM.begin(EEPROM_SIZE);
uint16_t magic;
EEPROM.get(0, magic);
Serial.print("Magic value: 0x");
Serial.println(magic, HEX);
Serial.print("Expected: 0x");
Serial.println(EEPROM_MAGIC, HEX);

char testSSID[32];
EEPROM.get(88, testSSID);  // Offset to wifiSSID in struct
Serial.print("Stored SSID: ");
Serial.println(testSSID);
EEPROM.end();
Serial.println("===================\n");
```

## What Gets Saved to EEPROM

The following configuration is now properly persisted:

```cpp
struct EEPROMConfig {
  uint16_t magic;              // 0xABCE - validation marker

  // Reflow profile parameters
  double preheatTemp;
  double soakTemp;
  double reflowTemp;
  double cooldownTemp;
  unsigned long preheatTime;
  unsigned long soakTime;
  unsigned long reflowTime;
  unsigned long cooldownTime;

  // PID parameters
  double Kp;
  double Ki;
  double Kd;

  // WiFi credentials (NEW - now properly saved!)
  char wifiSSID[32];
  char wifiPassword[64];
};
```

**Total size**: ~170 bytes (512 bytes allocated for future expansion)

## Important Notes

### Flash Memory Wear

Flash memory has a limited number of write cycles (~10,000-100,000 writes per block). The code saves configuration:
- When user clicks "Save Configuration" in web UI
- When WiFi credentials are updated
- When PID auto-tune completes

**This is acceptable** because these events are infrequent (typically < 100 times over device lifetime).

### Backup Recommendations

Since WiFi credentials are now stored in flash:
1. Document your WiFi credentials separately
2. If you reflash the firmware, you'll need to re-enter WiFi settings
3. Consider using a dedicated WiFi network for IoT devices

### Magic Number Update

The EEPROM magic number was updated from `0xABCD` to `0xABCE` to force re-initialization when the new code runs. This ensures:
- Old configuration (without WiFi credentials) is ignored
- Users enter WiFi credentials using the new structure
- No corruption from struct size mismatch

## Troubleshooting

### WiFi Still Not Persisting

1. **Check Serial Output**: Verify you see "Configuration saved to EEPROM" after WiFi setup
2. **Flash Memory Issue**: Try erasing flash completely:
   ```bash
   pio run -t erase -e pico
   pio run -t upload -e pico
   ```
3. **EEPROM Size**: Verify `#define EEPROM_SIZE 512` at the top of main.cpp

### "No valid EEPROM configuration found"

This is normal on first boot or after firmware update. It means:
- No previous configuration exists
- Magic number doesn't match (expected after code update)
- EEPROM is empty/corrupted

**Solution**: Just configure WiFi once through the captive portal.

### Captive Portal Appears After Every Reboot

If captive portal still appears after the fix:

1. **Verify the fix was uploaded**:
   ```bash
   # Check git diff to confirm changes
   cd /home/jack/CLionProjects/Solder-Reflow-Oven-IO
   git diff src/main.cpp | grep "EEPROM.begin"
   ```

2. **Check for compilation errors**:
   ```bash
   pio run -e pico
   # Look for errors related to EEPROM
   ```

3. **Hard reset flash**:
   - Hold BOOTSEL button on Pico W
   - Connect USB (while holding BOOTSEL)
   - Release BOOTSEL
   - Re-upload firmware

## Technical Details: Why This Works

### Before the Fix (Broken)
```
Power On
  ↓
loadConfigFromEEPROM() ← EEPROM.begin() called ✅
  ↓
User configures WiFi
  ↓
saveConfigToEEPROM() ← EEPROM.begin() NOT called ❌
  ↓
EEPROM.put() ← Writes to uninitialized buffer ❌
  ↓
EEPROM.commit() ← Commits garbage data ❌
  ↓
Power Off
  ↓
WiFi credentials LOST ❌
```

### After the Fix (Working)
```
Power On
  ↓
loadConfigFromEEPROM()
  ├─ EEPROM.begin() ✅
  ├─ EEPROM.get() ✅
  └─ EEPROM.end() ✅
  ↓
User configures WiFi
  ↓
saveConfigToEEPROM()
  ├─ EEPROM.begin() ✅ NEW!
  ├─ EEPROM.put() ✅
  ├─ EEPROM.commit() ✅
  └─ EEPROM.end() ✅ NEW!
  ↓
Power Off
  ↓
Power On
  ↓
WiFi credentials LOADED ✅
  ↓
Auto-connect to WiFi ✅
```

## Summary

✅ **Fixed**: Added `EEPROM.begin()` to `saveConfigToEEPROM()`
✅ **Fixed**: Added `EEPROM.end()` to both save and load functions
✅ **Result**: WiFi credentials now persist across power cycles
✅ **Benefit**: No need to reconfigure WiFi after every power disconnect

The Pico W will now remember:
- WiFi SSID and password
- Reflow profile settings
- PID tuning parameters

All configuration persists in flash memory and survives power cycles! 🎉

