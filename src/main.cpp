/*
 * FaMe IT ECG Filter for Cardiac Booster
 * 
 * Copyright (c) 2019 FaMe IT
 * 
 * Created : Saturday May 11th 2019 01:36:18 by Arjan Mels <admin@fame-it.net>
 * Modified: Saturday May 11th 2019 01:36:18 by Arjan Mels <admin@fame-it.net>
 * 
 * Revisions:
 * 28-05-2019	AM	Initial Version
 */

#include <Arduino.h>
#include <ArduinoOTA.h>

#include <WiFi.h>

#include "robomowwebserver.h"

#include <PageBuilder.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <time.h>
#include <HTTPClient.h>

//#include "config.h"

#include "buffer.h"
#include "images.h"
//#include "OTA.h"
#include "touchkey.h"
#include "display.h"
#include "startupinfo.h"
#include "mqtt.h"
/*
void mqttConnect(String const &server, int port, bool tls, String const &clientid, String const &user, String const &passwd) {}
void mqttSetup() {}
void mqttPublishStats() {}
void mqttPublishLocation() {}
*/

// Define Hardware components
TouchKey keyDown(12);
TouchKey keyUp(13);
bool flagUpdateSettings = false;

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
    String apiKey = Portal.getSetting("ipgeolocation_apikey");
    if (apiKey.length() > 0)
    {
      Serial.println("Trying to get location and timezone from ipgeolocation.io");
      HTTPClient http;
      http.begin("https://api.ipgeolocation.io/timezone?apiKey=" + apiKey);
      int httpCode = http.GET();
      // httpCode will be negative on error
      if (httpCode > 0)
      {
        if (httpCode == HTTP_CODE_OK)
        {
          String payload = http.getString();
          DynamicJsonDocument doc(1000);
          deserializeJson(doc, payload);
          info.gmtOffset = doc["timezone_offset"].as<int>() * 3600;
          info.daylightOffset = doc["dst_savings"].as<int>() * 3600;

          info.city = doc["geo"]["city"].as<String>();
          info.country = doc["geo"]["country_name"].as<String>();
          info.latitude = doc["geo"]["latitude"].as<String>();
          info.longitude = doc["geo"]["longitude"].as<String>();
          info.timezone = doc["timezone"].as<String>();

          Serial.printf("Location: %s, %s, Lat: %s, Lon: %s, Timezone: %s\n", info.city.c_str(), info.country.c_str(), info.latitude.c_str(), info.longitude.c_str(), info.timezone.c_str());
        }
        else
        {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("Failed to get timezone from server (%d).\n", httpCode);
        }
      }
      else
      {
        Serial.printf("Failed to get timezone from server (%s).\n", http.errorToString(httpCode).c_str());
      }

      http.end();
      flagUpdateSettings = true;
    }
    else
    {
      Serial.println("No API key for ipgeolocation.io");
    }
  }
}

void getLocationFromIP()
{
  String timeZone = Portal.getSetting("timezone");
  if (timeZone == "Auto")
    vTaskResume(taskIpgeolocation);
}

