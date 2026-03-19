/*
 * Web Server Implementation
 * Handles all HTTP routes and responses
 */

#include "web_server.h"
#include "config.h"
#include "state_machine.h"
#include "pid_controller.h"
#include "temperature.h"
#include "wifi_setup.h"
#include "storage.h"
#include "presets.h"
#include "pins.h"
#include "ota.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Updater.h>
#include <pico/bootrom.h>

// Web server instance
static WebServer server(80);

// Forward declarations for external variables
extern double preheatTemp, soakTemp, reflowTemp, cooldownTemp;
extern unsigned long preheatTime, soakTime, reflowTime, cooldownTime;

WebServer* getWebServer() {
  return &server;
}

// Forward declarations for handler functions
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
void handleGetPresets();
void handleLoadPreset();
void handleSavePreset();
void handleDeletePreset();

void setupWebServer() {
  // Main page
  server.on("/", HTTP_GET, handleRoot);

  // Favicon handler
  server.on("/favicon.ico", HTTP_GET, []() {
    server.send(204);
  });

  // Captive portal WiFi configuration
  server.on("/wifi", HTTP_GET, handleCaptivePortal);
  server.on("/wifi/save", HTTP_POST, handleWiFiSave);

  // Captive portal detection URLs
  server.on("/generate_204", HTTP_GET, handleCaptivePortal);
  server.on("/fwlink", HTTP_GET, handleCaptivePortal);
  server.on("/hotspot-detect.html", HTTP_GET, handleCaptivePortal);
  server.on("/connecttest.txt", HTTP_GET, handleCaptivePortal);
  server.on("/ncsi.txt", HTTP_GET, handleCaptivePortal);

  // API endpoints
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handleSetConfig);
  server.on("/api/start", HTTP_POST, handleStart);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/data", HTTP_GET, handleDataLog);
  server.on("/api/tune-pid", HTTP_POST, handleTunePID);
  server.on("/api/console", HTTP_GET, handleGetConsoleLog);

  // Preset endpoints
  server.on("/api/presets", HTTP_GET, handleGetPresets);
  server.on("/api/presets/load", HTTP_POST, handleLoadPreset);
  server.on("/api/presets/save", HTTP_POST, handleSavePreset);
  server.on("/api/presets/delete", HTTP_POST, handleDeletePreset);

  // OTA firmware update endpoint
  server.on(
    "/api/ota/update", HTTP_POST,
    []() {
      server.sendHeader("Connection", "close");
      if (Update.hasError()) {
        server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Firmware update failed\"}");
        Serial.println("HTTP OTA: Update failed");
      } else {
        server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Update successful, rebooting...\"}");
        Serial.println("HTTP OTA: Update successful, rebooting...");
        delay(REBOOT_DELAY_MS);
        rp2040.reboot();
      }
    },
    []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.print("HTTP OTA: Receiving ");
        Serial.print(upload.filename);
        Serial.print(" (");
        Serial.print(upload.contentLength);
        Serial.println(" bytes)");
        if (!Update.begin(upload.contentLength > 0 ? (size_t)upload.contentLength : UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.print("HTTP OTA: Received ");
          Serial.print(upload.totalSize);
          Serial.println(" bytes, finalizing...");
        } else {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.abort();
        Serial.println("HTTP OTA: Upload aborted");
      }
    }
  );

  // Catch-all
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started on port 80");
}

void processWebServerRequests() {
  server.handleClient();
}

