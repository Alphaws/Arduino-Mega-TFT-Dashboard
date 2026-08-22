#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <time.h>

void setup() {
  // Software Serial / Hardware Serial to Arduino Mega ADK
  Serial.begin(115200);
  delay(500);

  // WiFiManager captive portal
  WiFiManager wifiManager;
  wifiManager.autoConnect("MegaTFT-Clock-AP");

  // Config NTP Time for Hungary (GMT+2)
  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");
}

void loop() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // If Arduino Mega requests sync via UART RX
  if (Serial.available()) {
    String req = Serial.readStringUntil('\n');
    if (req.indexOf("REQUEST_SYNC_NOW") != -1 && timeinfo->tm_year > 70) {
      char syncBuf[50];
      snprintf(syncBuf, sizeof(syncBuf), "SYNC:%02d:%02d:%02d,%04d.%02d.%02d\n",
               timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
               timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
      Serial.print(syncBuf);
    }
  }

  // Periodic Auto-Sync Every 10 Seconds
  if (timeinfo->tm_year > 70) {
    static uint32_t last_send = 0;
    if (millis() - last_send >= 10000) {
      last_send = millis();
      char syncBuf[50];
      snprintf(syncBuf, sizeof(syncBuf), "SYNC:%02d:%02d:%02d,%04d.%02d.%02d\n",
               timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
               timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
      Serial.print(syncBuf);
    }
  }
  delay(100);
}
