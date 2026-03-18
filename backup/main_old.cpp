/*
 * Solder Reflow Oven Controller - Raspberry Pi Pico W Version
 *
 * Hardware:
 * - Raspberry Pi Pico W (RP2040 + CYW43439)
 * - Adafruit MAX31855 K-Type Thermocouple Amplifier
 * - Solid State Relay (SSR) for heater control
 * 
 * Features:
 * - Web-based interface (no display required)
 * - WiFi connectivity with AP and Station modes
 * - Real-time temperature monitoring via WebSocket
 * - Dual-core PID control (10 Hz update rate)
 * - Emergency stop functionality
 * - Automatic PID tuning
 *
 * Pin Connections (Pico W):
 * - GPIO 2: MAX31855 CLK
 * - GPIO 3: MAX31855 CS
 * - GPIO 4: MAX31855 DO (MISO)
 * - GPIO 5: SSR Control
 */

#include <Adafruit_MAX31855.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>  // Use LittleFS instead of EEPROM for Pico W flash storage
#include <PID_v1.h>
#include <SPI.h>
#include <pico/stdlib.h>
#include <pico/bootrom.h>

// Pin Definitions (Pico W)
#define THERMO_CLK 2    // GPIO2
#define THERMO_CS 3     // GPIO3
#define THERMO_DO 4     // GPIO4 (MISO)
#define SSR_PIN 5       // GPIO5

// WiFi Configuration
// AP mode credentials (used for captive portal setup)
const char* AP_SSID = "ReflowOven-Setup";   // No password for easy access

// Stored WiFi credentials (loaded from flash, configured via captive portal)
char storedSSID[32] = "";
char storedPassword[64] = "";

// Web Server, DNS Server, and captive portal state
WebServer server(80);
DNSServer dnsServer;
bool captivePortalActive = false;
const byte DNS_PORT = 53;
const unsigned long REBOOT_DELAY_MS = 1500;  // Delay before reboot to allow HTTP response to flush

// MAX31855 Thermocouple
Adafruit_MAX31855 maxthermo = Adafruit_MAX31855(THERMO_CLK, THERMO_CS, THERMO_DO);

// Reflow Profile Parameters (in Celsius and milliseconds)
double preheatTemp = 150;
double soakTemp = 180;
double reflowTemp = 230;
double cooldownTemp = 25;
unsigned long preheatTime = 90000;    // 90 seconds
unsigned long soakTime = 90000;       // 90 seconds
unsigned long reflowTime = 40000;     // 40 seconds
unsigned long cooldownTime = 60000;   // 60 seconds

// PID Variables
double Setpoint = 0, Input = 0, Output = 0;
double Kp = 0.05, Ki = 0.001, Kd = 0.75;  // Conservative defaults based on auto-tune results
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// LittleFS Configuration
// Using JSON file in flash filesystem (much better than EEPROM for Pico W)
// Configuration stored in /config.json on flash partition

// Temperature-based state transition settings
bool useTemperatureBasedTransitions = true;  // Enable temperature-aware state transitions
double tempReachThreshold = 5.0;  // Degrees within target to consider "reached"
unsigned long maxStateTimeout = 180000;  // 3 minutes max per stage (safety timeout)

// State Machine
enum ReflowState {
  IDLE,
  PREHEAT,
  SOAK,
  REFLOW,
  COOLDOWN,
  COMPLETE,
  ERROR_STATE,
  PID_TUNING
};

// Forward declarations (after enum)
void changeState(ReflowState newState);
void calculatePIDFromAutoTune();
void emergencyStopReflow();
void setupWiFi();
void setupCaptivePortal();
void setupWebServer();
void readTemperature();
void updatePIDTuningStateMachine();
void updateReflowStateMachine();
void logDataPoint();
void updateLED();
void setLED(bool state);

ReflowState currentState = IDLE;
String stateNames[] = {"IDLE", "PREHEAT", "SOAK", "REFLOW", "COOLDOWN", "COMPLETE", "ERROR", "PID_TUNING"};

// Timing
unsigned long reflowStartTime = 0;
unsigned long stateStartTime = 0;
unsigned long lastTempCheck = 0;
unsigned long tempCheckInterval = 1000;  // Check temperature every 1 second
unsigned long tempReachedTime = 0;  // Time when target temp was reached in current state

// LED Status Indicators
unsigned long lastLEDBlink = 0;
bool ledState = false;
enum LEDPattern {
  LED_OFF,           // Solid off - not initialized
  LED_SOLID,         // Solid on - connected and idle
  LED_SLOW_BLINK,    // 1 Hz - WiFi connecting or system starting
  LED_FAST_BLINK,    // 4 Hz - Active reflow
  LED_ERROR_BLINK    // 2 Hz - Error state
};
LEDPattern currentLEDPattern = LED_OFF;

// Safety
bool emergencyStop = false;
int thermocoupleErrorCount = 0;
const int maxThermocoupleErrors = 3;

// Data logging
struct DataPoint {
  unsigned long time;
  double temperature;
  double setpoint;
  ReflowState state;
};

const int maxDataPoints = 500;
DataPoint dataLog[maxDataPoints];
int dataLogIndex = 0;

// PID Auto-Tuning Variables
bool pidTuningActive = false;
int tuningStep = 0;  // 0=heat to temp1, 1=oscillate at temp1, 2=heat to temp2, etc.
double tuningTargetTemp[] = {100.0, 150.0, 200.0};  // Test temperatures
int tuningTempIndex = 0;
double tuningAmplitude = 5.0;  // Temperature swing for oscillation test
unsigned long tuningStartTime = 0;
unsigned long tuningStepStartTime = 0;
unsigned long oscillationStartTime = 0;
double lastPeakTemp = 0;
double lastValleyTemp = 0;
unsigned long lastPeakTime = 0;
unsigned long prevPeakTime = 0;  // For peak-to-peak period measurement
unsigned long lastValleyTime = 0;
int oscillationCount = 0;
double sumOscillationPeriod = 0;
double sumOscillationAmplitude = 0;
bool waitingForPeak = true;
// Per-step oscillation accumulators (reset each temperature)
double stepSumPeriod = 0;
double stepSumAmplitude = 0;
int stepOscillationCount = 0;
double tuningKu = 0;  // Ultimate gain
double tuningPu = 0;  // Ultimate period
double calculatedKp = 0;
double calculatedKi = 0;
double calculatedKd = 0;

// Console Log (for PID tuning output)
const int maxConsoleLines = 100;
String consoleLog[maxConsoleLines];
int consoleLogIndex = 0;

void addConsoleLog(String message) {
  if (consoleLogIndex < maxConsoleLines) {
    consoleLog[consoleLogIndex] = message;
    consoleLogIndex++;
  } else {
    // Shift array and add new message at end
    for (int i = 0; i < maxConsoleLines - 1; i++) {
      consoleLog[i] = consoleLog[i + 1];
    }
    consoleLog[maxConsoleLines - 1] = message;
  }
  Serial.println("[CONSOLE] " + message);
}

// LittleFS Configuration Storage Functions
// Uses onboard flash filesystem instead of EEPROM - more reliable on Pico W
#define CONFIG_FILE "/config.json"

void loadConfigFromEEPROM() {
  Serial.println("Initializing LittleFS...");
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS, formatting...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("ERROR: LittleFS format failed!");
      return;
    }
  }
  Serial.println("LittleFS mounted successfully");

  // Check if config file exists
  if (!LittleFS.exists(CONFIG_FILE)) {
    Serial.println("No configuration file found, using defaults");
    return;
  }

  // Open and read config file
  File configFile = LittleFS.open(CONFIG_FILE, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return;
  }

  Serial.println("Loading configuration from flash...");

  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();

  if (error) {
    Serial.print("Failed to parse config: ");
    Serial.println(error.c_str());
    return;
  }

  // Load reflow parameters
  preheatTemp = doc["preheatTemp"] | preheatTemp;
  soakTemp = doc["soakTemp"] | soakTemp;
  reflowTemp = doc["reflowTemp"] | reflowTemp;
  cooldownTemp = doc["cooldownTemp"] | cooldownTemp;
  preheatTime = doc["preheatTime"] | preheatTime;
  soakTime = doc["soakTime"] | soakTime;
  reflowTime = doc["reflowTime"] | reflowTime;
  cooldownTime = doc["cooldownTime"] | cooldownTime;

  // Load PID parameters
  Kp = doc["Kp"] | Kp;
  Ki = doc["Ki"] | Ki;
  Kd = doc["Kd"] | Kd;
  myPID.SetTunings(Kp, Ki, Kd);

  // Load WiFi credentials
  const char* ssid = doc["wifiSSID"];
  const char* password = doc["wifiPassword"];
  if (ssid) {
    strncpy(storedSSID, ssid, sizeof(storedSSID) - 1);
    storedSSID[sizeof(storedSSID) - 1] = '\0';
  }
  if (password) {
    strncpy(storedPassword, password, sizeof(storedPassword) - 1);
    storedPassword[sizeof(storedPassword) - 1] = '\0';
  }

  Serial.println("Configuration loaded successfully from flash!");
  Serial.print("PID Values - Kp: "); Serial.print(Kp);
  Serial.print(", Ki: "); Serial.print(Ki);
  Serial.print(", Kd: "); Serial.println(Kd);
  if (strlen(storedSSID) > 0) {
    Serial.print("Stored WiFi SSID: ");
    Serial.println(storedSSID);
  }
}

