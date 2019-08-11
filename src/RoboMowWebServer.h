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
 * Created : Friday June 21st 2019 01:52:22 by Arjan Mels <github@mels.email>
 * Modified: Friday June 21st 2019 01:52:22 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __ROBOMOWWEBSERVER_H_
#define __ROBOMOWWEBSERVER_H_

#include <WebServer.h>
#include <AutoConnect.h>
#include <AutoConnectCredential.h>
#include <cpp_utils/CPPNVS.h>

extern NVS nvs;

class AutoConnectSpiffs : public AutoConnect
{
    using AutoConnect::AutoConnect;

public:
    bool saveParamsToNVM(AutoConnectAux &aux);
    static bool saveParamsToNVM(PageArgument &args);
    bool static loadParamsFromNVM(AutoConnectAux &aux);
    static String saveParams(AutoConnectAux &aux, PageArgument &args);
    AutoConnectAux *loadConfig(String pageName);
    AutoConnectAux *loadConfigAux(String pageName, const __FlashStringHelper *auxString);
    AutoConnectAux *loadConfigAux(String pageName, const char *auxString);
};

bool startCP(IPAddress ip);

class RoboMowRCPortal : public AutoConnectSpiffs
{
    using AutoConnectSpiffs::AutoConnectSpiffs;

public:
    AutoConnectAux *home;
    AutoConnectAux *settings;

    String getSetting(String setting);

    uint8_t hexCharToValue(char c);

    std::vector<uint8_t> getSettingAsByteArray(String setting);
    bool begin();
};

extern RoboMowRCPortal Portal;

#endif