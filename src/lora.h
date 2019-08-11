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
 * Created : Friday June 28th 2019 08:44:42 by Arjan Mels <github@mels.email>
 * Modified: Friday June 28th 2019 08:44:42 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __LORA_H_
#define __LORA_H_

#include <Arduino.h>
#include <TinyLoRa.h>
#include <SPI.h>
#include <CayenneLPP.h>
#include <TinyGPS++.h>
#include "config.h"
#include "RoboMowWebServer.h"

// Network Session Key (MSB)
// Application Session Key (MSB)
// Device Address (MSB)
#define TTN
#ifdef TTN
// The Things Network
uint8_t NwkSkey[16] = {0x08, 0x1B, 0x1F, 0xC1, 0x71, 0x6F, 0x3D, 0x61, 0x8B, 0x2E, 0x8A, 0xF5, 0x4A, 0x72, 0xC7, 0xEF};
uint8_t AppSkey[16] = {0x14, 0x01, 0x64, 0xE1, 0xA8, 0x30, 0x9F, 0x4E, 0xCB, 0x7C, 0x8A, 0x18, 0x9D, 0xC1, 0xF1, 0x81};
uint8_t DevAddr[4] = {0x26, 0x01, 0x19, 0xD4};
#else
// KPN
uint8_t NwkSkey[16] = {0x22, 0xb1, 0xb4, 0xb1, 0xf6, 0xba, 0x97, 0xba, 0xaa, 0xc0, 0x40, 0x2c, 0xe7, 0x99, 0x8d, 0x6d};
uint8_t AppSkey[16] = {0x4e, 0x66, 0x1f, 0xea, 0x9a, 0x2b, 0x51, 0x2b, 0xf2, 0xeb, 0x87, 0x42, 0x9f, 0x6a, 0x32, 0x43};
uint8_t DevAddr[4] = {0x14, 0x20, 0x3C, 0x51};
#endif

// Pinout for Adafruit Feather 32u4 LoRa
TinyLoRa lora = TinyLoRa(26, 18);
CayenneLPP lpp(100);

static uint32_t framecountNVM;

void setupLora()
{

    log_i("Start LoRa");

    pinMode(23, OUTPUT);
    digitalWrite(23, LOW);
    delay(1);
    pinMode(23, INPUT);
    delay(10);

    lora.setChannel(CH2); // normally use MULTI, CH2 is for testing only
    // set datarate
    lora.setDatarate(SF7BW125); // 10 is the maximum static SF (11 & 12 can only be used together with ADR)
    if (!lora.begin(5, 19, 27))
    {
        log_e("LoRa initialization failed!");
        while (true)
            ;
    }
    nvs.get(CFG_LORAFRAMECOUNT, framecountNVM);
    lora.frameCounter = framecountNVM;
    log_i("LoRa framecounter: %u", lora.frameCounter);
}

void loopLora()
{
}

void sendLocationLora(uint8_t status, float lat = 0.0, float lon = 0.0, float alt = 0.0, uint32_t age = UINT32_MAX)
{
    static uint32_t prev5min = -5 * 60 * 1000;
    static uint32_t prev15min = -14 * 60 * 1000; // wait one minute max to acquire gps
    static uint32_t prev6hours = -6 * 60 * 60 * 1000;
    static float prevLat, prevLon, prevAlt;
    uint32_t nowmillis = millis();
    if ((nowmillis - prev15min > 15 * 60 * 1000) || (nowmillis - prev5min > 5 * 60 * 1000 && (TinyGPSPlus::distanceBetween(lat, lon, prevLat, prevLon) > 100 || fabs(alt - prevAlt) > 25)))
    {
        prev15min = nowmillis;
        prev5min = nowmillis;
        prevLat = lat;
        prevLon = lon;
        prevAlt = alt;

        lora.setNwkSkey(Portal.getSettingAsByteArray(CFG_LORANETWORKKEY).data());
        lora.setAppSkey(Portal.getSettingAsByteArray(CFG_LORAAPPKEY).data());
        lora.setDevAddr(Portal.getSettingAsByteArray(CFG_LORADEVICEADDR).data());

        // send message with SF12 every 6 hours to ensure best possible reception without exceeding 1% duty cycle and TTN 30 sec per day limit
        if (nowmillis - prev6hours > 6 * 60 * 60 * 1000)
        {
            prev6hours = nowmillis;
            lora.setDatarate(SF12BW125);
            log_i("Sending LoRa message with SF12: framecount: %u, status: %d, location: %.5f, %.5f, %.5f, age: %d", lora.frameCounter, status, lat, lon, alt, age / 1000);
        }
        else
        {
            lora.setDatarate(SF7BW125);
            log_i("Sending LoRa message with SF7: framecount: %u, status: %d, location: %.5f, %.5f, %.5f, age: %d", lora.frameCounter, status, lat, lon, alt, age / 1000);
        }

        time_t now;
        time(&now);
        lpp.reset();
        lpp.addDigitalOutput(1, status);
        lpp.addGPS(2, lat, lon, alt);
        if (age == UINT32_MAX)
            lpp.addUnixTime(3, 0);
        else
            lpp.addUnixTime(3, now - (age / 1000));

        if (lora.frameCounter >= framecountNVM)
        {
            framecountNVM += LORA_FRAMECOUNTSTOREINTERVAL;
            nvs.set(CFG_LORAFRAMECOUNT, framecountNVM);
            nvs.commit();
        }

        lora.sendData(lpp.getBuffer(), lpp.getSize(), lora.frameCounter, age > GPS_MAX_AGE_FOR_LORA ? 2 : 1);
        lora.frameCounter++;
    }
}

#endif