void saveConfigToEEPROM() {
  Serial.println("Saving configuration to flash...");

  // Ensure LittleFS is mounted
  if (!LittleFS.begin()) {
    Serial.println("ERROR: LittleFS not mounted!");
    return;
  }

  // Create JSON document
  JsonDocument doc;

  // Save reflow parameters
  doc["preheatTemp"] = preheatTemp;
  doc["soakTemp"] = soakTemp;
  doc["reflowTemp"] = reflowTemp;
  doc["cooldownTemp"] = cooldownTemp;
  doc["preheatTime"] = preheatTime;
  doc["soakTime"] = soakTime;
  doc["reflowTime"] = reflowTime;
  doc["cooldownTime"] = cooldownTime;

  // Save PID parameters
  doc["Kp"] = Kp;
  doc["Ki"] = Ki;
  doc["Kd"] = Kd;

  // Save WiFi credentials
  doc["wifiSSID"] = storedSSID;
  doc["wifiPassword"] = storedPassword;

  // Write to file
  File configFile = LittleFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  if (serializeJson(doc, configFile) == 0) {
    Serial.println("Failed to write config");
  } else {
    Serial.println("Configuration saved to flash successfully!");
    Serial.print("Saved WiFi SSID: ");
    Serial.println(storedSSID);
  }

  configFile.close();
}

// LED Control Functions (Pico W uses CYW43 chip LED)
void setLED(bool state) {
  // On Pico W, LED is controlled via the WiFi chip
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
}

void updateLED() {
  unsigned long currentTime = millis();
  unsigned long interval = 0;

  // Determine blink pattern based on system state
  switch (currentState) {
    case ERROR_STATE:
      currentLEDPattern = LED_ERROR_BLINK;
      interval = 500;  // 2 Hz (500ms on, 500ms off)
      break;

    case PID_TUNING:
    case PREHEAT:
    case SOAK:
    case REFLOW:
    case COOLDOWN:
      currentLEDPattern = LED_FAST_BLINK;
      interval = 250;  // 4 Hz (250ms on, 250ms off)
      break;

    case COMPLETE:
      currentLEDPattern = LED_SLOW_BLINK;
      interval = 1000;  // 1 Hz (1s on, 1s off)
      break;

    case IDLE:
    default:
      currentLEDPattern = LED_SOLID;
      interval = 0;  // Solid on
      break;
  }

  // Update LED based on pattern
  if (interval == 0) {
    // Solid on
    setLED(true);
  } else {
    // Blink at specified interval
    if (currentTime - lastLEDBlink >= interval) {
      lastLEDBlink = currentTime;
      ledState = !ledState;
      setLED(ledState);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);  // Wait for serial connection
  Serial.println("\n\n=================================");
  Serial.println("Solder Reflow Oven Controller");
  Serial.println("Pico W + MAX31855 + Web Interface");
  Serial.println("=================================\n");

  // Initialize onboard LED (Pico W uses LED_BUILTIN for CYW43 LED)
  pinMode(LED_BUILTIN, OUTPUT);
  setLED(false);  // Start with LED off
  currentLEDPattern = LED_SLOW_BLINK;  // Blink during startup
  Serial.println("Onboard LED initialized");

  // Initialize SSR pin
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);
  Serial.println("SSR pin initialized (OFF)");

  // Initialize MAX31855
  Serial.println("Initializing MAX31855 thermocouple...");

  // CRITICAL FOR PICO W: Explicitly configure GPIO pins before Adafruit library uses them
  // The software SPI in Adafruit library may not configure pins properly on RP2040
  pinMode(THERMO_CLK, OUTPUT);
  pinMode(THERMO_CS, OUTPUT);
  pinMode(THERMO_DO, INPUT);
  digitalWrite(THERMO_CS, HIGH);  // CS starts high (inactive)
  Serial.println("MAX31855 GPIO pins configured");

  // Small delay to let pins settle
  delay(50);

  // CRITICAL: Call begin() to properly initialize SPI communication
  // This is required on Pico W (worked without on ESP8266 due to different implementation)
  if (!maxthermo.begin()) {
    Serial.println("ERROR: Failed to initialize MAX31855!");
    Serial.println("Check wiring: GPIO 2->CLK, 3->CS, 4->DO, 3.3V->VCC, GND->GND");
    currentState = ERROR_STATE;
  } else {
    Serial.println("MAX31855 initialized successfully");

    // CRITICAL: MAX31855 needs significant warm-up time after power-on
    // Datasheet specifies 200ms conversion time + settling
    // False SHORT/VCC/GND errors are common if read too soon
    Serial.println("Waiting for MAX31855 to stabilize (500ms)...");
    delay(500);

    // First read - discard (may have stale data)
    maxthermo.readCelsius();
    delay(100);

    // Second read - actual test
    double testTemp = maxthermo.readCelsius();
    if (isnan(testTemp)) {
      Serial.println("WARNING: Initial temperature read returned NaN");
      Serial.println("Checking for faults...");
      uint8_t error = maxthermo.readError();
      if (error & 0x01) Serial.println("  - Thermocouple open circuit (not connected)");
      if (error & 0x02) Serial.println("  - Thermocouple short to GND (may be false positive)");
      if (error & 0x04) Serial.println("  - Thermocouple short to VCC (may be false positive)");
      if (error == 0) Serial.println("  - No fault detected, sensor may need more warm-up time");
      Serial.println("Will retry during operation...");
      // Don't immediately set ERROR_STATE - give it a chance to work after warm-up
    } else {
      Serial.print("MAX31855 OK - Current temp: ");
      Serial.print(testTemp);
      Serial.println(" °C");
    }
  }

  // Initialize PID
  myPID.SetOutputLimits(0, 1);
  myPID.SetMode(AUTOMATIC);
  Serial.println("PID controller initialized");

  // Load configuration from flash (initializes LittleFS)
  loadConfigFromEEPROM();

  // CRITICAL: After LittleFS init, give MAX31855 time to stabilize
  // LittleFS mounting can cause brief SPI bus interference
  Serial.println("Re-stabilizing MAX31855 after LittleFS mount...");
  delay(200);

  // Perform a few dummy reads to clear any stale data
  for (int i = 0; i < 3; i++) {
    maxthermo.readCelsius();
    delay(100);
  }
  Serial.println("MAX31855 re-stabilized");

  // Setup WiFi
  setupWiFi();

  // Set LED to solid on once WiFi is connected
  currentLEDPattern = LED_SOLID;
  setLED(true);

  // Setup web server routes
  setupWebServer();


  Serial.println("\n=================================");
  Serial.println("Setup complete!");
  Serial.println("LED Status Indicators:");
  Serial.println("  Solid ON    = Connected & Idle");
  Serial.println("  Fast Blink  = Reflow Active (4 Hz)");
  Serial.println("  Slow Blink  = Reflow Complete (1 Hz)");
  Serial.println("  Medium Blink = ERROR State (2 Hz)");
  Serial.println("=================================\n");
}

void setupWiFi() {
  // Try to connect using stored credentials
  if (strlen(storedSSID) > 0) {
    Serial.print("Connecting to WiFi: ");
    Serial.println(storedSSID);
    WiFi.begin(storedSSID, storedPassword);
    
    // Increased timeout: 60 attempts × 500ms = 30 seconds
    // This gives DHCP servers enough time to respond
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 60) {
      delay(500);
      Serial.print(".");
      attempts++;

      // Show progress every 10 attempts (5 seconds)
      if (attempts % 10 == 0) {
        Serial.print(" [");
        Serial.print(attempts / 2);
        Serial.print("s] ");
      }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      return;
    }
    
    Serial.println("\nWiFi connection failed! Starting captive portal...");
    WiFi.disconnect();
    WiFi.end();
    delay(100);
  } else {
    Serial.println("No WiFi credentials stored. Starting captive portal...");
  }
  
  setupCaptivePortal();
}

