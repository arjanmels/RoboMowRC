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

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>
#include <SSD1306Wire.h>
#include <time.h>
#include "config.h"

#define OLED_RST 16
#define OLED_SDA 4
#define OLED_SCL 15
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

void displayInit()
{
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(100);
    digitalWrite(OLED_RST, HIGH);
    display.init();
}

void displayStart()
{
    display.flipScreenVertically();
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 20, "RoboMow RC");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 44, "© 2019 FaMe IT");

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(128, 54, WiFi.localIP().toString());

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10))
    {
        char buff[20];
        strftime(buff, 20, "%H:%M:%S", &timeinfo);

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 54, buff);
    }

    display.display();
}

#endif