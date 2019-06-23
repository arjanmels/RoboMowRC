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
#include <PubSubClient.h>

#include "robomowwebserver.h"

#include <PageBuilder.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>

//#include "config.h"

#include "buffer.h"
#include "images.h"
//#include "OTA.h"
#include "touchkey.h"
#include "display.h"
#include "startupinfo.h"

// Define Hardware components
TouchKey keyDown(12);
TouchKey keyUp(13);

PubSubClient Mqtt;

void rootPage()
{
  char content[] = "Hello, world";
  Portal.host().send(200, "text/plain", content);
}
/*
bool mqttConnect() {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";  // For random generation of client ID.
  char    clientId[9];

  uint8_t retry = 3;
  while (!Mqtt.connected()) {
    if (serverName.length() <= 0)
      break;

    Mqtt.setServer(serverName.c_str(), 1883);
    Serial.println(String("Attempting MQTT broker:") + serverName);

    for (uint8_t i = 0; i < 8; i++) {
      clientId[i] = alphanum[random(62)];
    }
    clientId[8] = '\0';

    if (Mqtt.connect(clientId, MQTT_USER_ID, userKey.c_str())) {
      Serial.println("Established:" + String(clientId));
      return true;
    } else {
      Serial.println("Connection failed:" + String(Mqtt.state()));
      if (!--retry)
        break;
      delay(3000);
    }
  }
  return false;
}
*/

/**
 * @brief Clean up previous WiFi Connection
 * 
 */
void ResetWiFi() {
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


/**
 * @brief Arduino setup routine, containing one time initialization
 * 
 */
void setup()
{
  // Initialize Serial Speed and print starting message
  Serial.begin(115200);
  printStartupInfo();

  SPIFFS.begin();

  // Show initializing message
  displayInit();
  displayStart();

  ResetWiFi();

  // Start OTA
//  startOTA(display, &startOTA);

  if (Portal.begin())
  {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    if (MDNS.begin("RoboMowRC"))
      MDNS.addService("http", "tcp", 80);
  }
  Serial.println("testa");
  displayStart();
  Serial.println("testb");

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
  Portal.handleClient();
  Mqtt.loop();
}