void setupCaptivePortal() {
  Serial.println("Starting captive portal access point...");

  // Stop any existing WiFi connection first
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  // Set WiFi mode to Access Point
  WiFi.mode(WIFI_AP);
  delay(100);  // Give mode change time to take effect

  IPAddress apIP(192, 168, 4, 1);
  IPAddress netMask(255, 255, 255, 0);

  // Set AP IP configuration before starting the AP
  if (!WiFi.softAPConfig(apIP, apIP, netMask)) {
    Serial.println("ERROR: Failed to configure AP IP!");
  }

  // Start soft AP - no password (nullptr) so users can easily connect
  bool apStarted = WiFi.softAP(AP_SSID, nullptr, 1, 0, 4);  // channel 1, hidden=false, max_connections=4
  if (!apStarted) {
    Serial.println("ERROR: Failed to start Access Point!");
    Serial.println("Retrying with basic settings...");
    delay(500);
    apStarted = WiFi.softAP(AP_SSID);
    if (!apStarted) {
      Serial.println("ERROR: Access Point failed to start after retry!");
      return;
    }
  }

  delay(500);  // Increased delay - Allow AP to fully initialize

  Serial.print("Captive Portal SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Captive Portal IP:   ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Captive Portal MAC:  ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("Connect to '" + String(AP_SSID) + "' and visit http://" + apIP.toString() + " to configure WiFi");

  // Redirect all DNS queries to our IP so any URL opens the portal
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  captivePortalActive = true;

  Serial.println("Captive portal is READY and LISTENING");
}

// Forward declarations for web server handler functions
void handleRoot();
void handleStatus();
void handleGetConfig();
void handleSetConfig();
void handleStart();
void handleStop();
void handleDataLog();
void handleTunePID();
void handleGetConsoleLog();
void handleCaptivePortal();
void handleWiFiSave();
void handleNotFound();

void setupWebServer() {
  // Serve main page (or captive portal page if in setup mode)
  server.on("/", HTTP_GET, handleRoot);
  
  // Favicon handler (prevent 404 errors)
  server.on("/favicon.ico", HTTP_GET, []() {
    server.send(204);  // No content
  });

  // Captive portal WiFi configuration endpoints
  server.on("/wifi", HTTP_GET, handleCaptivePortal);
  server.on("/wifi/save", HTTP_POST, handleWiFiSave);
  
  // Captive portal detection URLs used by various OS/browsers
  server.on("/generate_204", HTTP_GET, handleCaptivePortal);         // Android
  server.on("/fwlink", HTTP_GET, handleCaptivePortal);               // Windows
  server.on("/hotspot-detect.html", HTTP_GET, handleCaptivePortal);  // Apple
  server.on("/connecttest.txt", HTTP_GET, handleCaptivePortal);      // Windows
  server.on("/ncsi.txt", HTTP_GET, handleCaptivePortal);             // Windows
  
  // API endpoints (only relevant when WiFi is connected in station mode)
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handleSetConfig);
  server.on("/api/start", HTTP_POST, handleStart);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/data", HTTP_GET, handleDataLog);
  server.on("/api/tune-pid", HTTP_POST, handleTunePID);
  server.on("/api/console", HTTP_GET, handleGetConsoleLog);
  
  // Catch-all: redirect unknown paths to root (needed for captive portal)
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started on port 80");
}

void loop() {
  // Process DNS queries when captive portal is active
  if (captivePortalActive) {
    dnsServer.processNextRequest();
  }
  
  server.handleClient();

  // Update LED status indicator
  updateLED();

  // Read temperature periodically
  unsigned long currentTime = millis();
  if (currentTime - lastTempCheck >= tempCheckInterval) {
    lastTempCheck = currentTime;
    readTemperature();
    
    // Update state machine based on current state
    if (currentState == PID_TUNING) {
      updatePIDTuningStateMachine();
    } else if (currentState != IDLE && currentState != COMPLETE && currentState != ERROR_STATE) {
      updateReflowStateMachine();
    }
    
    // Log data point
    logDataPoint();
  }
}

void readTemperature() {
  // MAX31855 requires minimum 220ms for full conversion cycle
  // Being conservative with 250ms to prevent false fault flags
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastReadTime < 250) {
    // Too soon, use previous value
    return;
  }
  lastReadTime = currentTime;

  double newTemp = maxthermo.readCelsius();

  if (isnan(newTemp)) {
    // First read returned an error. On the Pico W the fast processor can cause
    // transient SPI timing glitches that produce false fault flags in the
    // MAX31855. Retry after a short delay to distinguish real faults from
    // one-shot timing artifacts before counting this against the error limit.
    delay(50);
    newTemp = maxthermo.readCelsius();

    if (!isnan(newTemp)) {
      // Retry succeeded — this was a transient SPI timing glitch, not a real
      // fault. Accept the recovered reading and skip error counting.
      Serial.println("WARNING: Transient thermocouple read error (recovered - SPI glitch)");
      addConsoleLog("WARNING: Transient thermocouple error (recovered - SPI glitch)");
      Input = newTemp;
      return;
    }

    // Retry also failed — treat as a genuine error.
    thermocoupleErrorCount++;

    // Read error flags from MAX31855
    uint8_t error = maxthermo.readError();

    Serial.print("WARNING: Thermocouple read error! Error code: 0x");
    Serial.print(error, HEX);
    Serial.print(" - ");

    // Decode error
    if (error & 0x01) {
      Serial.println("Thermocouple OPEN (not connected or broken)");
    } else if (error & 0x02) {
      Serial.println("Thermocouple SHORT to GND (may be false - check SPI timing)");
    } else if (error & 0x04) {
      Serial.println("Thermocouple SHORT to VCC (may be false - check SPI timing)");
    } else if (error == 0) {
      Serial.println("No fault bits set - possible SPI communication issue");
    } else {
      Serial.println("Multiple faults detected");
    }

    // Log to console if available
    if (thermocoupleErrorCount == 1) {
      addConsoleLog("WARNING: Thermocouple read error!");
      if (error & 0x01) {
        addConsoleLog("  Error: Thermocouple OPEN (not connected)");
      } else if (error & 0x02) {
        addConsoleLog("  Error: SHORT to GND (SPI timing issue?)");
      } else if (error & 0x04) {
        addConsoleLog("  Error: SHORT to VCC (SPI timing issue?)");
      } else if (error == 0) {
        addConsoleLog("  Error: SPI communication problem");
        addConsoleLog("  Verify GPIO pins: 2->CLK, 3->CS, 4->DO");
      }
    }

    if (thermocoupleErrorCount >= maxThermocoupleErrors) {
      Serial.println("CRITICAL: Multiple thermocouple errors - EMERGENCY STOP!");
      addConsoleLog("CRITICAL: Multiple thermocouple errors!");
      addConsoleLog("If multimeter shows no short, this is SPI timing issue");
      addConsoleLog("Check MAX31855 wiring:");
      addConsoleLog("  GPIO 2 -> CLK");
      addConsoleLog("  GPIO 3 -> CS");
      addConsoleLog("  GPIO 4 -> DO (MISO)");
      addConsoleLog("  3.3V -> VCC, GND -> GND");
      if (error & 0x01) {
        addConsoleLog("FAULT: Thermocouple OPEN - Is it plugged in?");
      }
      emergencyStopReflow();
      currentState = ERROR_STATE;
    }
  } else {
    Input = newTemp;
    thermocoupleErrorCount = 0;  // Reset error counter on successful read
  }
}

