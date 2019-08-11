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
 * Created : Saturday July 6th 2019 06:54:02 by Arjan Mels <github@mels.email>
 * Modified: Saturday July 6th 2019 06:54:02 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __GENERAL_H_
#define __GENERAL_H_

#include <Arduino.h>
#include "cpp_utils/CPPNVS.h" 

extern void hexDump(uint8_t *data, size_t length);

extern NVS nvs;


#endif
