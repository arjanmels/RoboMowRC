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

#include <WebServer.h>
#include <AutoConnect.h>
#include <AutoConnectCredential.h>
#include <cpp_utils/CPPNVS.h>
#include "webpages.h"
#include "RoboMowWebServer.h"

extern NVS nvs;

bool AutoConnectSpiffs::saveParamsToNVM(AutoConnectAux &aux)
{
    AutoConnectElementVT elements = aux.getElements();
    for (AutoConnectElement &element : elements)
        nvs.set(element.name, element.value);

    nvs.commit();

    return true;
}

bool AutoConnectSpiffs::saveParamsToNVM(PageArgument &args)
{
    if (args.size() == 0)
        return true;

    for (int i = 0; i < args.size(); i++)
        nvs.set(args.argName(i), args.arg(i));

    nvs.commit();

    return true;
}

bool AutoConnectSpiffs::loadParamsFromNVM(AutoConnectAux &aux)
{
    for (AutoConnectElement &element : aux.getElements())
    {
        String str = nvs.get(element.name);
        if (str.length() > 0)
        {
            element.value = str;
        }
    }
    return true;
}

String AutoConnectSpiffs::saveParams(AutoConnectAux &aux, PageArgument &args)
{
    saveParamsToNVM(args);
    return "";
}

AutoConnectAux *AutoConnectSpiffs::loadConfig(String pageName)
{
    AutoConnectAux *auxPage = aux("/" + pageName);
    if (auxPage == nullptr)
        return nullptr;
    loadParamsFromNVM(*auxPage);
    //auxPage->on(saveParams);
    return auxPage;
}

AutoConnectAux *AutoConnectSpiffs::loadConfigAux(String pageName, const __FlashStringHelper *auxString)
{
    load(auxString);
    return loadConfig(pageName);
}

AutoConnectAux *AutoConnectSpiffs::loadConfigAux(String pageName, const char *auxString)
{
    load(auxString);
    return loadConfig(pageName);
}

bool startCP(IPAddress ip)
{
    Serial.println("Captive Portal started at IP: " + ip.toString());
    return true;
}

String RoboMowRCPortal::getSetting(String setting)
{
    AutoConnectElement *element = settings->getElement(setting);
    if (element == nullptr)
        return String();
    if (element->typeOf() == AC_Input)
    {
        if (!element->as<AutoConnectInput>().isValid())
            return String();
        else
            return element->value;
    }
    else if (element->typeOf() == AC_Checkbox)
    {
        return element->as<AutoConnectCheckbox>().checked ? "1" : "0";
    }
    else
    {
        return element->value;
    }
}

uint8_t RoboMowRCPortal::hexCharToValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0' + 0x0;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 0xA;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 0xA;
    else
    {
        log_e("Illegal character in hex string: %c", c);
        return 0;
    }
}

std::vector<uint8_t> RoboMowRCPortal::getSettingAsByteArray(String setting)
{
    String value = getSetting(setting);

    std::vector<uint8_t> res(value.length() / 2);

    for (int i = 0; i < value.length(); i += 2)
        res[i / 2] = (hexCharToValue(value[i]) << 4) | hexCharToValue(value[i + 1]);
    return res;
}

RoboMowRCPortal Portal;

void handleNotFound()
{
    Portal.host().sendHeader("Location", "/home", true);
    Portal.host().send(302, "text/html", "<html><body>Redirecting to <a href=\"/home\">Home Page</a></body></html>");
}

bool RoboMowRCPortal::begin()
{
    onDetect(startCP);
    onNotFound(handleNotFound);

    AutoConnectConfig config;
    config.autoReconnect = true;
    config.title = "RoboMow RC";
    config.apid = "RoboMowRC";
    config.psk = "12345678";
    config.portalTimeout = 30000;
    config.bootUri = AC_ONBOOTURI_HOME;
    config.homeUri = "/home";
    //        config.retainPortal = false;
    //        config.autoReset = false;
    AutoConnect::config(config);

    //    home = loadConfigAux("home", FPSTR(HomeSettings));
    settings = loadConfigAux("settings", FPSTR(AuxSettings));

    if (/* home == nullptr || */ settings == nullptr)
    {
        log_e("[AC+] Could not load settings");
        while (1)
            ;
    }

    return AutoConnectSpiffs::begin(nullptr, nullptr, 5000);
}