void updateReflowStateMachine() {
  if (emergencyStop) {
    return;
  }
  
  unsigned long stateElapsed = millis() - stateStartTime;
  
  switch (currentState) {
    case IDLE:
      // Do nothing in IDLE state - waiting for user to start
      break;

    case PREHEAT:
      Setpoint = preheatTemp;
      if (useTemperatureBasedTransitions) {
        // Wait until temperature is reached OR time/timeout expires
        if ((Input >= preheatTemp - tempReachThreshold && stateElapsed >= preheatTime) || 
            stateElapsed >= maxStateTimeout) {
          changeState(SOAK);
        }
      } else {
        // Original time-based only
        if (stateElapsed >= preheatTime) {
          changeState(SOAK);
        }
      }
      break;
      
    case SOAK:
      Setpoint = soakTemp;
      if (useTemperatureBasedTransitions) {
        // Wait until temperature is reached OR time/timeout expires
        if ((Input >= soakTemp - tempReachThreshold && stateElapsed >= soakTime) || 
            stateElapsed >= maxStateTimeout) {
          changeState(REFLOW);
        }
      } else {
        // Original time-based only
        if (stateElapsed >= soakTime) {
          changeState(REFLOW);
        }
      }
      break;
      
    case REFLOW:
      Setpoint = reflowTemp;
      if (useTemperatureBasedTransitions) {
        // CRITICAL FIX: Wait until temperature is reached AND time requirement met
        // This ensures we stay at reflow temp for the required duration
        if (Input >= reflowTemp - tempReachThreshold) {
          // Temperature reached
          if (tempReachedTime == 0) {
            tempReachedTime = millis();
            Serial.print("REFLOW temperature reached at ");
            Serial.print(Input);
            Serial.println("°C - starting reflow timer");
          }
          // Now wait for the full reflow time from when temp was reached
          if (millis() - tempReachedTime >= reflowTime) {
            changeState(COOLDOWN);
          }
        } else if (stateElapsed >= maxStateTimeout) {
          // Safety timeout - transition anyway
          Serial.println("WARNING: REFLOW timeout - temperature not reached!");
          changeState(COOLDOWN);
        }
      } else {
        // Original time-based only
        if (stateElapsed >= reflowTime) {
          changeState(COOLDOWN);
        }
      }
      break;
      
    case COOLDOWN:
      Setpoint = cooldownTemp;
      digitalWrite(SSR_PIN, LOW);  // Force heater off during cooldown
      if (Input <= cooldownTemp + 10 || stateElapsed >= cooldownTime) {
        changeState(COMPLETE);
      }
      break;
      
    case COMPLETE:
      Setpoint = cooldownTemp;
      digitalWrite(SSR_PIN, LOW);
      break;

    case ERROR_STATE:
      // In error state, turn off heater and wait for user intervention
      Setpoint = 0;
      digitalWrite(SSR_PIN, LOW);
      break;

    case PID_TUNING:
      // PID tuning is handled separately, just ensure safety
      digitalWrite(SSR_PIN, pidTuningActive ? HIGH : LOW);
      break;
  }
  
  // Run PID (only during active heating states, not during cooldown, complete, or error)
  if (currentState != COOLDOWN && currentState != COMPLETE && currentState != ERROR_STATE) {
    myPID.Compute();
    
    // Control SSR based on PID output
    if (Output > 0.5) {
      digitalWrite(SSR_PIN, HIGH);
    } else {
      digitalWrite(SSR_PIN, LOW);
    }
  } else {
    // Ensure SSR is off in safe states
    digitalWrite(SSR_PIN, LOW);
  }
  
  // Print status
  Serial.print(stateNames[currentState]);
  Serial.print(" | Time: ");
  Serial.print(stateElapsed / 1000);
  Serial.print("s | Temp: ");
  Serial.print(Input);
  Serial.print("°C | Setpoint: ");
  Serial.print(Setpoint);
  Serial.print("°C | Output: ");
  Serial.print(Output);
  Serial.print(" | SSR: ");
  Serial.println(digitalRead(SSR_PIN) ? "ON" : "OFF");
}

void changeState(ReflowState newState) {
  Serial.print("State change: ");
  Serial.print(stateNames[currentState]);
  Serial.print(" -> ");
  Serial.println(stateNames[newState]);
  
  currentState = newState;
  stateStartTime = millis();
  tempReachedTime = 0;  // Reset temperature reached tracker for new state

  // Reset setpoint when entering IDLE
  if (newState == IDLE) {
    Setpoint = 0;
  }
}

void startReflow() {
  if (currentState == IDLE || currentState == COMPLETE || currentState == ERROR_STATE) {
    Serial.println("Starting reflow process...");
    emergencyStop = false;
    thermocoupleErrorCount = 0;
    reflowStartTime = millis();
    dataLogIndex = 0;  // Reset data log
    
    // Reset PID controller for fresh start
    // This reinitializes the integral term and prevents stale state from affecting the new cycle
    Output = 0;
    myPID.SetMode(MANUAL);
    myPID.SetMode(AUTOMATIC);
    Serial.println("PID controller reset for new reflow cycle");
    
    changeState(PREHEAT);
  }
}

void stopReflow() {
  Serial.println("Stopping reflow process...");
  emergencyStopReflow();
  currentState = IDLE;
}

void startPIDTuning() {
  if (currentState == IDLE || currentState == COMPLETE || currentState == ERROR_STATE) {
    addConsoleLog("=== PID Auto-Tuning Started ===");
    addConsoleLog("This will test at 3 temperatures: 100C, 150C, 200C");
    addConsoleLog("Each test creates controlled oscillations to measure system response");
    addConsoleLog("");
    
    pidTuningActive = true;
    emergencyStop = false;
    thermocoupleErrorCount = 0;
    tuningStep = 0;
    tuningTempIndex = 0;
    tuningStartTime = millis();
    tuningStepStartTime = millis();
    oscillationCount = 0;
    sumOscillationPeriod = 0;
    sumOscillationAmplitude = 0;
    stepSumPeriod = 0;
    stepSumAmplitude = 0;
    stepOscillationCount = 0;
    prevPeakTime = 0;
    consoleLogIndex = 0;  // Clear console for new tuning session
    dataLogIndex = 0;  // Reset data log
    
    // Reset PID controller
    Output = 0;
    myPID.SetMode(MANUAL);
    
    currentState = PID_TUNING;
    addConsoleLog("Step 1/6: Heating to " + String(tuningTargetTemp[0]) + "C...");
  }
}

void stopPIDTuning() {
  addConsoleLog("PID Tuning stopped by user");
  pidTuningActive = false;
  digitalWrite(SSR_PIN, LOW);
  Output = 0;
  Setpoint = 0;  // Reset setpoint when returning to IDLE
  myPID.SetMode(AUTOMATIC);
  currentState = IDLE;
}

void updatePIDTuningStateMachine() {
  if (!pidTuningActive || emergencyStop) {
    return;
  }
  
  unsigned long elapsed = millis() - tuningStepStartTime;
  double targetTemp = tuningTargetTemp[tuningTempIndex];
  
  // Step 0, 2, 4: Heat to target temperature
  if (tuningStep % 2 == 0) {
    Setpoint = targetTemp;
    
    // Simple bang-bang control to reach temperature quickly
    if (Input < targetTemp - 2.0) {
      digitalWrite(SSR_PIN, HIGH);
      Output = 1;
    } else if (Input >= targetTemp - 2.0) {
      // Temperature reached, move to oscillation test
      addConsoleLog("Old PID values -> Kp: " + String(Kp, 3) + ", Ki: " + String(Ki, 3) + ", Kd: " + String(Kd, 3));
      tuningStep++;
      tuningStepStartTime = millis();
      oscillationStartTime = millis();
      stepSumPeriod = 0;
      stepSumAmplitude = 0;
      stepOscillationCount = 0;
      prevPeakTime = 0;
      waitingForPeak = true;
      lastPeakTemp = Input;
      lastValleyTemp = Input;
      
      addConsoleLog("Target " + String(targetTemp) + "C reached. Starting oscillation test...");
      addConsoleLog("Step " + String(tuningStep/2 + 1) + "/6: Testing at " + String(targetTemp) + "C");
    }
  }
  // Step 1, 3, 5: Oscillation test
  else {
    // Relay-based oscillation: switch SSR on/off to create oscillations
    if (Input < targetTemp - tuningAmplitude) {
      digitalWrite(SSR_PIN, HIGH);
      Output = 1;
      
      // Detect valley
      if (!waitingForPeak && Input < lastValleyTemp) {
        lastValleyTemp = Input;
        lastValleyTime = millis();
      } else if (!waitingForPeak && Input > lastValleyTemp + 1.0) {
        // Valley confirmed, now wait for peak
        waitingForPeak = true;
        lastPeakTemp = Input;
      }
    } else if (Input > targetTemp + tuningAmplitude) {
      digitalWrite(SSR_PIN, LOW);
      Output = 0;
      
      // Detect peak
      if (waitingForPeak && Input > lastPeakTemp) {
        lastPeakTemp = Input;
        lastPeakTime = millis();
      } else if (waitingForPeak && Input < lastPeakTemp - 1.0) {
        // Peak confirmed — use peak-to-peak period for accurate Pu measurement
        if (prevPeakTime > 0) {
          unsigned long period = lastPeakTime - prevPeakTime;  // Peak-to-peak
          double amplitude = lastPeakTemp - lastValleyTemp;
          
          if (period > 1000 && period < 300000 && amplitude > 1.0) {  // Sanity checks
            stepSumPeriod += period;
            stepSumAmplitude += amplitude;
            stepOscillationCount++;
            sumOscillationPeriod += period;
            sumOscillationAmplitude += amplitude;
            oscillationCount++;
            
            addConsoleLog("  Oscillation #" + String(oscillationCount) + 
                         ": Period=" + String(period/1000.0, 1) + "s, Amplitude=" + 
                         String(amplitude, 1) + "C");
          }
        }
        
        prevPeakTime = lastPeakTime;  // Save for next peak-to-peak calculation
        waitingForPeak = false;
        lastValleyTemp = Input;
      }
    }
    
    // Need at least 3 oscillations per step, or timeout after 10 minutes
    if (stepOscillationCount >= 3 || elapsed > 600000) {
      if (stepOscillationCount >= 3) {
        addConsoleLog("Oscillation test complete at " + String(targetTemp) + "C");
      } else {
        addConsoleLog("Timeout - moving to next temperature");
      }
      
      // Log per-step PID estimate if we have data for this step
      if (stepOscillationCount >= 1) {
        double stepAvgPeriod = stepSumPeriod / stepOscillationCount / 1000.0;  // seconds
        double stepAvgAmplitude = stepSumAmplitude / stepOscillationCount;
        double stepKu = (4.0 * 1.0) / (PI * stepAvgAmplitude);
        double stepKp = 0.6 * stepKu;
        double stepKi = 1.2 * stepKu / stepAvgPeriod;
        double stepKd = 0.075 * stepKu * stepAvgPeriod;
        addConsoleLog("  Step PID estimate @ " + String(targetTemp, 0) + "C:");
        addConsoleLog("    New Kp: " + String(stepKp, 3) + 
                      "  Ki: " + String(stepKi, 3) + 
                      "  Kd: " + String(stepKd, 3));
      }
      addConsoleLog("");
      
      // Move to next temperature or complete
      tuningTempIndex++;
      if (tuningTempIndex < 3) {
        tuningStep++;
        tuningStepStartTime = millis();
        addConsoleLog("Step " + String(tuningStep/2 + 2) + "/6: Heating to " + 
                     String(tuningTargetTemp[tuningTempIndex]) + "C...");
      } else {
        // All tests complete, calculate PID values
        calculatePIDFromAutoTune();
        pidTuningActive = false;
        digitalWrite(SSR_PIN, LOW);
        Output = 0;
        Setpoint = 0;  // Reset setpoint when returning to IDLE
        myPID.SetMode(AUTOMATIC);
        currentState = IDLE;
      }
    }
  }
}

