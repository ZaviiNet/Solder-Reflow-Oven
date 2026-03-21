/*
 * Configuration structures and constants
 * Solder Reflow Oven Controller
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Firmware Version
#define FIRMWARE_VERSION "2.2.0"

// OTA Configuration
#define OTA_HOSTNAME "reflow-oven"
// #define OTA_PASSWORD "reflowOTA"  // Uncomment to require a password for OTA

// WiFi Configuration
#define AP_SSID "ReflowOven-Setup"
#define DNS_PORT 53
#define REBOOT_DELAY_MS 1500

// LittleFS Configuration File Path
#define CONFIG_FILE "/config.json"

// Temperature Check Interval
#define TEMP_CHECK_INTERVAL 1000  // 1 second

// Safety Configuration
#define MAX_THERMOCOUPLE_ERRORS 3

// Data Logging Configuration
#define MAX_DATA_POINTS 500
#define MAX_CONSOLE_LINES 100

// Temperature-based state transition settings
#define TEMP_REACH_THRESHOLD 5.0  // Degrees within target to consider "reached"
#define MAX_STATE_TIMEOUT 180000   // 3 minutes max per stage (safety timeout)

// Reflow State Machine States
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

// Data Point Structure for Logging
struct DataPoint {
  unsigned long time;
  double temperature;
  double setpoint;
  ReflowState state;
};

// LED Status Patterns
enum LEDPattern {
  LED_OFF,           // Solid off - not initialized
  LED_SOLID,         // Solid on - connected and idle
  LED_SLOW_BLINK,    // 1 Hz - WiFi connecting or system starting
  LED_FAST_BLINK,    // 4 Hz - Active reflow
  LED_ERROR_BLINK    // 2 Hz - Error state
};

// Global state names array
extern String stateNames[];

#endif // CONFIG_H

