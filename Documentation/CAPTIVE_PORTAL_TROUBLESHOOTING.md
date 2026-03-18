# Captive Portal Timeout Troubleshooting

## Issue: Timeout When Accessing 192.168.4.1

**Date**: March 18, 2026
**Status**: In Progress

---

## Changes Made to Fix the Issue

### 1. **Improved Access Point Initialization**
- Added proper WiFi mode cleanup before starting AP
- Increased initialization delays
- Added retry logic if AP fails to start
- Added explicit channel and connection limit parameters
- Enhanced diagnostic output (MAC address, IP confirmation)

### 2. **Added Diagnostic Logging**
- All HTTP requests now log to Serial Monitor
- Shows redirects, 404s, and which client is connecting
- Helps identify if the web server is receiving requests

### 3. **Better Error Handling**
- Checks return value of `WiFi.softAPConfig()`
- Retries AP start with basic settings if initial attempt fails
- Provides clear error messages if AP won't start

---

## Debugging Steps

### Step 1: Check Serial Monitor Output

**Flash the updated firmware and watch the Serial Monitor during bootup:**

```bash
cd /home/jack/CLionProjects/Solder-Reflow-Oven-IO
pio run -t upload && pio device monitor
```

**Look for these key messages:**

✅ **Good startup sequence:**
```
Starting captive portal access point...
Captive Portal SSID: ReflowOven-Setup
Captive Portal IP:   192.168.4.1
Captive Portal MAC:  XX:XX:XX:XX:XX:XX
Connect to 'ReflowOven-Setup' and visit http://192.168.4.1 to configure WiFi
Captive portal is READY and LISTENING
HTTP server started on port 80
```

❌ **Bad startup (if you see these):**
```
ERROR: Failed to configure AP IP!
ERROR: Failed to start Access Point!
ERROR: Access Point failed to start after retry!
```

### Step 2: Connect to the AP

1. **Connect your device** to the WiFi network: `ReflowOven-Setup`
2. **Wait for connection** - should take 5-10 seconds
3. **Check your device's IP** - should be something like `192.168.4.2` or `192.168.4.3`

**On Windows:**
```cmd
ipconfig
```

**On Linux/Mac:**
```bash
ifconfig
# or
ip addr
```

**Expected output:**
- IP: `192.168.4.x` (where x = 2, 3, 4, etc.)
- Gateway: `192.168.4.1`
- Subnet: `255.255.255.0`

### Step 3: Test Connectivity

**Ping the Pico W:**
```bash
ping 192.168.4.1
```

✅ **Expected result:**
```
Reply from 192.168.4.1: bytes=32 time=5ms TTL=255
Reply from 192.168.4.1: bytes=32 time=3ms TTL=255
```

❌ **If you get timeouts:**
- Pico W AP is not running
- Firewall blocking ICMP
- Wrong IP address

### Step 4: Try Different URLs

**In your browser, try accessing:**

1. `http://192.168.4.1/` - Should redirect to `/wifi`
2. `http://192.168.4.1/wifi` - Should show captive portal page directly
3. `http://test.com` - DNS should redirect to portal (if connected to ReflowOven-Setup)

**Watch the Serial Monitor** - you should see:
```
HTTP: / requested - redirecting to /wifi (captive portal)
HTTP: /wifi requested - serving captive portal page
```

**If you see nothing in Serial Monitor:**
- Web server is not receiving the request
- Network connectivity issue

### Step 5: Check Browser Developer Tools

**Open browser DevTools (F12):**

1. Go to **Network** tab
2. Try accessing `http://192.168.4.1/`
3. Look for:
   - Request sent (should show pending or completed)
   - Response code (should be 302 redirect or 200 OK)
   - Errors (timeout, connection refused, etc.)

**Common issues:**
- **ERR_CONNECTION_TIMED_OUT**: Web server not responding
- **ERR_CONNECTION_REFUSED**: Server not listening on port 80
- **DNS_PROBE_FINISHED_NXDOMAIN**: DNS server not working (but direct IP should work)

---

## Common Root Causes & Fixes

### Issue 1: WiFi AP Not Starting

**Symptoms:**
- Can't see `ReflowOven-Setup` network
- Serial Monitor shows AP start errors

**Possible Causes:**
1. **WiFi already in use** - Previous connection not properly closed
2. **Hardware issue** - CYW43439 chip not responding
3. **Power issue** - Insufficient power for WiFi TX

**Fixes:**
- Power cycle the Pico W (unplug USB, wait 5 seconds, reconnect)
- Try different USB port / power supply
- Check for `WiFi.mode(WIFI_OFF)` followed by `WiFi.mode(WIFI_AP)` in code ✅ (already added)

### Issue 2: Web Server Not Responding

**Symptoms:**
- Can connect to AP
- Can ping 192.168.4.1
- Browser times out

**Possible Causes:**
1. **server.begin() not called** - Web server not started
2. **server.handleClient() not in loop** - Web server not processing requests
3. **Blocking code in loop** - Web server can't respond
4. **Firewall on client device** - Blocking HTTP requests

**Fixes:**
- Verify `server.begin()` is called ✅ (line 609)
- Verify `server.handleClient()` is in loop ✅ (line 637)
- Reduce temperature check interval if too short
- Try from different device (phone vs laptop)
- Disable firewall temporarily on client device