void calculatePIDFromAutoTune() {
  addConsoleLog("");
  addConsoleLog("=== Calculating PID Parameters ===");
  
  if (oscillationCount < 1) {
    addConsoleLog("ERROR: Not enough oscillation data collected");
    addConsoleLog("Current PID values unchanged");
    addConsoleLog("Old Kp: " + String(Kp, 3) + "  Ki: " + String(Ki, 3) + "  Kd: " + String(Kd, 3));
    return;
  }
  
  // Calculate average period and amplitude
  double avgPeriod = sumOscillationPeriod / oscillationCount / 1000.0;  // Convert to seconds
  double avgAmplitude = sumOscillationAmplitude / oscillationCount;
  
  addConsoleLog("Average oscillation period: " + String(avgPeriod, 2) + " seconds");
  addConsoleLog("Average amplitude: " + String(avgAmplitude, 2) + " C");
  
  // For relay method, estimate ultimate gain and period
  // Ku ≈ (4 * relay_amplitude) / (π * oscillation_amplitude)
  double relayAmplitude = 1.0;  // Our output is 0 or 1
  tuningKu = (4.0 * relayAmplitude) / (PI * avgAmplitude);
  tuningPu = avgPeriod;
  
  addConsoleLog("Ultimate gain (Ku): " + String(tuningKu, 3));
  addConsoleLog("Ultimate period (Pu): " + String(tuningPu, 2) + "s");
  addConsoleLog("");
  
  // Calculate PID using Ziegler-Nichols method
  // Classic Ziegler-Nichols tuning rules:
  // Kp = 0.6 * Ku
  // Ki = 1.2 * Ku / Pu  (or Ti = Pu / 2)
  // Kd = 0.075 * Ku * Pu  (or Td = Pu / 8)
  calculatedKp = 0.6 * tuningKu;
  calculatedKi = 1.2 * tuningKu / tuningPu;
  calculatedKd = 0.075 * tuningKu * tuningPu;
  
  addConsoleLog("=== PID Values: Old vs New (Ziegler-Nichols) ===");
  addConsoleLog("         Old      New");
  addConsoleLog("  Kp:  " + String(Kp, 3) + "  ->  " + String(calculatedKp, 3));
  addConsoleLog("  Ki:  " + String(Ki, 3) + "  ->  " + String(calculatedKi, 3));
  addConsoleLog("  Kd:  " + String(Kd, 3) + "  ->  " + String(calculatedKd, 3));
  addConsoleLog("");
  addConsoleLog("Applying new PID values...");
  Kp = calculatedKp;
  Ki = calculatedKi;
  Kd = calculatedKd;
  myPID.SetTunings(Kp, Ki, Kd);
  saveConfigToEEPROM();
  addConsoleLog("PID values saved to flash.");
  addConsoleLog("");
  addConsoleLog("=== Auto-Tuning Complete ===");
}

void emergencyStopReflow() {
  digitalWrite(SSR_PIN, LOW);
  emergencyStop = true;
  Output = 0;
  Setpoint = cooldownTemp;
  Serial.println("EMERGENCY STOP - SSR turned OFF");
}

void logDataPoint() {
  if (dataLogIndex < maxDataPoints) {
    dataLog[dataLogIndex].time = millis() - reflowStartTime;
    dataLog[dataLogIndex].temperature = Input;
    dataLog[dataLogIndex].setpoint = Setpoint;
    dataLog[dataLogIndex].state = currentState;
    dataLogIndex++;
  }
}

// ==================== Web Server Handlers ====================

