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
            "name": "ntpserver",
            "type": "ACInput",
            "value": "pool.ntp.org",
            "placeholder": "NTP / Time server",
            "label": "NTP server",
            "pattern": "(^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$)|(^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])(\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))*$)"
        },
        {
            "name": "ipgeolocation_apikey",
            "type": "ACInput",
            "value": "",
            "placeholder": "API key for ipgeolocation.io",
            "label": "ipgeolocation API Key",
            "options": "(^[A-Fa-f0-9]{32}$)|(^$)"
        },
        {
            "name": "timezone",
            "type": "AutoConnectSelect",
            "selected": "Auto",
            "label": "TimeZone",
            "options": [
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
            "name": "mqttserver",
            "type": "ACInput",
            "value": "",
            "placeholder": "MQTT broker server",
            "label": "Server",
            "pattern": "(^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$)|(^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])(\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))*$)"
        },
        {
            "name": "channelid",
            "type": "ACInput",
            "value": "",
            "label": "Channel ID"
        },
        {
            "name": "userkey",
            "type": "ACInput",
            "value": "NRTFYGJ6TJFGX4RC",
            "label": "User Key"
        },
        {
            "name": "apikey",
            "type": "ACInput",
            "value": "",
            "label": "API Key"
        },
        {
            "name": "period",
            "type": "ACRadio",
            "label": "Update period",
            "value": [
                "30 sec.",
                "60 sec.",
                "180 sec."
            ],
            "arrange": "vertical",
            "checked": 1
        },
        {
            "name": "newline",
            "type": "ACElement",
            "value": "<hr>"
        },
        {
            "name": "uniqueid",
            "type": "ACCheckbox",
            "value": "unique",
            "label": "Use APID unique",
            "checked": false
        },
        {
            "name": "hostname",
            "type": "ACInput",
            "label": "ESP host name",
            "value": ""
        },
        {
            "name": "save",
            "type": "ACSubmit",
            "value": "Apply",
            "uri": "/mqtt_setting"
        },
        {
            "name": "cancel",
            "type": "ACSubmit",
            "value": "Cancel",
            "uri": "/"
        }
    ]
}
)---";

#endif