// Include the main HTML page inline
void handleRoot() {
  if (isCaptivePortalActive()) {
    Serial.println("HTTP: / requested - redirecting to /wifi (captive portal)");
    server.sendHeader("Location", "/wifi", true);
    server.send(302, "text/plain", "");
    return;
  }

  Serial.println("HTTP: / requested - serving main page");

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");

  // NOTE: The HTML is quite large. In a real refactoring, this could be moved to a separate file
  // or served from LittleFS. For now, keeping it inline for completeness.
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

      <div class="config-grid" style="margin-bottom: 20px;">
        <div class="config-item">
          <label>📋 Profile Preset</label>
          <select id="presetSelect" onchange="loadSelectedPreset()" style="width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 4px; font-size: 1em;">
            <option value="">-- Manual Configuration --</option>
          </select>
          <div class="help-text">Load a preset or configure manually</div>
        </div>
        <div class="config-item">
          <label>💾 Save Current as Preset</label>
          <div style="display: flex; gap: 5px;">
            <input type="text" id="newPresetName" placeholder="Enter preset name" style="flex: 1; padding: 10px; border: 1px solid #ddd; border-radius: 4px;">
            <button onclick="saveCurrentAsPreset()" style="padding: 10px 20px; background: #28a745; color: white; border: none; border-radius: 4px; cursor: pointer; white-space: nowrap;">Save</button>
          </div>
          <div class="help-text">Save your current settings as a new preset</div>
        </div>
      </div>

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

    <div class="card">
      <h2>🔄 Firmware Update (OTA)</h2>
      <div class="status-grid">
        <div class="status-item">
          <div class="status-label">Current Version</div>
          <div class="status-value" id="fwVersion">-</div>
        </div>
        <div class="status-item">
          <div class="status-label">OTA Hostname</div>
          <div class="status-value" style="font-size:1em;" id="otaHostname">-</div>
        </div>
      </div>

      <h3 style="margin:15px 0 8px; color:#555; font-size:1em;">IDE / PlatformIO Network Upload</h3>
      <p class="help-text">Upload firmware wirelessly using Arduino IDE 2.x or PlatformIO — no USB cable needed.</p>
      <div style="background:#f8f9fa; border-radius:6px; padding:12px; margin-top:8px; font-family:monospace; font-size:0.85em; word-break:break-all;">
        <div style="color:#888; margin-bottom:4px;">PlatformIO:</div>
        <div style="color:#333;" id="pioCommand">pio run -t upload --upload-port reflow-oven.local</div>
      </div>

      <h3 style="margin:18px 0 8px; color:#555; font-size:1em;">Browser Upload (.bin)</h3>
      <p class="help-text">Upload a compiled firmware <code>.bin</code> file directly from your browser.</p>
      <div style="margin-top:12px;">
        <input type="file" id="otaFile" accept=".bin" style="display:none;" onchange="handleOTAFile(event)">
        <label for="otaFile" class="btn-save" style="display:inline-block; cursor:pointer; padding:10px 20px;">
          📁 Choose .bin File
        </label>
        <div id="otaFileName" style="color:#666; font-size:0.9em; margin-top:8px;"></div>
        <div id="otaProgress" style="display:none; margin-top:10px;">
          <div style="background:#e9ecef; border-radius:4px; height:18px; overflow:hidden; margin-bottom:6px;">
            <div id="otaProgressBar" style="background:#667eea; height:100%; width:0%; transition:width 0.3s;"></div>
          </div>
          <div id="otaProgressText" style="text-align:center; font-size:0.85em; color:#666;"></div>
        </div>
        <button id="otaUploadBtn" class="btn-start" style="display:none; margin-top:10px;" onclick="startOTAUpload()">
          🚀 Upload Firmware
        </button>
        <div id="otaStatus" style="margin-top:10px; padding:10px; border-radius:4px; display:none;"></div>
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

      chartData.labels.push(timeInSeconds);
      chartData.datasets[0].data.push(temp);
      chartData.datasets[1].data.push(setpoint);

      if (chartData.labels.length > 300) {
        chartData.labels.shift();
        chartData.datasets[0].data.shift();
        chartData.datasets[1].data.shift();
      }

      chart.update('none');
    }

    function clearChart() {
      chartData.labels = [];
      chartData.datasets[0].data = [];
      chartData.datasets[1].data = [];
      chart.update();
    }

    function connectWebSocket() {
      fetchStatus();
      setInterval(fetchStatus, 500);
    }

    function fetchStatus() {
      fetch('/api/status')
        .then(r => {
          if (!r.ok) throw new Error('HTTP ' + r.status);
          return r.json();
        })
        .then(data => {
          updateDisplay(data);
        })
        .catch(err => {
          console.error('Error fetching status:', err);
          document.getElementById('currentTemp').textContent = 'OFFLINE';
          document.getElementById('targetTemp').textContent = '--';
        });
    }

    function updateDisplay(data) {
      const tempText = (data.temp !== null && !isNaN(data.temp)) ? data.temp.toFixed(1) + '°C' : 'ERROR';
      const setpointText = (data.setpoint !== null && !isNaN(data.setpoint)) ? data.setpoint.toFixed(1) + '°C' : '--';

      const tempEl = document.getElementById('currentTemp');
      tempEl.textContent = tempText;

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

      if (isActive) {
        updateChart(data.temp, data.setpoint, data.elapsed);
      }

      if (data.state === 'IDLE' && chartData.labels.length > 0) {
        clearChart();
      }

      // Update OTA info from status
      if (data.firmware) {
        document.getElementById('fwVersion').textContent = 'v' + data.firmware;
      }
      if (data.otaHostname) {
        document.getElementById('otaHostname').textContent = data.otaHostname + '.local';
        document.getElementById('pioCommand').textContent =
          'pio run -t upload --upload-port ' + data.otaHostname + '.local';
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
            const consoleDiv = document.getElementById('console');
            consoleDiv.scrollTop = consoleDiv.scrollHeight;

            const currentState = document.getElementById('state').textContent;
            if (currentState !== 'PID_TUNING') {
              stopConsolePolling();
              document.getElementById('tuneBtn').textContent = '🔧 Auto-Tune PID';
            }
          })
          .catch(err => console.error('Error fetching console:', err));
      }, 2000);
    }

    function stopConsolePolling() {
      if (consolePolling) {
        clearInterval(consolePolling);
        consolePolling = null;
      }
    }

    // Preset management functions
    function loadPresets() {
      fetch('/api/presets')
        .then(r => r.json())
        .then(presets => {
          const select = document.getElementById('presetSelect');
          // Keep first option (Manual Configuration)
          while (select.options.length > 1) {
            select.remove(1);
          }

          presets.forEach(preset => {
            const option = document.createElement('option');
            option.value = preset.name;
            option.textContent = preset.name + ' - ' + preset.description;
            select.appendChild(option);
          });
        })
        .catch(err => console.error('Error loading presets:', err));
    }

    function loadSelectedPreset() {
      const select = document.getElementById('presetSelect');
      const presetName = select.value;

      if (!presetName) {
        // Manual configuration selected, do nothing
        return;
      }

      if (confirm('Load preset "' + presetName + '"? This will overwrite your current configuration.')) {
        fetch('/api/presets/load', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ name: presetName })
        })
        .then(r => r.json())
        .then(data => {
          if (data.status === 'ok') {
            // Reload the config to show the preset values
            return fetch('/api/config');
          } else {
            throw new Error(data.message || 'Failed to load preset');
          }
        })
        .then(r => r.json())
        .then(data => {
          // Update all form fields with the loaded preset values
          document.getElementById('preheatTemp').value = data.preheatTemp;
          document.getElementById('preheatTime').value = data.preheatTime / 1000;
          document.getElementById('soakTemp').value = data.soakTemp;
          document.getElementById('soakTime').value = data.soakTime / 1000;
          document.getElementById('reflowTemp').value = data.reflowTemp;
          document.getElementById('reflowTime').value = data.reflowTime / 1000;

          console.log('Preset loaded:', data);
          alert('Preset loaded successfully!\n\n' +
                'Preheat: ' + data.preheatTemp + '°C for ' + (data.preheatTime/1000) + 's\n' +
                'Soak: ' + data.soakTemp + '°C for ' + (data.soakTime/1000) + 's\n' +
                'Reflow: ' + data.reflowTemp + '°C for ' + (data.reflowTime/1000) + 's');
        })
        .catch(err => {
          alert('Error loading preset: ' + err.message);
          console.error('Error:', err);
          select.value = '';  // Reset dropdown on error
        });
      } else {
        // User cancelled, reset dropdown
        select.value = '';
      }
    }

    function saveCurrentAsPreset() {
      const name = document.getElementById('newPresetName').value.trim();

      if (!name) {
        alert('Please enter a preset name');
        return;
      }

      const description = prompt('Enter a description for this preset (optional):', 'Custom preset');
      if (description === null) return;  // User cancelled

      fetch('/api/presets/save', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          name: name,
          description: description || 'Custom preset'
        })
      })
      .then(r => r.json())
      .then(data => {
        if (data.status === 'ok') {
          alert('Preset saved successfully!');
          document.getElementById('newPresetName').value = '';
          loadPresets();  // Reload preset list
        } else {
          alert('Error saving preset: ' + data.message);
        }
      })
      .catch(err => {
        alert('Error saving preset');
        console.error('Error:', err);
      });
    }

    initChart();
    connectWebSocket();
    loadConfig();
    loadPresets();

    // ---- OTA functions ----
    let otaFileData = null;

    function handleOTAFile(event) {
      const file = event.target.files[0];
      if (!file) return;
      otaFileData = file;
      document.getElementById('otaFileName').textContent =
        'Selected: ' + file.name + ' (' + Math.round(file.size / 1024) + ' KB)';
      document.getElementById('otaUploadBtn').style.display = 'inline-block';
    }

    function startOTAUpload() {
      if (!otaFileData) {
        alert('Please select a firmware .bin file first.');
        return;
      }
      if (!confirm('Are you sure you want to upload new firmware?\nThe device will restart after the update.')) return;

      const progressEl   = document.getElementById('otaProgress');
      const progressBar  = document.getElementById('otaProgressBar');
      const progressText = document.getElementById('otaProgressText');
      const statusEl     = document.getElementById('otaStatus');
      const uploadBtn    = document.getElementById('otaUploadBtn');

      progressEl.style.display = 'block';
      progressBar.style.width  = '0%';
      progressText.textContent = 'Uploading...';
      statusEl.style.display   = 'none';
      uploadBtn.disabled = true;

      const formData = new FormData();
      formData.append('firmware', otaFileData, otaFileData.name);

      const xhr = new XMLHttpRequest();
      xhr.upload.addEventListener('progress', (e) => {
        if (e.lengthComputable) {
          const pct = Math.round((e.loaded / e.total) * 100);
          progressBar.style.width  = pct + '%';
          progressText.textContent = pct + '% (' + Math.round(e.loaded / 1024) + ' / ' + Math.round(e.total / 1024) + ' KB)';
        }
      });
      xhr.addEventListener('load', () => {
        if (xhr.status === 200) {
          progressBar.style.width  = '100%';
          progressText.textContent = 'Upload complete!';
          statusEl.style.display   = 'block';
          statusEl.style.background = '#d4edda';
          statusEl.style.color      = '#155724';
          statusEl.textContent = '✅ Firmware update successful! The device is rebooting...';
        } else {
          progressEl.style.display = 'none';
          statusEl.style.display   = 'block';
          statusEl.style.background = '#f8d7da';
          statusEl.style.color      = '#721c24';
          statusEl.textContent = '❌ Update failed: ' + (xhr.responseText || 'Unknown error');
          uploadBtn.disabled = false;
        }
      });
      xhr.addEventListener('error', () => {
        progressEl.style.display = 'none';
        statusEl.style.display   = 'block';
        statusEl.style.background = '#f8d7da';
        statusEl.style.color      = '#721c24';
        statusEl.textContent = '❌ Upload error. Check your connection and try again.';
        uploadBtn.disabled = false;
      });
      xhr.open('POST', '/api/ota/update');
      xhr.send(formData);
    }
  </script>
