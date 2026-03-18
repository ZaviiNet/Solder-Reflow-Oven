# Architecture Overview

## System Block Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         main_new.cpp                             │
│                    (Main Program - 120 lines)                    │
│                                                                   │
│  • setup() - Initialize all modules                              │
│  • loop()  - Process events and update state                    │
└─────────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
        ▼                     ▼                     ▼
┌──────────────────┐  ┌──────────────┐   ┌─────────────────┐
│  Hardware Layer  │  │ Control Layer│   │ Communication   │
├──────────────────┤  ├──────────────┤   ├─────────────────┤
│ • pins.h         │  │ • pid_ctrl   │   │ • wifi_setup    │
│ • temperature    │  │ • state_mach │   │ • web_server    │
│ • led_control    │  └──────────────┘   └─────────────────┘
└──────────────────┘          │                    │
        │                     │                    │
        └─────────────────────┼────────────────────┘
                              ▼
                    ┌──────────────────┐
                    │    Data Layer    │
                    ├──────────────────┤
                    │ • storage        │
                    │ • config.h       │
                    └──────────────────┘
```

## Module Interaction Flow

### Temperature Reading Flow
```
loop() → readTemperature() → MAX31855 → Input variable
                                    ↓
                            Error handling
                                    ↓
                         Emergency stop (if needed)
```

### PID Control Flow
```
State Machine → setPIDSetpoint() → PID Controller → computePID()
                                                          ↓
                                                   Output (0-1)
                                                          ↓
                                                   SSR Control
```

### Web Interface Flow
```
Browser → HTTP Request → web_server → API Handler
                              ↓              ↓
                        state_machine   pid_controller
                              ↓              ↓
                         temperature    storage
                              ↓              ↓
                        JSON Response ← ←  ← ←
```

### Configuration Storage Flow
```
Web Interface → handleSetConfig() → saveConfigToEEPROM()
                                              ↓
                                    LittleFS (Flash)
                                              ↓
                                    Persist: WiFi, PID, Profile
```

## Data Flow Diagram

```
                    ┌──────────────┐
                    │   Sensors    │
                    │  (MAX31855)  │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │ Temperature  │
                    │   Module     │
                    └──────┬───────┘
                           │
                ┏━━━━━━━━━━┻━━━━━━━━━━┓
                ▼                      ▼
        ┌──────────────┐      ┌───────────────┐
        │State Machine │      │ PID Controller│
        │              │◄────►│               │
        └──────┬───────┘      └───────┬───────┘
               │                      │
               └──────────┬───────────┘
                          │
                          ▼
                   ┌─────────────┐
                   │  SSR Output │
                   │   (Heater)  │
                   └─────────────┘
```

## Module Responsibility Matrix

| Module | Reads | Writes | Responsibilities |
|--------|-------|--------|------------------|
| **temperature** | MAX31855 | Input variable | Sensor management, error detection |
| **pid_controller** | Input, Setpoint | Output | Control algorithm, auto-tuning |
| **state_machine** | Input, Time | Setpoint, State | Process stages, transitions |
| **led_control** | State | LED pin | Visual feedback |
| **storage** | Flash | Flash | Configuration persistence |
| **wifi_setup** | Network | WiFi config | Connectivity management |
| **web_server** | All | Responses | User interface, API |
| **main_new** | - | - | Orchestration, initialization |

## Call Hierarchy

```
main_new.cpp
│
├─ setup()
│  ├─ initLED()
│  ├─ initTemperatureSensor()
│  │  └─ Adafruit_MAX31855.begin()
│  ├─ initPIDController()
│  │  └─ PID.SetMode(AUTOMATIC)
│  ├─ initStorage()
│  │  └─ LittleFS.begin()
│  ├─ loadConfigFromEEPROM()
│  │  ├─ LittleFS.open()
│  │  └─ deserializeJson()
│  ├─ setupWiFi()
│  │  ├─ WiFi.begin()
│  │  └─ setupCaptivePortal() [if needed]
│  ├─ setupWebServer()
│  │  └─ server.on() [multiple routes]
│  └─ initStateMachine()
│
└─ loop()
   ├─ processDNSRequests()
   ├─ processWebServerRequests()
   ├─ updateLED()
   ├─ readTemperature()
   │  └─ maxthermo.readCelsius()
   ├─ updatePIDTuningStateMachine() [if tuning]
   │  ├─ Oscillation detection
   │  └─ calculatePIDFromAutoTune()
   ├─ updateReflowStateMachine() [if active]
   │  ├─ Temperature checking
   │  ├─ State transitions
   │  ├─ computePID()
   │  └─ SSR control
   └─ logDataPoint()
```

## Memory Layout

```
┌─────────────────────────┐
│   Program Flash         │
│                         │
│  • Compiled Code        │
│  • HTML/CSS/JS (inline) │
│  • Constants            │
└─────────────────────────┘

┌─────────────────────────┐
│   LittleFS Flash        │
│                         │
│  • config.json          │
│    - WiFi credentials   │
│    - PID parameters     │
│    - Reflow profile     │
└─────────────────────────┘

┌─────────────────────────┐
│   RAM (SRAM)            │
│                         │
│  • Static variables     │
│  • Data log buffer      │
│  • Console log buffer   │
│  • Network buffers      │
└─────────────────────────┘
```

## Communication Protocols

### Internal (Module-to-Module)
- **Direct function calls**: Fast, compile-time checked
- **Shared variables**: Carefully managed with forward declarations
- **Return values**: Standard C++ types

### External (Device-to-World)
- **SPI**: MAX31855 thermocouple (hardware)
- **GPIO**: SSR control, LED (digital out)
- **WiFi**: HTTP/TCP/IP stack
- **HTTP**: RESTful API (JSON)
- **DNS**: Captive portal detection

## Timing Diagram

```
Time →
│
├─ 0ms: Power on
│  └─ setup() executes (~2000ms)
│
├─ Loop iteration (~1ms)
│  ├─ DNS processing
│  ├─ Web requests
│  ├─ LED update
│  └─ Check timer
│
├─ Every 1000ms (TEMP_CHECK_INTERVAL)
│  ├─ Read temperature
│  ├─ Update state machine
│  ├─ Compute PID
│  ├─ Control SSR
│  └─ Log data
│
└─ Continuous cycle...
```

## Error Handling Strategy

```
Error Detected
    ↓
Is it critical?
    ├─ YES → Emergency stop
    │         └─ Turn off SSR
    │             └─ Set ERROR_STATE
    │                 └─ Wait for user
    │
    └─ NO → Log warning
              └─ Retry operation
                  └─ Increment error counter
                      └─ Check threshold
                          └─ If exceeded → Emergency stop
```

## Testing Strategy

### Unit Testing (Possible with new architecture)
```
temperature_test.cpp
    └─ Test sensor initialization
    └─ Test error detection
    └─ Test retry logic

pid_controller_test.cpp
    └─ Test PID computation
    └─ Test auto-tune calculations
    └─ Test parameter updates

state_machine_test.cpp
    └─ Test state transitions
    └─ Test timing logic
    └─ Test emergency stop
```

### Integration Testing
```
Device Testing
    └─ Test full reflow cycle
    └─ Test web interface
    └─ Test WiFi setup
    └─ Test configuration persistence
```

---

This modular architecture makes the system easier to understand, test, and extend!

