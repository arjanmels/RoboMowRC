/*
 * RoboMow RC - ESP32 Application to monitor and control RoboMow lawn mowers
 * 
 * Copyright (c) 2019 Arjan Mels
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Created : Thursday June 20th 2019 08:17:42 by Arjan Mels <github@mels.email>
 * Modified: Thursday June 20th 2019 08:17:42 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#include <Arduino.h>

#include <WiFi.h>

#include "robomowwebserver.h"

#include <PageBuilder.h>
//#include <ESPmDNS.h>
#include <time.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <esp_task_wdt.h>

//#include "config.h"

#include "images.h"
//#include "display.h"
#include "startupinfo.h"
#include "mqtt.h"
#include "lora.h"
#include "ble.h"
#include "config.h"
#include "general.h"
//#include "websocket.h"
/*
void mqttConnect(String const &server, int port, bool tls, String const &clientid, String const &user, String const &passwd) {}
void mqttSetup() {}
void mqttPublishStats() {}
void mqttPublishLocation() {}
*/

// Define Hardware components
TinyGPSPlus gps;
NVS nvs;

static bool flagUpdateLocation = false;

/**
 * @brief Clean up previous WiFi Connection
 * 
 */
void resetWiFi()
{
  // After a reboot without WiFi.disconnect() a WiFi error Reason202 - AUTH_FAIL occurs
  // with some routers: the connection was not broken of correctly
  // a begin/disconnect pair seems to properly cleanup. Delays are experimentally found
  // and may need to be different for different routers
  WiFi.disconnect(true);
  WiFi.begin();
  delay(500);
  WiFi.disconnect(true);
  delay(500);
}

Info info;

TaskHandle_t taskIpgeolocation;

void taskIpgeolocationHandler(void *parameter)
{
  while (true)
  {
    vTaskSuspend(NULL);

    String apiKey = Portal.getSetting(CFG_GEOAPIKEY);
    if (!WiFi.isConnected())
    {
      Serial.printf("Failed to get timezone: wifi not connected\n");
    }
    if (apiKey.length() == 0)
    {
      Serial.println("No API key for ipgeolocation.io");
    }
    else
    {
      Serial.println("Trying to get location and timezone from ipgeolocation.io");

      String url = "https://api.ipgeolocation.io/timezone?apiKey=" + apiKey;
      if (gps.location.age() <= GPS_MAX_AGE_FOR_TIMEZONE)
        url += String() + "&lat=" + String(info.latitude, 5) + "&long=" + String(info.longitude, 5);

      HTTPClient http;
      http.begin(url);
      int httpCode = http.GET();
      // httpCode will be negative on error
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
        DynamicJsonDocument doc(500);
        deserializeJson(doc, payload);
        info.gmtOffset = doc["timezone_offset"].as<int>() * 3600;
        info.daylightOffset = doc["dst_savings"].as<int>() * 3600;

        info.timezone = Portal.getSetting(CFG_TIMEZONE);
        if (info.timezone == "Auto")
          info.timezone = doc["timezone"].as<String>();

        if (gps.location.age() > GPS_MAX_AGE_FOR_TIMEZONE)
        {
          info.source = "ip";
          info.latitude = doc["geo"]["latitude"].as<float>();
          info.longitude = doc["geo"]["longitude"].as<float>();
          info.altitude = 0;
          info.satellites = 0;
        }

        Serial.printf("Got location from ipgeolocation.io: %.5f, %.5f, Timezone: %s\n", info.latitude, info.longitude, info.timezone.c_str());

        http.end();
        flagUpdateLocation = true;
        continue; // skip error handling
      }
      else
      {
        Serial.printf("Failed to get timezone from server (%d: %s).\n", httpCode, http.errorToString(httpCode).c_str());
      }
    }

    // could not get form ipgeolocation
    if (gps.location.age() > GPS_MAX_AGE_FOR_TIMEZONE)
    {
      info.source = "none";
      info.satellites = 0;
    }
    info.timezone = Portal.getSetting(CFG_TIMEZONE);
  }
}

void GPSHandle()
{
  while (Serial1.available())
    gps.encode(Serial1.read());

  static float prevLat;
  static float prevLng;
  static float prevAlt;
  static uint32_t prevSats;

  if (gps.location.isUpdated())
  {
    if (gps.distanceBetween(gps.location.lat(), gps.location.lng(), prevLat, prevLng) > 10.0 || fabs(gps.altitude.meters() - prevAlt) > 2.5 || gps.satellites.value() != prevSats)
    {
      prevLat = gps.location.lat();
      prevLng = gps.location.lng();
      prevAlt = gps.altitude.meters();
      prevSats = gps.satellites.value();
      log_d("%.5f, %.5f, %.2f, satellites: %d", prevLat, prevLng, prevAlt, prevSats);
      info.source = "gps";
      info.latitude = prevLat;
      info.longitude = prevLng;
      info.altitude = prevAlt;
      info.satellites = prevSats;
      mqttPublishLocation();
    }
  }
}

void getLocationFromIP()
{
  static unsigned long prevGet;
  static unsigned long prevCount = 3;
  if (millis() - prevGet > 5 * 60 * 1000)
  {
    prevCount += (millis() - prevGet) / (5 * 60 * 1000);
    if (prevCount > 3)
      prevCount = 3;
    prevGet = millis();
  }

  if (prevCount > 0)
  {
    prevCount--;
    vTaskResume(taskIpgeolocation);
  }
  Serial.println(String("PrevCount: ") + prevCount);
}

