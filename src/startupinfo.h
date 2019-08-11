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
 * Created : Thursday June 20th 2019 08:23:10 by Arjan Mels <github@mels.email>
 * Modified: Thursday June 20th 2019 08:23:10 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __STARTUPINF_H_
#define __STARTUPINF_H_

#include <Arduino.h>
#include "info.h"

/**
 * @brief Get Flash Mode Description
 * 
 * @return const char* Description of Flash Mode
 */
const char *getFlashModeDescription()
{
  switch (ESP.getFlashChipMode())
  {
  case FM_QIO:
    return "QIO";
  case FM_QOUT:
    return "QOUT";
  case FM_DIO:
    return "DIO";
  case FM_DOUT:
    return "DOUT";
  case FM_FAST_READ:
    return "Fast Read";
  case FM_SLOW_READ:
    return "Slow Read";
  default:
    return "Unknown";
  }
}

#define FWVERSION

/**
 * @brief Print startup information
 * 
 */
void printStartupInfo()
{
  info.firmwareMD5 = ESP.getSketchMD5();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("FaMe IT RoboMow Remote Control");
  Serial.println("Compiled on " __DATE__ " at " __TIME__);
  Serial.println("Copyright © 2019 FaMe IT, The Netherlands");
  Serial.println();
  Serial.println();
  Serial.printf("Running on: ESP32 r%d @ %d MHz on Arduino %d.%d.%d, SDK %s\n", ESP.getChipRevision(), ESP.getCpuFreqMHz(), ARDUINO / 10000, ARDUINO / 100 % 100, ARDUINO % 100, ESP.getSdkVersion());
  Serial.printf("Code Size:  %d kByte (MD5: %s)\n", ESP.getSketchSize() / 1024, info.firmwareMD5.c_str());
  Serial.printf("Flash Size: %d kByte (%s @ %d MHz)\n", ESP.getFlashChipSize() / 1024, getFlashModeDescription(), ESP.getFlashChipSpeed() / 1000 / 1000);
  Serial.printf("PSRAM Size: %d kByte / Free: %d kByte\n", ESP.getPsramSize() / 1024, ESP.getFreePsram() / 1024);
  Serial.printf("Heap Size:  %d kByte / Free: %d kByte\n", +ESP.getHeapSize() / 1024, ESP.getFreeHeap() / 1024);

  Serial.println();
  Serial.println();
}

#endif