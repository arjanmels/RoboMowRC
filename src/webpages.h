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
 * Created : Saturday June 22nd 2019 10:23:18 by Arjan Mels <github@mels.email>
 * Modified: Saturday June 22nd 2019 10:23:18 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __WEBPAGES_H_
#define __WEBPAGES_H_

#include <Arduino.h>

// uri longer then 5 chars or so otherwise unexpected stack smashing
const char HomeSettings[] PROGMEM = R"---(
{
    "title": "Home",
    "uri": "/homepage",
    "menu": false,
    "element": [
        {
            "name": "style",
            "type": "ACStyle",
            "value": "label+input,label+select{position:sticky;left:250px;width:300px!important;box-sizing:border-box;}"
        },
        {
            "name": "robomowstatus",
            "type": "ACElement",
            "value": "<div id='robomowstatus'></div>"
        },
        {
            "name": "robomowschedule",
            "type": "ACCheckbox",
            "value": "robomowschedule",
            "checked": "false",
            "label": "Schedule Enabled",
            "labelposition": "infront"
        }
    ]
}
)---";

const char AuxSettings[] PROGMEM = R"---(
{
    "title": "Settings",
    "uri": "/settings",
    "menu": true,
    "element": [
        {
            "name": "style",
            "type": "ACStyle",
            "value": "label+input,label+select { position:sticky; left: 250px; width:300px!important; box-sizing: border-box;}"
        },
        {
            "name": "mbserial",
            "type": "ACInput",
            "value": "",
            "placeholder": "RoboMow motherboard serial number",
            "label": "RoboMow serial",
            "pattern": "[0-9]{13}"
        },
        {
            "name": "newline1",
            "type": "ACElement",
            "value": "<hr>"
        },
        {
            "name": "mqttserver",
            "type": "ACInput",
            "value": "",
            "placeholder": "MQTT server",
            "label": "MQTT Server",
            "pattern": "((?:\d{1,3}\.){3}\d{1,3})|(([a-zA-Z0-9]|([a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9]))"
        },
        {
            "name": "mqttport",
            "type": "ACInput",
            "value": "1883",
            "placeholder": "default 1883 or for TLS 8883",
            "label": "MQTT port"
        },
        {
            "name": "mqtttls",
            "type": "ACCheckbox",
            "value": "mqttls",
            "checked": "false",
            "label": "MQTT use TLS",
            "labelposition": "infront"
        },
        {
            "name": "mqttuser",
            "type": "ACInput",
            "value": "",
            "placeholder": "MQTT user name",
            "label": "MQTT user name"
        },
        {
            "name": "mqttpasswd",
            "type": "ACInput",
            "value": "",
            "placeholder": "MQTT password",
            "label": "MQTT password"
        },
        {
            "name": "mqttclientid",
            "type": "ACInput",
            "value": "RoboMowRC",
            "placeholder": "MQTT client id",
            "label": "MQTT client id"
        },
        {
            "name": "newline3",
            "type": "ACElement",
            "value": "<hr>"
        },
        {
            "name": "loranetworkkey",
            "type": "ACInput",
            "value": "",
            "placeholder": "LoRa Network Session Key",
            "label": "LoRa Network Key",
            "pattern": "[A-Fa-f0-9]{32}"
        },
        {
            "name": "loraappkey",
            "type": "ACInput",
            "value": "",
            "placeholder": "LoRa Application Session Key",
            "label": "LoRa Application Key",
            "pattern": "[A-Fa-f0-9]{32}"
        },
        {
            "name": "loradeviceaddr",
            "type": "ACInput",
            "value": "",
            "placeholder": "LoRa Device Address",
            "label": "LoRa Device Address",
            "pattern": "[A-Fa-f0-9]{8}"
        },
        {
            "name": "newline4",
            "type": "ACElement",
            "value": "<hr>"
        },
        {
            "name": "timezone",
            "type": "ACSelect",
            "selected": "Auto",
            "label": "TimeZone",
            "option": [
                "Auto",
                "GMT -12",
                "GMT -11.5",
                "GMT -11",
                "GMT -10.5",
                "GMT -10",
                "GMT -9.5",
                "GMT -9",
                "GMT -8.5",
                "GMT -8",
                "GMT -7.5",
                "GMT -7",
                "GMT -6.5",
                "GMT -6",
                "GMT -5.5",
                "GMT -5",
                "GMT -4.5",
                "GMT -4",
                "GMT -3.5",
                "GMT -3",
                "GMT -2.5",
                "GMT -2",
                "GMT -1.5",
                "GMT -1",
                "GMT -0.5",
                "GMT",
                "GMT +0.5",
                "GMT +1",
                "GMT +1.5",
                "GMT +2",
                "GMT +2.5",
                "GMT +3",
                "GMT +3.5",
                "GMT +4",
                "GMT +4.5",
                "GMT +5",
                "GMT +5.5",
                "GMT +6",
                "GMT +6.5",
                "GMT +7",
                "GMT +7.5",
                "GMT +8",
                "GMT +8.5",
                "GMT +9",
                "GMT +9.5",
                "GMT +10",
                "GMT +10.5",
                "GMT +11",
                "GMT +11.5",
                "GMT +12",
                "GMT +12.5",
                "GMT +13",
                "GMT +13.5",
                "GMT +14"
            ]
        },
        {
            "name": "geoapikey",
            "type": "ACInput",
            "value": "",
            "placeholder": "API key for ipgeolocation.io",
            "label": "ipgeolocation API Key",
            "pattern": "[A-Fa-f0-9]{32}"
        },
        {
            "name": "save",
            "type": "ACSubmit",
            "value": "Apply",
            "uri": "/settings"
        },
        {
            "name": "cancel",
            "type": "ACButton",
            "value": "Cancel",
            "action": "window.location.reload()"
        }
    ]
}
)---";

const char AuxSettings2[] PROGMEM = R"---(
{
    "title": "Settings2",
    "uri": "/settings2",
    "menu": true,
    "element": [
        {
            "name": "style2",
            "type": "ACStyle",
            "value": "label+input,label+select { position:sticky; left: 250px; width:300px!important; box-sizing: border-box;}"
        },
        {
            "name": "mbserial2",
            "type": "ACInput",
            "value": "",
            "placeholder": "RoboMow motherboard serial number",
            "label": "RoboMow serial",
            "pattern": "[0-9]{13}"
        }
    ]
}
)---";

#endif