void handleRoot() {
  // If we're in captive portal mode, redirect to WiFi setup page
  if (captivePortalActive) {
    Serial.println("HTTP: / requested - redirecting to /wifi (captive portal)");
    server.sendHeader("Location", "/wifi", true);
    server.send(302, "text/plain", "");
    return;
  }

  Serial.println("HTTP: / requested - serving main page");

  // Add cache control headers to prevent browser caching
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");

  String html = F(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Reflow Oven Controller</title>
  <!-- Cache buster: v1.1 - Force reload after updates -->
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 1200px;
      margin: 0 auto;
    }
    .card {
      background: white;
      border-radius: 10px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }
    h1 {
      color: white;
      text-align: center;
      margin-bottom: 20px;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    h2 {
      color: #333;
      margin-bottom: 15px;
      border-bottom: 2px solid #667eea;
      padding-bottom: 10px;
    }
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 15px;
      margin-bottom: 20px;
    }
    .status-item {
      background: #f8f9fa;
      padding: 15px;
      border-radius: 8px;
      border-left: 4px solid #667eea;
    }
    .status-label {
      font-size: 0.9em;
      color: #666;
      margin-bottom: 5px;
    }
    .status-value {
      font-size: 1.5em;
      font-weight: bold;
      color: #333;
    }
    .temp-display {
      font-size: 3em !important;
      color: #667eea;
    }
    .controls {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
    }
    button {
      flex: 1;
      min-width: 120px;
      padding: 15px 30px;
      font-size: 1.1em;
      font-weight: bold;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      transition: all 0.3s;
    }
    .btn-start {
      background: #28a745;
      color: white;
    }
    .btn-start:hover { background: #218838; }
    .btn-stop {
      background: #dc3545;
      color: white;
    }
    .btn-stop:hover { background: #c82333; }
    .btn-stop:disabled, .btn-start:disabled {
      opacity: 0.5;
      cursor: not-allowed;
    }
    .config-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 15px;
    }
    .config-item {
      display: flex;
      flex-direction: column;
    }
    .config-item label {
      font-weight: bold;
      margin-bottom: 5px;
      color: #555;
    }
    .config-item input {
      padding: 10px;
      border: 2px solid #ddd;
      border-radius: 5px;
      font-size: 1em;
    }
    .config-item input:focus {
      outline: none;
      border-color: #667eea;
    }
    .btn-save {
      background: #007bff;
      color: white;
      margin-top: 15px;
    }
    .btn-save:hover { background: #0056b3; }
    #chartContainer {
      position: relative;
      height: 400px;
      margin: 20px 0;
    }
    .pid-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 15px;
    }
    .help-text {
      font-size: 0.85em;
      color: #666;
      margin-top: 5px;
      font-style: italic;
    }
    .state-IDLE { color: #6c757d; }
    .state-PREHEAT { color: #fd7e14; }
    .state-SOAK { color: #ffc107; }
    .state-REFLOW { color: #dc3545; }
    .state-COOLDOWN { color: #007bff; }
    .state-COMPLETE { color: #28a745; }
    .state-ERROR { color: #dc3545; font-weight: bold; }
    .ssr-indicator {
      display: inline-block;
      width: 20px;
      height: 20px;
      border-radius: 50%;
      margin-left: 10px;
      vertical-align: middle;
    }
    .ssr-on { background: #28a745; box-shadow: 0 0 10px #28a745; }
    .ssr-off { background: #6c757d; }
    .btn-tune {
      background: #6f42c1;
      color: white;
    }
    .btn-tune:hover { background: #5a32a3; }
    .console-container {
      background: #1e1e1e;
      color: #d4d4d4;
      font-family: 'Courier New', monospace;
      font-size: 0.9em;
      padding: 15px;
      border-radius: 5px;
      max-height: 400px;
      overflow-y: auto;
      margin-top: 15px;
      display: block;
    }
    .console-container.visible {
      display: block;
    }
    .console-container.hidden {
      display: none;
    }
    .console-line {
      margin: 2px 0;
      white-space: pre-wrap;
      word-wrap: break-word;
    }
    .console-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 10px;
    }
    .btn-toggle-console {
      background: #6c757d;
      color: white;
      font-size: 0.9em;
      padding: 5px 15px;
    }
    .btn-toggle-console:hover { background: #5a6268; }
    .state-PID_TUNING { color: #6f42c1; font-weight: bold; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔥 Solder Reflow Oven Controller</h1>
    
    <div class="card">
      <h2>Status</h2>
      <div class="status-grid">
        <div class="status-item">
          <div class="status-label">Current Temperature</div>
          <div class="status-value temp-display" id="currentTemp">--</div>
        </div>
        <div class="status-item">
          <div class="status-label">Target Temperature</div>
          <div class="status-value" id="targetTemp">--</div>
        </div>
        <div class="status-item">
          <div class="status-label">State</div>
          <div class="status-value" id="state">IDLE</div>
        </div>
        <div class="status-item">
          <div class="status-label">Time Elapsed</div>
          <div class="status-value" id="timeElapsed">0s</div>
        </div>
        <div class="status-item">
          <div class="status-label">SSR Status</div>
          <div class="status-value">
            <span id="ssrStatus">OFF</span>
            <span class="ssr-indicator ssr-off" id="ssrIndicator"></span>
          </div>
        </div>
      </div>
      
      <div class="controls">
        <button class="btn-start" id="startBtn" onclick="startReflow()">START REFLOW</button>
        <button class="btn-stop" id="stopBtn" onclick="stopReflow()" disabled>EMERGENCY STOP</button>
      </div>
    </div>
    
    <div class="card">
      <h2>Reflow Profile Configuration</h2>
      <div class="config-grid">
        <div class="config-item">
          <label>Preheat Temp (°C)</label>
          <input type="number" id="preheatTemp" value="150" min="0" max="300">
        </div>
        <div class="config-item">
          <label>Preheat Time (sec)</label>
          <input type="number" id="preheatTime" value="90" min="0" max="300">
        </div>
        <div class="config-item">
          <label>Soak Temp (°C)</label>
          <input type="number" id="soakTemp" value="180" min="0" max="300">
        </div>
        <div class="config-item">
          <label>Soak Time (sec)</label>
          <input type="number" id="soakTime" value="90" min="0" max="300">
        </div>
        <div class="config-item">
          <label>Reflow Temp (°C)</label>
          <input type="number" id="reflowTemp" value="230" min="0" max="300">
        </div>
        <div class="config-item">
          <label>Reflow Time (sec)</label>
          <input type="number" id="reflowTime" value="40" min="0" max="300">
        </div>
      </div>
      <button class="btn-save" onclick="saveConfig()">Save Configuration</button>
    </div>
    
    <div class="card">
      <h2>📊 Temperature Chart</h2>
      <div id="chartContainer">
        <canvas id="tempChart"></canvas>
      </div>
    </div>
    
    <div class="card">
      <h2>⚙️ PID Tuning</h2>
      <div class="pid-grid">
        <div class="config-item">
          <label>Kp (Proportional)</label>
          <input type="number" id="Kp" value="1.0" step="0.1" min="0">
          <div class="help-text">Increase for faster response</div>
        </div>
        <div class="config-item">
          <label>Ki (Integral)</label>
          <input type="number" id="Ki" value="0.05" step="0.05" min="0">
          <div class="help-text">Eliminates steady-state error</div>
        </div>
        <div class="config-item">
          <label>Kd (Derivative)</label>
          <input type="number" id="Kd" value="5.0" step="0.1" min="0">
          <div class="help-text">Reduces overshoot</div>
        </div>
      </div>
      <button class="btn-save" onclick="savePID()">Save PID Settings</button>
      <button class="btn-tune" onclick="tunePID()" id="tuneBtn">🔧 Auto-Tune PID</button>
      
      <div class="console-header" style="margin-top: 20px;">
        <h3 style="margin: 0; color: #333;">Console Log</h3>
        <button class="btn-toggle-console" onclick="toggleConsole()">Show/Hide Console</button>
      </div>
      <div id="console" class="console-container">
        <div id="consoleContent"></div>
      </div>
    </div>
  </div>

  <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
  <script>
    let ws;
    let chart;
    let chartData = {
      labels: [],
      datasets: [
        {
          label: 'Temperature',
          data: [],
          borderColor: '#dc3545',
          backgroundColor: 'rgba(220, 53, 69, 0.1)',
          borderWidth: 2,
          tension: 0.4
        },
        {
          label: 'Setpoint',
          data: [],
          borderColor: '#667eea',
          backgroundColor: 'rgba(102, 126, 234, 0.1)',
          borderWidth: 2,
          borderDash: [5, 5],
          tension: 0.4
        }
      ]
    };
    
    function initChart() {
      const ctx = document.getElementById('tempChart').getContext('2d');
      chart = new Chart(ctx, {
        type: 'line',
        data: chartData,
        options: {
          responsive: true,
          maintainAspectRatio: false,
          scales: {
            y: {
              beginAtZero: true,
              title: {
                display: true,
                text: 'Temperature (°C)'
              }
            },
            x: {
              title: {
                display: true,
                text: 'Time (s)'
              }
            }
          },
          plugins: {
            legend: {
              display: true,
              position: 'top'
            }
          }
        }
      });
    }
    
    function updateChart(temp, setpoint, elapsed) {
      const timeInSeconds = Math.floor(elapsed / 1000);
      
      // Add new data point
      chartData.labels.push(timeInSeconds);
      chartData.datasets[0].data.push(temp);
      chartData.datasets[1].data.push(setpoint);
      
      // Keep only last 300 points (5 minutes at 1 sample/second WebSocket update rate)
      if (chartData.labels.length > 300) {
        chartData.labels.shift();
        chartData.datasets[0].data.shift();
        chartData.datasets[1].data.shift();
      }
      
      // Update chart without animation ('none' mode) for better real-time performance
      chart.update('none');
    }
    
    function clearChart() {
      chartData.labels = [];
      chartData.datasets[0].data = [];
      chartData.datasets[1].data = [];
      chart.update();
    }
    
    function connectWebSocket() {
      // Using HTTP polling instead of WebSocket for Pico W compatibility

      // Fetch status immediately on page load
      fetchStatus();

      // Then poll every 500ms
      setInterval(fetchStatus, 500);
    }

    function fetchStatus() {
      fetch('/api/status')
        .then(r => {
          if (!r.ok) throw new Error('HTTP ' + r.status);
          return r.json();
        })
        .then(data => {
          console.log('Status update:', data);  // Debug logging
          updateDisplay(data);
        })
        .catch(err => {
          console.error('Error fetching status:', err);
          // Show connection error in UI
          document.getElementById('currentTemp').textContent = 'OFFLINE';
          document.getElementById('targetTemp').textContent = '--';
        });
    }
    
    function updateDisplay(data) {
      // Handle NaN temperature gracefully
      const tempText = (data.temp !== null && !isNaN(data.temp)) ? data.temp.toFixed(1) + '°C' : 'ERROR';
      const setpointText = (data.setpoint !== null && !isNaN(data.setpoint)) ? data.setpoint.toFixed(1) + '°C' : '--';

      const tempEl = document.getElementById('currentTemp');
      tempEl.textContent = tempText;

      // Highlight error state
      if (tempText === 'ERROR') {
        tempEl.style.color = '#dc3545';
        tempEl.style.fontWeight = 'bold';
      } else {
        tempEl.style.color = '';
        tempEl.style.fontWeight = '';
      }

      document.getElementById('targetTemp').textContent = setpointText;

      const stateEl = document.getElementById('state');
      stateEl.textContent = data.state;
      stateEl.className = 'status-value state-' + data.state;
      
      document.getElementById('timeElapsed').textContent = Math.floor(data.elapsed / 1000) + 's';
      
      const ssrOn = data.ssr === 1;
      document.getElementById('ssrStatus').textContent = ssrOn ? 'ON' : 'OFF';
      document.getElementById('ssrIndicator').className = 'ssr-indicator ' + (ssrOn ? 'ssr-on' : 'ssr-off');
      
      // Update button states
      const isActive = data.state !== 'IDLE' && data.state !== 'COMPLETE' && data.state !== 'ERROR';
      const isTuning = data.state === 'PID_TUNING';
      const canStart = (data.state === 'IDLE' || data.state === 'COMPLETE' || data.state === 'ERROR');

      document.getElementById('startBtn').disabled = !canStart || isTuning;
      document.getElementById('stopBtn').disabled = !isActive;
      document.getElementById('tuneBtn').disabled = (isActive && !isTuning);
      
      if (isTuning) {
        document.getElementById('tuneBtn').textContent = '⏹ Stop Auto-Tune';
      } else {
        document.getElementById('tuneBtn').textContent = '🔧 Auto-Tune PID';
      }
      
      // Update chart if reflow or tuning is active
      if (isActive) {
        updateChart(data.temp, data.setpoint, data.elapsed);
      }
      
      // Clear chart when returning to IDLE
      if (data.state === 'IDLE' && chartData.labels.length > 0) {
        clearChart();
      }
    }
    
    function startReflow() {
      fetch('/api/start', { method: 'POST' })
        .then(r => r.json())
        .then(data => console.log('Reflow started:', data))
        .catch(err => console.error('Error:', err));
    }
    
    function stopReflow() {
      if (confirm('Are you sure you want to emergency stop the reflow process?')) {
        fetch('/api/stop', { method: 'POST' })
          .then(r => r.json())
          .then(data => console.log('Reflow stopped:', data))
          .catch(err => console.error('Error:', err));
      }
    }
    
    function saveConfig() {
      const config = {
        preheatTemp: parseFloat(document.getElementById('preheatTemp').value),
        preheatTime: parseInt(document.getElementById('preheatTime').value) * 1000,
        soakTemp: parseFloat(document.getElementById('soakTemp').value),
        soakTime: parseInt(document.getElementById('soakTime').value) * 1000,
        reflowTemp: parseFloat(document.getElementById('reflowTemp').value),
        reflowTime: parseInt(document.getElementById('reflowTime').value) * 1000
      };
      
      fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
      })
      .then(r => r.json())
      .then(data => {
        alert('Configuration saved successfully!');
        console.log('Config saved:', data);
      })
      .catch(err => {
        alert('Error saving configuration');
        console.error('Error:', err);
      });
    }
    
    function loadConfig() {
      fetch('/api/config')
        .then(r => r.json())
        .then(data => {
          document.getElementById('preheatTemp').value = data.preheatTemp;
          document.getElementById('preheatTime').value = data.preheatTime / 1000;
          document.getElementById('soakTemp').value = data.soakTemp;
          document.getElementById('soakTime').value = data.soakTime / 1000;
          document.getElementById('reflowTemp').value = data.reflowTemp;
          document.getElementById('reflowTime').value = data.reflowTime / 1000;
          
          // Load PID values
          if (data.Kp !== undefined) {
            document.getElementById('Kp').value = data.Kp;
            document.getElementById('Ki').value = data.Ki;
            document.getElementById('Kd').value = data.Kd;
          }
        })
        .catch(err => console.error('Error loading config:', err));
    }
    
    function savePID() {
      const config = {
        Kp: parseFloat(document.getElementById('Kp').value),
        Ki: parseFloat(document.getElementById('Ki').value),
        Kd: parseFloat(document.getElementById('Kd').value)
      };
      
      fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
      })
      .then(r => r.json())
      .then(data => {
        alert('PID settings saved successfully!');
        console.log('PID saved:', data);
      })
      .catch(err => {
        alert('Error saving PID settings');
        console.error('Error:', err);
      });
    }
    
    function tunePID() {
      const tuneBtn = document.getElementById('tuneBtn');
      const currentState = document.getElementById('state').textContent;
      
      if (currentState === 'PID_TUNING') {
        // Stop tuning
        if (confirm('Stop PID auto-tuning?')) {
          fetch('/api/tune-pid', { method: 'POST' })
            .then(r => r.json())
            .then(data => {
              console.log('Tuning stopped:', data);
              tuneBtn.textContent = '🔧 Auto-Tune PID';
            })
            .catch(err => console.error('Error:', err));
        }
      } else if (currentState === 'IDLE' || currentState === 'COMPLETE' || currentState === 'ERROR') {
        // Start tuning
        if (confirm('Start PID auto-tuning? This will take 15-30 minutes and heat the oven to test temperatures. Make sure the oven is empty.')) {
          fetch('/api/tune-pid', { method: 'POST' })
            .then(r => r.json())
            .then(data => {
              console.log('Tuning started:', data);
              tuneBtn.textContent = '⏹ Stop Auto-Tune';
              clearChart();
              document.getElementById('console').classList.add('visible');
              startConsolePolling();
            })
            .catch(err => console.error('Error:', err));
        }
      } else {
        alert('Cannot start PID tuning while reflow is active. Please wait for completion or stop the reflow.');
      }
    }
    
    function toggleConsole() {
      const consoleEl = document.getElementById('console');
      if (consoleEl.classList.contains('hidden')) {
        consoleEl.classList.remove('hidden');
        consoleEl.classList.add('visible');
      } else {
        consoleEl.classList.remove('visible');
        consoleEl.classList.add('hidden');
      }
    }
    
    let consolePolling = null;
    
    function startConsolePolling() {
      if (consolePolling) return;
      
      consolePolling = setInterval(() => {
        fetch('/api/console')
          .then(r => r.json())
          .then(data => {
            const consoleContent = document.getElementById('consoleContent');
            consoleContent.innerHTML = '';
            data.forEach(line => {
              const div = document.createElement('div');
              div.className = 'console-line';
              div.textContent = line;
              consoleContent.appendChild(div);
            });
            // Auto-scroll to bottom
            const consoleDiv = document.getElementById('console');
            consoleDiv.scrollTop = consoleDiv.scrollHeight;
            
            // Stop polling if not tuning
            const currentState = document.getElementById('state').textContent;
            if (currentState !== 'PID_TUNING') {
              stopConsolePolling();
              document.getElementById('tuneBtn').textContent = '🔧 Auto-Tune PID';
            }
          })
          .catch(err => console.error('Error fetching console:', err));
      }, 2000);  // Poll every 2 seconds
    }
    
    function stopConsolePolling() {
      if (consolePolling) {
        clearInterval(consolePolling);
        consolePolling = null;
      }
    }
    
    // Initialize
    initChart();
    connectWebSocket();
    loadConfig();
  </script>
</body>
</html>
)rawliteral");
  
  server.send(200, "text/html", html);
}

void handleCaptivePortal() {
  Serial.println("HTTP: /wifi requested - serving captive portal page");

  // Scan for available networks to populate the dropdown
  int networkCount = WiFi.scanNetworks();
  String networkOptions = "";
  if (networkCount > 0) {
    for (int i = 0; i < networkCount; i++) {
      String encLabel;
      uint8_t encType = WiFi.encryptionType(i);
      switch (encType) {
        case ENC_TYPE_NONE: encLabel = " (Open)"; break;
        case ENC_TYPE_WEP:  encLabel = " (WEP)";  break;
        case ENC_TYPE_TKIP: encLabel = " (WPA)";  break;
        case ENC_TYPE_CCMP: encLabel = " (WPA2)"; break;
        case ENC_TYPE_AUTO: encLabel = " (WPA/WPA2)"; break;
        default:            encLabel = " (Encrypted)";  break;
      }
      networkOptions += "<option value=\"" + String(WiFi.SSID(i)) + "\">" +
                        String(WiFi.SSID(i)) + encLabel + " (" + String(WiFi.RSSI(i)) + " dBm)</option>\n";
    }
  } else {
    networkOptions = "<option value=''>-- No networks found; enter SSID below --</option>\n";
  }

  String html = F(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Reflow Oven - WiFi Setup</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    .card {
      background: white;
      border-radius: 12px;
      padding: 30px;
      width: 100%;
      max-width: 420px;
      box-shadow: 0 8px 24px rgba(0,0,0,0.2);
    }
    h1 { color: #333; margin-bottom: 8px; font-size: 1.6em; }
    p.subtitle { color: #666; margin-bottom: 24px; font-size: 0.95em; }
    label { display: block; font-weight: bold; color: #555; margin-bottom: 6px; }
    select, input[type="text"], input[type="password"] {
      width: 100%;
      padding: 10px 12px;
      border: 2px solid #ddd;
      border-radius: 6px;
      font-size: 1em;
      margin-bottom: 16px;
      transition: border-color 0.2s;
    }
    select:focus, input:focus { outline: none; border-color: #667eea; }
    button {
      width: 100%;
      padding: 14px;
      background: #667eea;
      color: white;
      border: none;
      border-radius: 6px;
      font-size: 1.1em;
      font-weight: bold;
      cursor: pointer;
      transition: background 0.2s;
    }
    button:hover { background: #5568d4; }
    .notice {
      margin-top: 16px;
      padding: 12px;
      background: #fff3cd;
      border-left: 4px solid #ffc107;
      border-radius: 4px;
      font-size: 0.9em;
      color: #856404;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>🔥 Reflow Oven Setup</h1>
    <p class="subtitle">Connect your oven to a WiFi network to access the control interface.</p>
    <form method="POST" action="/wifi/save">
      <label for="ssid">WiFi Network</label>
      <select name="ssid" id="ssid" onchange="document.getElementById('ssid_manual').value=this.value">
        <option value="">-- Select a network --</option>
)rawliteral");

  html += networkOptions;

  html += F(R"rawliteral(
      </select>
      <label for="ssid_manual">Or enter SSID manually</label>
      <input type="text" id="ssid_manual" name="ssid_manual" placeholder="Network name (SSID)" maxlength="31">
      <label for="password">Password</label>
      <input type="password" id="password" name="password" placeholder="Leave blank for open networks" maxlength="63">
      <button type="submit">💾 Save &amp; Connect</button>
    </form>
    <div class="notice">
      ⚠️ The device will restart and attempt to connect. If the connection fails, this setup portal will reappear.
    </div>
  </div>
  <script>
    // Sync manual SSID field back to select when user types
    document.getElementById('ssid_manual').addEventListener('input', function() {
      document.getElementById('ssid').value = '';
    });
  </script>
</body>
</html>
)rawliteral");

  server.send(200, "text/html", html);
}

void handleWiFiSave() {
  String newSSID = "";
  String newPassword = "";

  // Prefer manual entry if filled in, otherwise use dropdown selection
  if (server.hasArg("ssid_manual") && server.arg("ssid_manual").length() > 0) {
    newSSID = server.arg("ssid_manual");
  } else if (server.hasArg("ssid")) {
    newSSID = server.arg("ssid");
  }

  if (server.hasArg("password")) {
    newPassword = server.arg("password");
  }

  if (newSSID.length() == 0) {
    server.send(400, "text/html", "<h2>Error: No SSID provided.</h2><a href='/wifi'>Go back</a>");
    return;
  }

  // Save credentials to EEPROM
  strncpy(storedSSID, newSSID.c_str(), sizeof(storedSSID) - 1);
  storedSSID[sizeof(storedSSID) - 1] = '\0';
  strncpy(storedPassword, newPassword.c_str(), sizeof(storedPassword) - 1);
  storedPassword[sizeof(storedPassword) - 1] = '\0';
  saveConfigToEEPROM();

  Serial.print("WiFi credentials saved. SSID: ");
  Serial.println(storedSSID);
  Serial.println("Rebooting to connect...");

  String html = "<html><head><meta charset='UTF-8'>"
                "<style>body{font-family:sans-serif;display:flex;align-items:center;justify-content:center;"
                "min-height:100vh;background:linear-gradient(135deg,#667eea,#764ba2);margin:0;}"
                ".card{background:white;padding:30px;border-radius:12px;text-align:center;max-width:380px;}"
                "h2{color:#333;margin-bottom:10px;}p{color:#666;}</style></head>"
                "<body><div class='card'><h2>✅ Credentials Saved</h2>"
                "<p>Connecting to <strong>" + newSSID + "</strong>...</p>"
                "<p>The device is restarting. If the connection succeeds, find it on your network. "
                "If it fails, reconnect to <strong>ReflowOven-Setup</strong> to try again.</p></div></body></html>";
  server.send(200, "text/html", html);

  delay(REBOOT_DELAY_MS);
  rp2040.reboot();  // Reboot using arduino-pico reboot function
}

void handleNotFound() {
  Serial.print("HTTP: 404 - ");
  Serial.print(server.uri());
  Serial.print(" from ");
  Serial.println(server.client().remoteIP());

  // In captive portal mode, redirect everything to the portal page
  if (captivePortalActive) {
    Serial.println("  -> Redirecting to /wifi");
    server.sendHeader("Location", "/wifi", true);
    server.send(302, "text/plain", "");
    return;
  }
  server.send(404, "text/plain", "Not found");
}

void handleStatus() {
  JsonDocument doc;

  // Handle NaN temperature gracefully in JSON
  if (isnan(Input)) {
    doc["temp"] = nullptr;  // Send null instead of NaN
  } else {
    doc["temp"] = Input;
  }

  if (isnan(Setpoint)) {
    doc["setpoint"] = nullptr;
  } else {
    doc["setpoint"] = Setpoint;
  }

  doc["state"] = stateNames[currentState];

  // Calculate elapsed time based on current state
  if (currentState == PID_TUNING) {
    doc["elapsed"] = millis() - tuningStartTime;
  } else if (currentState != IDLE) {
    doc["elapsed"] = millis() - reflowStartTime;
  } else {
    doc["elapsed"] = 0;
  }

  doc["ssr"] = digitalRead(SSR_PIN);
  doc["output"] = Output;
  doc["tuning"] = pidTuningActive;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleGetConfig() {
  JsonDocument doc;
  doc["preheatTemp"] = preheatTemp;
  doc["preheatTime"] = preheatTime;
  doc["soakTemp"] = soakTemp;
  doc["soakTime"] = soakTime;
  doc["reflowTemp"] = reflowTemp;
  doc["reflowTime"] = reflowTime;
  doc["cooldownTemp"] = cooldownTemp;
  doc["cooldownTime"] = cooldownTime;
  doc["Kp"] = Kp;
  doc["Ki"] = Ki;
  doc["Kd"] = Kd;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSetConfig() {
  if (server.hasArg("plain")) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (!error) {
      preheatTemp = doc["preheatTemp"] | preheatTemp;
      preheatTime = doc["preheatTime"] | preheatTime;
      soakTemp = doc["soakTemp"] | soakTemp;
      soakTime = doc["soakTime"] | soakTime;
      reflowTemp = doc["reflowTemp"] | reflowTemp;
      reflowTime = doc["reflowTime"] | reflowTime;

      // Update PID parameters if provided
      if (doc["Kp"].is<double>()) {
        Kp = doc["Kp"];
        Ki = doc["Ki"];
        Kd = doc["Kd"];
        myPID.SetTunings(Kp, Ki, Kd);
        Serial.println("PID parameters updated:");
        Serial.print("Kp: "); Serial.print(Kp);
        Serial.print(", Ki: "); Serial.print(Ki);
        Serial.print(", Kd: "); Serial.println(Kd);
      }

      // Save to EEPROM
      saveConfigToEEPROM();

      Serial.println("Configuration updated:");
      Serial.print("Preheat: "); Serial.print(preheatTemp); Serial.print("°C for "); Serial.print(preheatTime/1000); Serial.println("s");
      Serial.print("Soak: "); Serial.print(soakTemp); Serial.print("°C for "); Serial.print(soakTime/1000); Serial.println("s");
      Serial.print("Reflow: "); Serial.print(reflowTemp); Serial.print("°C for "); Serial.print(reflowTime/1000); Serial.println("s");

      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
  }
}

void handleStart() {
  startReflow();
  server.send(200, "application/json", "{\"status\":\"started\"}");
}

void handleStop() {
  stopReflow();
  server.send(200, "application/json", "{\"status\":\"stopped\"}");
}

void handleDataLog() {
  String json = "[";
  for (int i = 0; i < dataLogIndex && i < maxDataPoints; i++) {
    if (i > 0) json += ",";
    json += "{\"t\":" + String(dataLog[i].time) + 
            ",\"temp\":" + String(dataLog[i].temperature) + 
            ",\"sp\":" + String(dataLog[i].setpoint) + "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleTunePID() {
  if (currentState == IDLE || currentState == COMPLETE || currentState == ERROR_STATE) {
    startPIDTuning();
    server.send(200, "application/json", "{\"status\":\"tuning_started\"}");
  } else if (currentState == PID_TUNING) {
    stopPIDTuning();
    server.send(200, "application/json", "{\"status\":\"tuning_stopped\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cannot start tuning while reflow active\"}");
  }
}

void handleGetConsoleLog() {
  String json = "[";
  for (int i = 0; i < consoleLogIndex && i < maxConsoleLines; i++) {
    if (i > 0) json += ",";
    json += "\"" + consoleLog[i] + "\"";
  }
  json += "]";
  server.send(200, "application/json", json);
}