void updateSettings()
{
  Serial.println("Updating settings");

  static String prevTimeZone;
  static int prevGmtOffset;
  static int prevDaylightOffset;
  String timeZone = Portal.getSetting(CFG_TIMEZONE);
  if (prevTimeZone != timeZone || info.gmtOffset != prevGmtOffset || info.daylightOffset != prevDaylightOffset)
  {
    prevTimeZone = timeZone;
    if (timeZone != "Auto")
    {
      timeZone.remove(0, 4);
      timeZone.replace("+", "");

      info.gmtOffset = timeZone.toFloat() * 3600;
      info.daylightOffset = 0;
      info.timezone = timeZone;
    }

    Serial.println(String("Configuring time with offset: ") + info.gmtOffset + " and DST Offset: " + info.daylightOffset);
    configTime(info.gmtOffset, info.daylightOffset, "pool.ntp.org", "time.nist.gov", "time.windows.com");
  }

  static String prevApiKey;
  String apiKey = Portal.getSetting(CFG_GEOAPIKEY);
  if (apiKey != prevApiKey)
  {
    prevApiKey = apiKey;
    getLocationFromIP();
  }
  mqttReconnect();
}

static String settingsChanged(AutoConnectAux &aux, PageArgument &args)
{
  if (args.size() > 0)
  {
    Portal.saveParams(aux, args);
    // handle update in main loop: limits stack depth
    flagUpdateLocation = true;
  }
  return String();
}

void WiFiEvent(WiFiEvent_t event)
{
  Serial.println(String("WiFi event: ") + event);
  switch (event)
  {
  case SYSTEM_EVENT_AP_START:
    break;
  case SYSTEM_EVENT_STA_START:
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    break;
  case SYSTEM_EVENT_GOT_IP6:
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    break;
  default:
    break;
  }
}

#ifdef CUSTOM_STACK_SIZE
extern TaskHandle_t loopTaskHandle;
extern bool loopTaskWDTEnabled;

void setup2();
static void loopTask(void *pvParameters)
{
  setup2();
  for (;;)
  {
    if (loopTaskWDTEnabled)
    {
      esp_task_wdt_reset();
    }
    loop();
  }
}
#endif

/**
 * @brief Arduino setup routine, containing one time initialization
 * 
 */
void setup()
{
#ifdef CUSTOM_STACK_SIZE
  // Recreate loop task with bigger stack
  TaskHandle_t loopTaskHandle = NULL;
  xTaskCreateUniversal(loopTask, "loopTask", CUSTOM_STACK_SIZE, NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
  vTaskDelete(NULL);
  Serial.println("Should not be reached...");
#else
  setup2();
#endif
}

void setup2()
{
  // Create RTOS items
  xTaskCreate(taskIpgeolocationHandler, "ipgeolocation", 4096, NULL, 1, &taskIpgeolocation);

  // Initialize Serial Speed
  Serial.begin(115200);

  // Show initializing message (this takes a while due to MD5sum calculation etc.)
  printStartupInfo();

  Serial.println("Start NVS...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  nvs.init("settings");

  /*  Serial.println("Start Display...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  displayInit();
  displayStart();
*/

  Serial.println("Start MQTT...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  mqttSetup();

  Serial.println("Reset WIFI...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  resetWiFi();
  WiFi.onEvent(WiFiEvent);

  Serial.println("Start Websocket...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  //  websocketSetup();

  Serial.println("Start Webserver...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);

  Portal.on("/settings", settingsChanged);
  if (Portal.begin())
  {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    /*    Serial.println("Start MDNS...");
    log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
    if (MDNS.begin("RoboMowRC"))
      MDNS.addService("http", "tcp", 80);
      */
  }

  Serial.println("Start GPS receiver...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  //GPS Receiver settings
  Serial1.begin(9600, SERIAL_8N1, 12, 15);

  /*
  Serial.println("Start Display...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  displayStart();
*/

  Serial.println("Start BLE...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  setupBLE();

  Serial.println("Start LoRA...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  setupLora();

  Serial.println("Update Settings...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  updateSettings();

  Serial.println("Starting Main Loop...");
  Serial.println(String("Free heap: ") + ESP.getFreeHeap() / 1024 + " kByte");
}

/**
 * @brief Arduino main loop
 *        
 */
void loop()
{

  //ArduinoOTA.handle();
  mqttHandle();
  Portal.handleClient();
  GPSHandle();
  loopBLE();
  //  websocketHandle();

  static unsigned long prevSec = 0;
  if (millis() - prevSec > 1000)
  {
    prevSec = millis();
    //    displayStart();

    // get the timezoneooffset at the start of every hour to allow for DST changes
    static int prevHour = 0;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10) && timeinfo.tm_hour != prevHour)
    {
      prevHour = timeinfo.tm_hour;
      getLocationFromIP();
    }
  }

  sendLocationLora(1, info.latitude, info.longitude, info.altitude, gps.location.age());

  if (flagUpdateLocation)
  {
    updateSettings();
    mqttPublishLocation();
    flagUpdateLocation = false;
  }

  // update Mqtt state at least every minute
  static unsigned long prevMqtt = 0;
  if (millis() - prevMqtt > 10000)
  {
    prevMqtt = millis();
    mqttPublishStats();
  }
}
