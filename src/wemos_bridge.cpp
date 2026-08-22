#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <time.h>

ESP8266WebServer server(80);
String logBuffer = "--- WEMOS D1 MINI WI-FI SYSTEM LOG ---\n";

void appendLog(String msg) {
  logBuffer += msg + "\n";
  if (logBuffer.length() > 2500) {
    logBuffer = logBuffer.substring(logBuffer.length() - 1800);
  }
}

String liveTemp = "-- C";
String liveDesc = "KERE";
String liveHum = "--%";

String f06Temp = "21 C", f06Desc = "FELHOS", f06Hum = "75%";
String f09Temp = "24 C", f09Desc = "NAPOS", f09Hum = "60%";
String f12Temp = "27 C", f12Desc = "MELEG", f12Hum = "52%";

void fetchOnlineWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    // Fetch Budapest JSON weather data from wttr.in
    if (http.begin(client, "http://wttr.in/Budapest?format=j1")) {
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Parse current temp
        int tIdx = payload.indexOf("\"temp_C\": \"");
        if (tIdx != -1) {
          int tEnd = payload.indexOf("\"", tIdx + 11);
          liveTemp = payload.substring(tIdx + 11, tEnd) + " C";
        }

        // Parse current humidity
        int hIdx = payload.indexOf("\"humidity\": \"");
        if (hIdx != -1) {
          int hEnd = payload.indexOf("\"", hIdx + 13);
          liveHum = payload.substring(hIdx + 13, hEnd) + "%";
        }

        // Parse weather desc
        int dIdx = payload.indexOf("\"value\": \"");
        if (dIdx != -1) {
          int dEnd = payload.indexOf("\"", dIdx + 10);
          String rawDesc = payload.substring(dIdx + 10, dEnd);
          rawDesc.toUpperCase();
          if (rawDesc.indexOf("SUN") != -1 || rawDesc.indexOf("CLEAR") != -1) liveDesc = "NAPOS";
          else if (rawDesc.indexOf("RAIN") != -1 || rawDesc.indexOf("SHOWER") != -1) liveDesc = "ZAPOR";
          else if (rawDesc.indexOf("CLOUD") != -1 || rawDesc.indexOf("OVERCAST") != -1) liveDesc = "FELHOS";
          else if (rawDesc.indexOf("SNOW") != -1) liveDesc = "HAVAS";
          else liveDesc = rawDesc.substring(0, 6);
        }

        appendLog("Weather Updated: " + liveTemp + ", " + liveDesc + ", " + liveHum);
      } else {
        appendLog("Weather HTTP Error: " + String(httpCode));
      }
      http.end();
    }
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

  server.on("/", []() {
    server.send(200, "text/plain", "WeMos D1 Mini Wi-Fi Bridge Ready! View log at /log");
  });
  server.on("/log", []() {
    server.send(200, "text/plain", logBuffer);
  });
  server.begin();

  appendLog("NTP & Web Server Log Engine Ready!");
  fetchOnlineWeather();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // Heartbeat LED
  digitalWrite(LED_BUILTIN, (millis() % 1000 < 100) ? LOW : HIGH);

  static uint32_t last_weather = 0;
  // Fetch weather every 30 minutes
  if (millis() - last_weather >= 1800000UL) {
    last_weather = millis();
    fetchOnlineWeather();
  }

  static uint32_t last_send = 0;
  if (millis() - last_send >= 2000) {
    last_send = millis();

    if (timeinfo->tm_year > 70) {
      char syncBuf[60];
      snprintf(syncBuf, sizeof(syncBuf), "SYNC:%02d:%02d:%02d,%04d.%02d.%02d\n",
               timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
               timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
      
      Serial.print(syncBuf);

      // Send Dynamic Weather Data to Mega ADK
      char weatherBuf[80];
      snprintf(weatherBuf, sizeof(weatherBuf), "WEATHER:%s,%s,%s\n",
               liveTemp.c_str(), liveDesc.c_str(), liveHum.c_str());
      Serial.print(weatherBuf);

      appendLog("Sent SYNC + WEATHER: " + String(liveTemp) + " " + String(liveDesc));
    }
  }
}