### Issue 3: DNS Server Not Working

**Symptoms:**
- `http://192.168.4.1` works
- `http://test.com` doesn't redirect

**Possible Causes:**
1. **dnsServer.start() failed**
2. **dnsServer.processNextRequest() not called in loop**
3. **Client device using cached DNS**

**Fixes:**
- Verify `dnsServer.processNextRequest()` in loop ✅ (line 634)
- Clear DNS cache on client device
- Use direct IP address instead

### Issue 4: Pico W WiFi Library Bug

**Known Issue:**
Some Pico W WiFi library versions have issues with AP mode. The CYW43439 chip can be finicky.

**Workaround:**
1. Power cycle between tests
2. Add longer delays in `setupCaptivePortal()` ✅ (already increased to 500ms)
3. Try setting explicit channel: `WiFi.softAP(AP_SSID, nullptr, 1)` ✅ (already added)

---

## Advanced Debugging

### Enable WiFi Debug Logging

Add to `platformio.ini`:
```ini
build_flags =
    -DDEBUG_RP2040_PORT=Serial
    -DDEBUG_RP2040_WIFI
```

This will show low-level WiFi events.

### Manual Web Server Test

Add this to `setup()` after `server.begin()`:

```cpp
Serial.println("\n=== TESTING WEB SERVER ===");
Serial.print("Listening on: http://");
Serial.print(WiFi.softAPIP());
Serial.println(":80");
Serial.println("Registered routes:");
Serial.println("  GET  /");
Serial.println("  GET  /wifi");
Serial.println("  POST /wifi/save");
Serial.println("  GET  /api/status");
Serial.println("========================\n");
```

### Test Minimal Web Server

If the captive portal still doesn't work, test with a minimal server:

```cpp
void setupWebServer() {
  server.on("/", []() {
    Serial.println("TEST: / accessed");
    server.send(200, "text/html", "<h1>TEST OK</h1>");
  });

  server.on("/test", []() {
    Serial.println("TEST: /test accessed");
    server.send(200, "text/plain", "Server is working!");
  });

  server.begin();
  Serial.println("Minimal test server started");
}
```

If this works but the full captive portal doesn't:
- Issue is with the captive portal HTML/logic
- Network scanning might be hanging

---

## What to Check in Serial Monitor

After flashing the updated firmware, connect your device to `ReflowOven-Setup` and try to access `http://192.168.4.1`. You should see:

```
HTTP: / requested - redirecting to /wifi (captive portal)
HTTP: /wifi requested - serving captive portal page
```

**If you see nothing:**
1. Web server not receiving requests
2. Check network connectivity (ping test)
3. Check client device is actually connected to `ReflowOven-Setup`

**If you see "redirecting" but browser times out:**
1. Browser not following redirect
2. Try accessing `/wifi` directly
3. Check browser console for errors

---

## Temporary Workaround

**If captive portal still doesn't work**, you can manually configure WiFi credentials:

### Option 1: Hardcode Credentials (Quick Test)

Edit `src/main.cpp` around line 45:

```cpp
// Stored WiFi credentials (loaded from flash, configured via captive portal)
char storedSSID[32] = "YOUR_WIFI_NAME";      // ← Change this
char storedPassword[64] = "YOUR_WIFI_PASSWORD";  // ← Change this
```

Flash the firmware - it will skip captive portal and connect directly.

### Option 2: Use Serial Commands

Add a Serial command parser in `loop()`:

```cpp
if (Serial.available()) {
  String cmd = Serial.readStringUntil('\n');
  if (cmd.startsWith("wifi:")) {
    // Parse: wifi:SSID,PASSWORD
    int commaPos = cmd.indexOf(',', 5);
    if (commaPos > 0) {
      String ssid = cmd.substring(5, commaPos);
      String pass = cmd.substring(commaPos + 1);
      ssid.toCharArray(storedSSID, sizeof(storedSSID));
      pass.toCharArray(storedPassword, sizeof(storedPassword));
      saveConfigToEEPROM();
      Serial.println("WiFi saved! Rebooting...");
      delay(1000);
      rp2040.reboot();
    }
  }
}
```

Then use Serial Monitor:
```
wifi:YourNetworkName,YourPassword
```

---

## Next Steps

1. **Flash updated firmware** with improved captive portal
2. **Monitor Serial output** during connection attempt
3. **Report findings**:
   - Can you see the AP?
   - Can you ping 192.168.4.1?
   - What do you see in Serial Monitor?
   - Any errors in browser console?

4. **If still not working**, try:
   - Different browser (Chrome, Firefox, Safari)
   - Different device (phone vs laptop)
   - Direct URL access: `http://192.168.4.1/wifi`

---

## Files Modified

1. `src/main.cpp`:
   - Line ~529: Enhanced `setupCaptivePortal()` with better initialization
   - Line ~1164: Added logging to `handleRoot()`
   - Line ~1847: Added logging to `handleCaptivePortal()`
   - Line ~2013: Added logging to `handleNotFound()`

All changes preserve existing functionality while adding diagnostics and robustness.

---

**Last Updated**: March 18, 2026
**Status**: Ready for testing