void updateSettings()
{
  Serial.println("Updating settings");

  static String prevTimeZone;
  static int prevGmtOffset;
  static int prevDaylightOffset;
  String timeZone = Portal.getSetting("timezone");
  if (prevTimeZone != timeZone || info.gmtOffset != prevGmtOffset || info.daylightOffset != prevDaylightOffset)
  {
    prevTimeZone = timeZone;
    if (timeZone != "Auto")
    {
      timeZone.remove(0, 4);
      timeZone.replace("+", "");

      info.gmtOffset = timeZone.toFloat() * 3600;
      info.daylightOffset = 0;
    }

    Serial.println(String("Configuring time with offset: ") + info.gmtOffset + " and DST Offset: " + info.daylightOffset);
    configTime(info.gmtOffset, info.daylightOffset, "pool.ntp.org", "time.nist.gov", "time.windows.com");
  }

  static String prevApiKey;
  String apiKey = Portal.getSetting("ipgeolocation_apikey");
  if (apiKey != prevApiKey)
  {
    prevApiKey = apiKey;
    getLocationFromIP();
  }

  static int prevPort;
  static String prevServer;
  static bool prevTls;
  static String prevClientid;
  static String prevUser;
  static String prevPasswd;

  int port = Portal.getSetting("mqttport").toInt();
  String server = Portal.getSetting("mqttserver");
  bool tls = Portal.getSetting("mqtttls").toInt();
  String clientid = Portal.getSetting("mqttclientid");
  String user = Portal.getSetting("mqttuser");
  String passwd = Portal.getSetting("mqttpasswd");

  Serial.println(String("diff: ") + port + "==" + prevPort + ", " + server + "==" + prevServer + ", " + tls + "==" + prevTls + ", " + clientid + "==" + prevClientid + ", " + user + "==" + prevUser + ", " + passwd + "==" + prevPasswd + ", ");
  if (!Mqtt.connected() || port != prevPort || server != prevServer || tls != prevTls || clientid != prevClientid || user != prevUser || passwd != prevPasswd)
  {
    prevPort = port;
    prevServer = server;
    prevTls = tls;
    prevClientid = clientid;
    prevUser = user;
    prevPasswd = passwd;

    if (server.length() > 0 && port > 0)
      mqttConnect(server, port, tls, clientid, user, passwd);
  }
}

static String settingsChanged(AutoConnectAux &aux, PageArgument &args)
{
  if (args.size() > 0)
  {
    Portal.saveParams(aux, args);
    // handle update in main loop: limits stack depth
    flagUpdateSettings = true;
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
  case SYSTEM_EVENT_AP_STA_GOT_IP6:
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    break;
  default:
    break;
  }
}

/**
 * @brief Arduino setup routine, containing one time initialization
 * 
 */
void setup()
{
  // Create RTOS items
  xTaskCreate(taskIpgeolocationHandler, "ipgeolocation", 10000, NULL, 1, &taskIpgeolocation);

  // Initialize Serial Speed
  Serial.begin(115200);

  // Show initializing message (this takes a while due to MD5sum calculation etc.)
  printStartupInfo();

  // Show startup display
  displayInit();
  displayStart();

  Serial.println("Init SPIFFS...");
  SPIFFS.begin(true);

  mqttSetup();

  Serial.println("Reset WIFI...");
  resetWiFi();

  // Start OTA
  //  startOTA(display, &startOTA);

  Serial.println("Start Webserver...");

  if (Portal.begin())
  {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    Serial.println("Start MDNS...");
    if (MDNS.begin("RoboMowRC"))
      MDNS.addService("http", "tcp", 80);
  }
  Portal.on("/settings", settingsChanged);
  WiFi.onEvent(WiFiEvent);

  Serial.println("Update Settings...");
  updateSettings();

  Serial.println("Starting Main Loop...");
  displayStart();

  getLocationFromIP();
}

/**
 * @brief Arduino main loop
 *        
 */
void loop()
{

  //ArduinoOTA.handle();

  /* 
  if (millis() - lastpub > MQTT_UPDATE_INTERVAL) {
    if (!Mqtt.connected()) {
      mqttConnect();
    }
  }
*/
  if (WiFi.isConnected())
  {
    if (!Mqtt.connected())
      Mqtt.connect();
  }

  Portal.handleClient();

  static unsigned long prevSec = 0;
  if (millis() > prevSec + 1000)
  {
    prevSec = millis();
    displayStart();

    // get the timezoneooffset at the start of every hour to allow for DST changes
    static int prevHour = 0;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10) && timeinfo.tm_hour != prevHour)
    {
      prevHour = timeinfo.tm_hour;
      if (WiFi.isConnected())
        getLocationFromIP();
    }
  }

  if (flagUpdateSettings)
  {
    updateSettings();
    if (WiFi.isConnected())
      mqttPublishLocation();
    flagUpdateSettings = false;
  }

  if (WiFi.isConnected())
  {
    static unsigned long prevMqtt = 0;
    if (millis() > prevMqtt + 60000)
    {
      prevMqtt = millis();
      mqttPublishStats();
    }
  }
}