</body>
</html>
)rawliteral");

  server.send(200, "text/html", html);
}

void handleCaptivePortal() {
  Serial.println("HTTP: /wifi requested - serving captive portal page");

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

  setStoredWiFiCredentials(newSSID.c_str(), newPassword.c_str());
  saveConfigToEEPROM();

  Serial.print("WiFi credentials saved. SSID: ");
  Serial.println(newSSID);
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
  rp2040.reboot();
}

void handleNotFound() {
  Serial.print("HTTP: 404 - ");
  Serial.print(server.uri());
  Serial.print(" from ");
  Serial.println(server.client().remoteIP());

  if (isCaptivePortalActive()) {
    Serial.println("  -> Redirecting to /wifi");
    server.sendHeader("Location", "/wifi", true);
    server.send(302, "text/plain", "");
    return;
  }
  server.send(404, "text/plain", "Not found");
}

void handleStatus() {
  JsonDocument doc;

  double temp = getTemperatureInput();
  double setpoint = getPIDSetpoint();

  if (isnan(temp)) {
    doc["temp"] = nullptr;
  } else {
    doc["temp"] = temp;
  }

  if (isnan(setpoint)) {
    doc["setpoint"] = nullptr;
  } else {
    doc["setpoint"] = setpoint;
  }

  ReflowState state = getCurrentState();
  doc["state"] = getStateName(state);

  // Calculate elapsed time
  static unsigned long tuningStartTime = 0;
  static unsigned long reflowStartTime = 0;

  if (state == PID_TUNING && isPIDTuningActive()) {
    if (tuningStartTime == 0) tuningStartTime = millis();
    doc["elapsed"] = millis() - tuningStartTime;
  } else if (state != IDLE) {
    if (reflowStartTime == 0 || state == IDLE) reflowStartTime = millis();
    doc["elapsed"] = millis() - reflowStartTime;
  } else {
    doc["elapsed"] = 0;
    tuningStartTime = 0;
    reflowStartTime = 0;
  }

  doc["ssr"] = digitalRead(SSR_PIN);
  doc["output"] = getPIDOutput();
  doc["tuning"] = isPIDTuningActive();
  doc["firmware"] = FIRMWARE_VERSION;
  doc["otaHostname"] = OTA_HOSTNAME;

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
  doc["Kp"] = getPIDKp();
  doc["Ki"] = getPIDKi();
  doc["Kd"] = getPIDKd();

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

      if (doc["Kp"].is<double>()) {
        setPIDTunings(doc["Kp"], doc["Ki"], doc["Kd"]);
        Serial.println("PID parameters updated:");
        Serial.print("Kp: "); Serial.print(getPIDKp());
        Serial.print(", Ki: "); Serial.print(getPIDKi());
        Serial.print(", Kd: "); Serial.println(getPIDKd());
      }

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
  DataPoint* dataLog = getDataLog();
  int dataLogIndex = getDataLogIndex();

  for (int i = 0; i < dataLogIndex && i < MAX_DATA_POINTS; i++) {
    if (i > 0) json += ",";
    json += "{\"t\":" + String(dataLog[i].time) +
            ",\"temp\":" + String(dataLog[i].temperature) +
            ",\"sp\":" + String(dataLog[i].setpoint) + "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleTunePID() {
  ReflowState state = getCurrentState();

  if (state == IDLE || state == COMPLETE || state == ERROR_STATE) {
    startPIDTuning();
    server.send(200, "application/json", "{\"status\":\"tuning_started\"}");
  } else if (state == PID_TUNING) {
    stopPIDTuning();
    server.send(200, "application/json", "{\"status\":\"tuning_stopped\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cannot start tuning while reflow active\"}");
  }
}

void handleGetConsoleLog() {
  String json = "[";
  String* consoleLog = getConsoleLog();
  int consoleLogIndex = getConsoleLogIndex();

  for (int i = 0; i < consoleLogIndex && i < MAX_CONSOLE_LINES; i++) {
    if (i > 0) json += ",";
    // Escape quotes in console log messages
    String escaped = consoleLog[i];
    escaped.replace("\"", "\\\"");
    json += "\"" + escaped + "\"";
  }
  json += "]";
  server.send(200, "application/json", json);
}

// Preset handlers
void handleGetPresets() {
  String json = getPresetsJSON();
  server.send(200, "application/json", json);
}

void handleLoadPreset() {
  if (server.hasArg("plain")) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (!error && doc["name"].is<const char*>()) {
      const char* presetName = doc["name"];
      loadPresetIntoWorkingProfile(presetName);
      saveConfigToEEPROM();  // Save the loaded preset as current config
      server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Preset loaded\"}");
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
  }
}

void handleSavePreset() {
  if (server.hasArg("plain")) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (!error && doc["name"].is<const char*>()) {
      const char* name = doc["name"];
      const char* description = doc["description"] | "Custom preset";

      if (saveWorkingProfileAsPreset(name, description)) {
        server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Preset saved\"}");
      } else {
        server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to save preset\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
  }
}

void handleDeletePreset() {
  if (server.hasArg("plain")) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (!error && doc["name"].is<const char*>()) {
      const char* name = doc["name"];

      if (deletePreset(name)) {
        server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Preset deleted\"}");
      } else {
        server.send(404, "application/json", "{\"status\":\"error\",\"message\":\"Preset not found\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data\"}");
  }
}

