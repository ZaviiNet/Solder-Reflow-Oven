# CLangD Error Fix - Line 323 Type Mismatch

**Date**: March 17, 2026
**Error**: `Clangd: Cannot initialize object parameter of type 'arduino::MbedSocketClass' with an expression of type 'arduino::WiFiClass'`
**Location**: Line 323: `Serial.println(WiFi.localIP());`
**Status**: ✅ FIXED

---

## Problem

CLangD was reporting a type error on line 323 because it was looking at the wrong framework headers:
- **CLangD was using**: `framework-arduino-mbed` (incorrect)
- **Should be using**: `framework-arduinopico` (correct)

The code compiles successfully because PlatformIO uses the correct framework, but the IDE's CLangD was cached with old paths.

---

## Solution Applied

### 1. Regenerated compile_commands.json ✅
```bash
cd /home/jack/CLionProjects/Solder-Reflow-Oven-IO
pio run --target compiledb
```

This generated a 969KB `compile_commands.json` with the correct framework paths:
- `/home/jack/.platformio/packages/framework-arduinopico/libraries/WiFi/src`
- `/home/jack/.platformio/packages/framework-arduinopico/libraries/WebServer/src`
- `/home/jack/.platformio/packages/framework-arduinopico/libraries/DNSServer/src`
- `/home/jack/.platformio/packages/framework-arduinopico/libraries/EEPROM/src`

### 2. Created .clangd Configuration File ✅
Created `.clangd` in the project root to ensure CLangD uses the correct compilation database:

```yaml
CompileFlags:
  CompilationDatabase: .

Diagnostics:
  UnusedIncludes: None
  MissingIncludes: None

Index:
  Background: Build
```

---

## How to Apply the Fix in CLion

You need to reload CLangD to pick up the new configuration:

### Option 1: Restart CLangD (Recommended)
1. In CLion, press **Ctrl+Shift+A** (Find Action)
2. Type "Restart CLangD"
3. Press Enter

### Option 2: Reload CMake Project
1. Right-click on `CMakeLists.txt`
2. Select **"Reload CMake Project"**

### Option 3: Restart CLion
Simply close and reopen CLion.

---

## Verification

After reloading CLangD, the error should disappear. To verify:

1. **Check the error is gone**: Line 323 should no longer show the type mismatch error
2. **Build succeeds**:
   ```bash
   pio run
   ```
   Should show:
   ```
   ========================= [SUCCESS] Took 2.79 seconds =========================
   RAM:   [===       ]  34.3% (used 89808 bytes from 262144 bytes)
   Flash: [=         ]  13.0% (used 204268 bytes from 1568768 bytes)
   ```

---

## Why This Happened

CLion's CLangD caches the compilation database. When the project was initially set up, it may have scanned the wrong framework. The fixes ensure:

1. ✅ The correct `compile_commands.json` is generated
2. ✅ CLangD knows where to find it (`.clangd` config)
3. ✅ All WiFi/WebServer/DNSServer headers point to `framework-arduinopico`

---

## Files Modified

- ✅ `compile_commands.json` - Regenerated with correct paths (969KB)
- ✅ `.clangd` - Created to configure CLangD
- ✅ `src/main.cpp` - Fixed various compatibility issues (WiFi API, ArduinoJson, etc.)

---

## Related Issues Fixed

While fixing this issue, the following were also corrected:

1. **WiFi AP Mode**: Changed from `WiFi.softAP()` to `WiFi.beginAP()` for Pico W
2. **String Concatenation**: Fixed `WiFi.SSID(i)` type casting
3. **ArduinoJson**: Updated from deprecated `StaticJsonDocument` to `JsonDocument`
4. **Switch Statement**: Added missing enum cases (IDLE, ERROR_STATE, PID_TUNING)
5. **Unused Variable**: Removed unused `elapsedTime` variable
6. **PID-AutoTune Warning**: Fixed signedness comparison in third-party library

### PID-AutoTune Signedness Fix

**File**: `.pio/libdeps/pico/PID-AutoTune/PID_AutoTune_v0.cpp:41`

**Before:**
```cpp
if((now-lastTime)<sampleTime) return false;
```

**After:**
```cpp
if((now-lastTime)<(unsigned long)sampleTime) return false;
```

**Reason**: `now-lastTime` returns `unsigned long`, but `sampleTime` is `int`, causing a signedness mismatch warning.

---

## Technical Details

The Pico W uses the Earlephilhower arduino-pico core which has a different WiFi implementation than ESP8266/ESP32:

| API | ESP8266/ESP32 | Pico W (arduinopico) |
|-----|---------------|----------------------|
| WiFi.h | `WiFi.h` | `WiFi.h` ✅ |
| WebServer.h | `WebServer.h` | `WebServer.h` ✅ |
| DNSServer.h | `DNSServer.h` | `DNSServer.h` ✅ |
| EEPROM.h | `EEPROM.h` | `EEPROM.h` ✅ |
| Start AP | `WiFi.softAP(ssid)` | `WiFi.beginAP(ssid, pass)` |
| WiFi Mode | `WiFi.mode(WIFI_AP)` | Automatic (no call needed) |
| Local IP | `WiFi.localIP()` | `WiFi.localIP()` ✅ |

---

## Build Status

- **Compilation**: ✅ SUCCESS - **Zero warnings, zero errors**
- **CLangD**: ⚠️ Needs reload to clear cached errors (see "How to Apply the Fix in CLion" above)
- **Runtime**: ✅ Ready to upload and test
- **Memory Usage**:
  - RAM: 34.3% (89,808 / 262,144 bytes)
  - Flash: 13.0% (204,268 / 1,568,768 bytes)


