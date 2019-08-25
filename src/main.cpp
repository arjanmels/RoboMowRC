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
#include <ESPmDNS.h>
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
#include "websocket.h"
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
    log_i("Location, Free Heap: %d kByte, Free PSRAM: %d kByte, Minimum Stack Free: %d Bytes", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024, uxTaskGetStackHighWaterMark(NULL));
    vTaskSuspend(NULL);

    String apiKey = Portal.getSetting(CFG_GEOAPIKEY);
    if (!WiFi.isConnected())
    {
      log_w("Failed to get timezone: wifi not connected");
      continue;
    }
    else if (apiKey.length() == 0)
    {
      log_w("No API key for ipgeolocation.io");
      continue;
    }
    else
    {

      log_i("Trying to get location and timezone from ipgeolocation.io");

      String url = "https://api.ipgeolocation.io/timezone?apiKey=" + apiKey;
      if (gps.location.age() <= GPS_MAX_AGE_FOR_TIMEZONE)
        url += String() + "&lat=" + String(info.latitude, 5) + "&long=" + String(info.longitude, 5);

      HTTPClient http;
      http.begin(url);
      int httpCode = http.GET();
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

        log_i("Got location from ipgeolocation.io: %.5f, %.5f, Timezone: %s", info.latitude, info.longitude, info.timezone.c_str());

        http.end();
        flagUpdateLocation = true;
        continue; // skip error handling
      }
      else
      {
        log_w("Failed to get timezone from server (%d: %s).", httpCode, http.errorToString(httpCode).c_str());
      }
    }

    // Error handling when could not get from ipgeolocation
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
  log_i("PrevCount: %d", prevCount);
}

void updateTimeSettings()
{
  log_i("Updating time settings");

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

    log_i("Configuring time with offset: %d and DST offset: %d", info.gmtOffset, info.daylightOffset);
    configTime(info.gmtOffset, info.daylightOffset, "pool.ntp.org", "time.nist.gov", "time.windows.com");
  }

  static String prevApiKey;
  String apiKey = Portal.getSetting(CFG_GEOAPIKEY);
  if (apiKey != prevApiKey)
  {
    prevApiKey = apiKey;
    getLocationFromIP();
  }
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
  log_i("WiFi event: %d", event);
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
  log_e("Should not be reached...");
#else
  setup2();
#endif
}

void setup2()
{
  // Create RTOS items
  xTaskCreate(taskIpgeolocationHandler, "ipgeolocation", 6000, NULL, 1, &taskIpgeolocation);

  // Initialize Serial Speed
  Serial.begin(115200);

  // Show initializing message (this takes a while due to MD5sum calculation etc.)
  printStartupInfo();

  log_i("Start NVS... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
  nvs.init("settings");

  /*  Serial.println("Start Display...");
  log_i("Free heap: %i kByte", ESP.getFreeHeap() / 1024);
  displayInit();
  displayStart();
*/

  log_i("Reset WiFi... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
  resetWiFi();
  WiFi.onEvent(WiFiEvent);

  log_i("Start Webserver... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);

  Portal.on("/settings", settingsChanged);
  if (Portal.begin())
  {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    log_i("Start mDNS Server... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
    if (MDNS.begin("RoboMowRC"))
      MDNS.addService("http", "tcp", 80);

    log_i("Start MQTT Client... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
    mqttSetup();

    log_i("Start Websocket Server... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
    websocketSetup();
  }
  log_i("Start BLE... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
  setupBLE();

  log_i("Start LoRa... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
  setupLora();

  log_i("Start GPS... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
  Serial1.begin(9600, SERIAL_8N1, 12, 15);

  /*
  log_i("Start Display... (Free heap: %d kByte, Free PSRAM: %d kByte)", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
  displayStart();
*/

  log_i("Updating time settings...");
  updateTimeSettings();
  Serial.printf("\nStarting Main Loop... (Free heap: %d kByte, Free PSRAM: %d kByte)\n", ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
}

/**
 * @brief Arduino main loop
 *        
 */
void loop()
{
  Portal.handleClient();
  mqttHandle();
  GPSHandle();
  BLEHandle();
  websocketHandle();

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

  // TODO: wait with location sending untill initial time update
  sendLocationLora(1, info.latitude, info.longitude, info.altitude, gps.location.age());

  if (flagUpdateLocation)
  {
    updateTimeSettings();
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

  static unsigned long prevBeat = 0;
  if (millis() - prevBeat > 1000)
  {
    prevBeat = millis();
    log_i("Heartbeat: %7ld, Free Heap: %d kB, PSRAM: %d kB, Minimum Stack: %d B", prevBeat, ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024, uxTaskGetStackHighWaterMark(NULL));
  }
}
