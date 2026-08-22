#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <time.h>

// WebServer for Wireless Live Remote Logging (/log)
ESP8266WebServer server(80);
String logBuffer = "--- WEMOS D1 MINI WI-FI SYSTEM LOG ---\n";

void appendLog(String msg) {
  logBuffer += msg + "\n";
  if (logBuffer.length() > 2000) {
    logBuffer = logBuffer.substring(logBuffer.length() - 1500);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  appendLog("System Booting...");

  WiFiManager wifiManager;
  wifiManager.autoConnect("MegaTFT-Clock-AP");

  appendLog("Wi-Fi Connected! IP: " + WiFi.localIP().toString());

  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  ArduinoOTA.setHostname("wemos-tft-bridge");
  ArduinoOTA.begin();

  // Setup Web Server Log Endpoint http://<WEMOS_IP>/log
  server.on("/", []() {
    server.send(200, "text/plain", "WeMos D1 Mini Wi-Fi Bridge Ready! View log at /log");
  });
  server.on("/log", []() {
    server.send(200, "text/plain", logBuffer);
  });
  server.begin();

  appendLog("NTP & Web Server Log Engine Ready!");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // Heartbeat LED
  digitalWrite(LED_BUILTIN, (millis() % 1000 < 100) ? LOW : HIGH);

  static uint32_t last_log = 0;
  if (millis() - last_log >= 3000) {
    last_log = millis();

    if (timeinfo->tm_year > 70) {
      char syncBuf[60];
      snprintf(syncBuf, sizeof(syncBuf), "SYNC:%02d:%02d:%02d,%04d.%02d.%02d",
               timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
               timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
      
      // Output to Hardware Serial & Append to Wireless Web Log
      Serial.println(syncBuf);
      appendLog("Sent: " + String(syncBuf));
    } else {
      appendLog("Waiting for NTP Time Sync...");
    }
  }
}
