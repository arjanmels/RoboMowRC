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
 * Created : Wednesday July 3rd 2019 10:30:43 by Arjan Mels <admin@fame-it.net>
 * Modified: Wednesday July 3rd 2019 10:30:43 by Arjan Mels <admin@fame-it.net>
 * 
 * Revisions:
 */

#ifndef __CONFIG_H_
#define __CONFIG_H_

#define GPS_MAX_AGE_FOR_TIMEZONE (60 * 60 * 1000)
#define GPS_MAX_AGE_FOR_LORA (5 * 60 * 1000)

#define LORA_FRAMECOUNTSTOREINTERVAL 10

#define CUSTOM_STACK_SIZE (15000)

#define CFG_MQTTSERVER F("mqttserver")
#define CFG_MQTTPORT F("mqttport")
#define CFG_MQTTTLS F("mqtttls")
#define CFG_MQTTUSER F("mqttuser")
#define CFG_MQTTPASSWD F("mqttpasswd")
#define CFG_MQTTCLIENTID F("mqttclientid")

#define CFG_LORANETWORKKEY F("loranetworkkey")
#define CFG_LORAAPPKEY F("loraappkey")
#define CFG_LORADEVICEADDR F("loradeviceaddr")
#define CFG_LORAFRAMECOUNT F("loraframecount")

#define CFG_TIMEZONE F("timezone")
#define CFG_GEOAPIKEY F("geoapikey")
#define CFG_MBSERIAL F("mbserial")

#endif