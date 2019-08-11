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
 * Created : Wednesday June 26th 2019 06:13:30 by Arjan Mels <github@mels.email>
 * Modified: Wednesday June 26th 2019 06:13:30 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __INFO_H_
#define __INFO_H_

#include <Arduino.h>

struct Info
{
  String timezone;
  int gmtOffset = 0;
  int daylightOffset = 0;

  String source;
  float latitude;
  float longitude;
  float altitude;
  int satellites;

  String firmwareMD5;
};

extern Info info;

#endif