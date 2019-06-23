/*
 * FaMe IT ECG Filter for Cardiac Booster
 * 
 * Copyright (c) 2019 FaMe IT
 * 
 * Created : Thursday May 16th 2019 09:21:23 by Arjan Mels <admin@fame-it.net>
 * Modified: Thursday May 16th 2019 09:21:23 by Arjan Mels <admin@fame-it.net>
 * 
 * Revisions:
 * 28-05-2019	AM	Initial Version
 */

#ifndef _OTA_H_
#define _OTA_H_

#include "Arduino.h"
#include "SSD1306Wire.h"

void startOTA(OLEDDisplay &display, void startOTA())
{
    ArduinoOTA
        .setPassword("CBFilterYUIfbr785324dfhgkjf5")
        .onStart([&display, startOTA]() {
            startOTA();

            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "program";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);

            display.clear();
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.setFont(ArialMT_Plain_16);
            display.drawString(0, 0, "Updating " + type);
            display.display();
        })
        .onEnd([&display]() {
            Serial.println("\nEnd");
        })
        .onProgress([&display](unsigned int progress, unsigned int total) {
            display.drawProgressBar(0, 30, 127, 10, (progress / (total / 100)));
            display.display();

            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([&display](ota_error_t error) {
            display.drawString(0, 0, String("Error %u") + error);

            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
            ESP.restart();
        });

    ArduinoOTA.begin();
}

#endif