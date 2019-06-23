#ifndef __STARTUPINF_H_
#define __STARTUPINF_H_

#include <Arduino.h>

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

/**
 * @brief Print startup information
 * 
 */
void printStartupInfo()
{
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("FaMe IT RoboMow Remote Control");
  Serial.println("Compiled on " __DATE__ " at " __TIME__);
  Serial.println("Copyright © 2019 FaMe IT, The Netherlands");
  Serial.println();
  Serial.println();
  Serial.println(String("Running on: ESP32 r") + ESP.getChipRevision() + " @ " + ESP.getCpuFreqMHz() + "MHz on Arduino " + ARDUINO / 10000 + "." + ARDUINO / 100 % 100 + "." + ARDUINO % 100 + ", SDK " + ESP.getSdkVersion());
  Serial.println(String("Code Size:  ") + ESP.getSketchSize() / 1024 + " kByte (MD5: " + ESP.getSketchMD5() + ")");
  Serial.println(String("Flash Size: ") + ESP.getFlashChipSize() / 1024 + " kByte (" + getFlashModeDescription() + " @ " + ESP.getFlashChipSpeed() / 1000 / 1000 + "MHz)");
  Serial.println(String("PSRAM Size: ") + ESP.getPsramSize() / 1024 + " kByte");
  Serial.println();
  Serial.println();
}


#endif