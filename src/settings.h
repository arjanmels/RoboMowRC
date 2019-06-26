#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include <Arduino.h>

const char AuxSettings[] PROGMEM = R"---(
{
    "title": "Settings",
    "uri": "/settings",
    "menu": true,
    "element": [
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
            "name": "ipgeolocation_apikey",
            "type": "ACInput",
            "value": "",
            "placeholder": "API key for ipgeolocation.io",
            "label": "ipgeolocation API Key",
            "pattern": "(^[A-Fa-f0-9]{32}$)|(^$)"
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
            "pattern": "(^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$)|(^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])(\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))*$)"
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
            "value": "true",
            "checked": "false",
            "label": "MQTT use TLS"
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

#endif