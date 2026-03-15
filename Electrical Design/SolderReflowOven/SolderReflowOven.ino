/*
 * Solder Reflow Oven Controller - ESP8266 Web Interface Version
 * 
 * Hardware:
 * - NodeMCU 1.0 ESP12E (ESP8266)
 * - Adafruit MAX31855 K-Type Thermocouple Amplifier
 * - Solid State Relay (SSR) for heater control
 * 
 * Features:
 * - Web-based interface (no display required)
 * - WiFi connectivity with AP and Station modes
 * - Real-time temperature monitoring via WebSocket
 * - PID-controlled reflow profile
 * - Emergency stop functionality
 * 
 * Pin Connections:
 * - D5 (GPIO14): MAX31855 CLK
 * - D4 (GPIO2):  MAX31855 CS
 * - D6 (GPIO12): MAX31855 DO (MISO)
 * - D8 (GPIO15): SSR Control
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <PID_v1.h>
#include <Adafruit_MAX31855.h>
#include <SPI.h>
#include <ArduinoJson.h>

// Pin Definitions
#define THERMO_CLK D5   // GPIO14
#define THERMO_CS D4    // GPIO2
#define THERMO_DO D6    // GPIO12
#define SSR_PIN D8      // GPIO15

// WiFi Configuration - Change these to your network or leave for AP mode
const char* ssid = "ReflowOven";           // AP mode SSID
const char* password = "reflow123";         // AP mode password (min 8 chars)
bool useAPMode = true;                      // Set to false to connect to existing WiFi

// Web Server and WebSocket
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

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
double Kp = 2, Ki = 5, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// State Machine
enum ReflowState {
  IDLE,
  PREHEAT,
  SOAK,
  REFLOW,
  COOLDOWN,
  COMPLETE,
  ERROR_STATE
};

ReflowState currentState = IDLE;
String stateNames[] = {"IDLE", "PREHEAT", "SOAK", "REFLOW", "COOLDOWN", "COMPLETE", "ERROR"};

// Timing
unsigned long reflowStartTime = 0;
unsigned long stateStartTime = 0;
unsigned long lastTempCheck = 0;
unsigned long tempCheckInterval = 1000;  // Check temperature every 1 second

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

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n=================================");
  Serial.println("Solder Reflow Oven Controller");
  Serial.println("ESP8266 + MAX31855 + Web Interface");
  Serial.println("=================================\n");

  // Initialize SSR pin
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);
  Serial.println("SSR pin initialized (OFF)");

  // Initialize MAX31855
  Serial.println("Initializing MAX31855 thermocouple...");
  delay(500);
  double testTemp = maxthermo.readCelsius();
  if (isnan(testTemp)) {
    Serial.println("ERROR: Could not read from thermocouple!");
    Serial.println("Check wiring and connections.");
    currentState = ERROR_STATE;
  } else {
    Serial.print("MAX31855 OK - Current temp: ");
    Serial.print(testTemp);
    Serial.println(" °C");
  }

  // Initialize PID
  myPID.SetOutputLimits(0, 1);
  myPID.SetMode(AUTOMATIC);
  Serial.println("PID controller initialized");

  // Setup WiFi
  setupWiFi();

  // Setup web server routes
  setupWebServer();

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started on port 81");

  Serial.println("\n=================================");
  Serial.println("Setup complete!");
  Serial.println("=================================\n");
}

void setupWiFi() {
  WiFi.mode(WIFI_AP_STA);
  
  if (useAPMode) {
    Serial.println("Starting WiFi Access Point...");
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP SSID: ");
    Serial.println(ssid);
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.println("Connect to this network and navigate to http://" + IP.toString());
  } else {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nWiFi connection failed! Starting AP mode...");
      WiFi.softAP(ssid, password);
      Serial.print("AP IP: ");
      Serial.println(WiFi.softAPIP());
    }
  }
}

void setupWebServer() {
  // Serve main page
  server.on("/", HTTP_GET, handleRoot);
  
  // API endpoints
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handleSetConfig);
  server.on("/api/start", HTTP_POST, handleStart);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/data", HTTP_GET, handleDataLog);
  
  server.begin();
  Serial.println("HTTP server started on port 80");
}

void loop() {
  server.handleClient();
  webSocket.loop();
  
  // Read temperature periodically
  unsigned long currentTime = millis();
  if (currentTime - lastTempCheck >= tempCheckInterval) {
    lastTempCheck = currentTime;
    readTemperature();
    
    // Update state machine if reflow is active
    if (currentState != IDLE && currentState != COMPLETE && currentState != ERROR_STATE) {
      updateReflowStateMachine();
    }
    
    // Send status update via WebSocket
    sendWebSocketUpdate();
    
    // Log data point
    logDataPoint();
  }
}

void readTemperature() {
  Input = maxthermo.readCelsius();
  
  if (isnan(Input)) {
    thermocoupleErrorCount++;
    Serial.println("WARNING: Thermocouple read error!");
    
    if (thermocoupleErrorCount >= maxThermocoupleErrors) {
      Serial.println("CRITICAL: Multiple thermocouple errors - EMERGENCY STOP!");
      emergencyStopReflow();
      currentState = ERROR_STATE;
    }
  } else {
    thermocoupleErrorCount = 0;  // Reset error counter on successful read
  }
}

void updateReflowStateMachine() {
  if (emergencyStop) {
    return;
  }
  
  unsigned long elapsedTime = millis() - reflowStartTime;
  unsigned long stateElapsed = millis() - stateStartTime;
  
  switch (currentState) {
    case PREHEAT:
      Setpoint = preheatTemp;
      if (stateElapsed >= preheatTime) {
        changeState(SOAK);
      }
      break;
      
    case SOAK:
      Setpoint = soakTemp;
      if (stateElapsed >= soakTime) {
        changeState(REFLOW);
      }
      break;
      
    case REFLOW:
      Setpoint = reflowTemp;
      if (stateElapsed >= reflowTime) {
        changeState(COOLDOWN);
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
}

void startReflow() {
  if (currentState == IDLE || currentState == COMPLETE || currentState == ERROR_STATE) {
    Serial.println("Starting reflow process...");
    emergencyStop = false;
    thermocoupleErrorCount = 0;
    reflowStartTime = millis();
    dataLogIndex = 0;  // Reset data log
    changeState(PREHEAT);
  }
}

void stopReflow() {
  Serial.println("Stopping reflow process...");
  emergencyStopReflow();
  currentState = IDLE;
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
  String html = F(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Reflow Oven Controller</title>
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
  </div>

  <script>
    let ws;
    
    function connectWebSocket() {
      ws = new WebSocket('ws://' + window.location.hostname + ':81');
      
      ws.onopen = function() {
        console.log('WebSocket connected');
      };
      
      ws.onmessage = function(event) {
        const data = JSON.parse(event.data);
        updateDisplay(data);
      };
      
      ws.onclose = function() {
        console.log('WebSocket disconnected, reconnecting...');
        setTimeout(connectWebSocket, 1000);
      };
    }
    
    function updateDisplay(data) {
      document.getElementById('currentTemp').textContent = data.temp.toFixed(1) + '°C';
      document.getElementById('targetTemp').textContent = data.setpoint.toFixed(1) + '°C';
      
      const stateEl = document.getElementById('state');
      stateEl.textContent = data.state;
      stateEl.className = 'status-value state-' + data.state;
      
      document.getElementById('timeElapsed').textContent = Math.floor(data.elapsed / 1000) + 's';
      
      const ssrOn = data.ssr === 1;
      document.getElementById('ssrStatus').textContent = ssrOn ? 'ON' : 'OFF';
      document.getElementById('ssrIndicator').className = 'ssr-indicator ' + (ssrOn ? 'ssr-on' : 'ssr-off');
      
      // Update button states
      const isActive = data.state !== 'IDLE' && data.state !== 'COMPLETE' && data.state !== 'ERROR';
      document.getElementById('startBtn').disabled = isActive;
      document.getElementById('stopBtn').disabled = !isActive;
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
        })
        .catch(err => console.error('Error loading config:', err));
    }
    
    // Initialize
    connectWebSocket();
    loadConfig();
  </script>
</body>
</html>
)rawliteral");
  
  server.send(200, "text/html", html);
}

void handleStatus() {
  StaticJsonDocument<256> doc;
  doc["temp"] = Input;
  doc["setpoint"] = Setpoint;
  doc["state"] = stateNames[currentState];
  doc["elapsed"] = (currentState != IDLE) ? (millis() - reflowStartTime) : 0;
  doc["ssr"] = digitalRead(SSR_PIN);
  doc["output"] = Output;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleGetConfig() {
  StaticJsonDocument<256> doc;
  doc["preheatTemp"] = preheatTemp;
  doc["preheatTime"] = preheatTime;
  doc["soakTemp"] = soakTemp;
  doc["soakTime"] = soakTime;
  doc["reflowTemp"] = reflowTemp;
  doc["reflowTime"] = reflowTime;
  doc["cooldownTemp"] = cooldownTemp;
  doc["cooldownTime"] = cooldownTime;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSetConfig() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      preheatTemp = doc["preheatTemp"] | preheatTemp;
      preheatTime = doc["preheatTime"] | preheatTime;
      soakTemp = doc["soakTemp"] | soakTemp;
      soakTime = doc["soakTime"] | soakTime;
      reflowTemp = doc["reflowTemp"] | reflowTemp;
      reflowTime = doc["reflowTime"] | reflowTime;
      
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

void sendWebSocketUpdate() {
  StaticJsonDocument<256> doc;
  doc["temp"] = Input;
  doc["setpoint"] = Setpoint;
  doc["state"] = stateNames[currentState];
  doc["elapsed"] = (currentState != IDLE) ? (millis() - reflowStartTime) : 0;
  doc["ssr"] = digitalRead(SSR_PIN);
  doc["output"] = Output;
  
  String response;
  serializeJson(doc, response);
  webSocket.broadcastTXT(response);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
  }
}
