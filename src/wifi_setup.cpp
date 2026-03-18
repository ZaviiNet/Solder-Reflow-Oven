/*
 * WiFi Setup Implementation
 */

#include "wifi_setup.h"
#include "config.h"
#include "storage.h"

static DNSServer dnsServer;
static bool captivePortalActive = false;

bool isCaptivePortalActive() {
  return captivePortalActive;
}

void processDNSRequests() {
  if (captivePortalActive) {
    dnsServer.processNextRequest();
  }
}

DNSServer* getDNSServer() {
  return &dnsServer;
}

void setupCaptivePortal() {
  Serial.println("Starting captive portal access point...");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  WiFi.mode(WIFI_AP);
  delay(100);

  IPAddress apIP(192, 168, 4, 1);
  IPAddress netMask(255, 255, 255, 0);

  if (!WiFi.softAPConfig(apIP, apIP, netMask)) {
    Serial.println("ERROR: Failed to configure AP IP!");
  }

  bool apStarted = WiFi.softAP(AP_SSID, nullptr, 1, 0, 4);
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

  delay(500);

  Serial.print("Captive Portal SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Captive Portal IP:   ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Captive Portal MAC:  ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("Connect to '" + String(AP_SSID) + "' and visit http://" + apIP.toString() + " to configure WiFi");

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  captivePortalActive = true;

  Serial.println("Captive portal is READY and LISTENING");
}

void setupWiFi() {
  char ssid[32];
  char password[64];
  getStoredWiFiCredentials(ssid, password, sizeof(ssid), sizeof(password));

  if (strlen(ssid) > 0) {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 60) {
      delay(500);
      Serial.print(".");
      attempts++;

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

