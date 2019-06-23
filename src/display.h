/*
 * FaMe IT ECG Filter for Cardiac Booster
 * 
 * Copyright (c) 2019 FaMe IT
 * 
 * Created : Saturday May 25th 2019 10:31:25 by Arjan Mels <admin@fame-it.net>
 * Modified: Saturday May 25th 2019 10:31:25 by Arjan Mels <admin@fame-it.net>
 * 
 * Revisions:
 * 28-05-2019	AM	Initial Version
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>
#include <SSD1306Wire.h>
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

    display.display();
}

#endif