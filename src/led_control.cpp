/*
 * LED Status Control Implementation
 */

#include "led_control.h"

// Forward declarations
extern ReflowState currentState;

static unsigned long lastLEDBlink = 0;
static bool ledState = false;
static LEDPattern currentLEDPattern = LED_OFF;

void initLED() {
  pinMode(LED_BUILTIN, OUTPUT);
  setLED(false);
  currentLEDPattern = LED_SLOW_BLINK;
  Serial.println("Onboard LED initialized");
}

void setLED(bool state) {
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
}

void updateLED() {
  unsigned long currentTime = millis();
  unsigned long interval = 0;

  // Determine blink pattern based on system state
  switch (currentState) {
    case ERROR_STATE:
      currentLEDPattern = LED_ERROR_BLINK;
      interval = 500;  // 2 Hz
      break;

    case PID_TUNING:
    case PREHEAT:
    case SOAK:
    case REFLOW:
    case COOLDOWN:
      currentLEDPattern = LED_FAST_BLINK;
      interval = 250;  // 4 Hz
      break;

    case COMPLETE:
      currentLEDPattern = LED_SLOW_BLINK;
      interval = 1000;  // 1 Hz
      break;

    case IDLE:
    default:
      currentLEDPattern = LED_SOLID;
      interval = 0;  // Solid on
      break;
  }

  // Update LED based on pattern
  if (interval == 0) {
    setLED(true);  // Solid on
  } else {
    if (currentTime - lastLEDBlink >= interval) {
      lastLEDBlink = currentTime;
      ledState = !ledState;
      setLED(ledState);
    }
  }
}

