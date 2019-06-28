#ifndef __ROBOMOWWEBSERVER_H_
#define __ROBOMOWWEBSERVER_H_

#include <WebServer.h>
#include <AutoConnect.h>
#include <AutoConnectCredential.h>
#include <Spiffs.h>
#include "settings.h"

class AutoConnectSpiffs : public AutoConnect
{
    static const size_t JSON_BUFFER_SIZE = 1000;

    using AutoConnect::AutoConnect;

public:
    bool loadFromSpiffs(const String fileName)
    {
        bool rc = false;
        File fs = SPIFFS.open(fileName, "r");
        if (fs)
        {
            rc = load(fs);
            fs.close();
        }
        else
            Serial.println("[AC+] SPIFFS open failed: " + fileName);
        return rc;
    }

    bool saveParamsToSpiffs(AutoConnectAux &aux, String fileName)
    {
        File fs = SPIFFS.open(fileName, "w");
        if (fs)
        {
            AutoConnectElementVT elements = aux.getElements();
            DynamicJsonDocument doc(JSON_BUFFER_SIZE);
            for (AutoConnectElement &element : elements)
                doc[element.name] = element.value;

            serializeJson(doc, fs);
            fs.close();
        }
        else
            Serial.println("[AC+] SPIFFS open for write failed: " + fileName);
        return true;
    }

    static bool saveParamsToSpiffs(PageArgument &args, String fileName)
    {
        if (args.size() == 0)
            return true;

        File fs = SPIFFS.open(fileName, "w");
        if (fs)
        {
            DynamicJsonDocument doc(JSON_BUFFER_SIZE);
            for (int i = 0; i < args.size(); i++)
                doc[args.argName(i)] = args.arg(i);

            serializeJson(doc, fs);
            fs.close();
        }
        else
            Serial.println("[AC+] SPIFFS open for write failed: " + fileName);
        return true;
    }

    bool static loadParamsFromSpiffs(AutoConnectAux &aux, String fileName)
    {
        File fs = SPIFFS.open(fileName, "r");
        if (fs)
        {
            DynamicJsonDocument doc(JSON_BUFFER_SIZE);
            deserializeJson(doc, fs);
            JsonObject root = doc.as<JsonObject>();
            for (JsonPair kv : root)
            {
                AutoConnectElement *element = aux.getElement(kv.key().c_str());
                if (element)
                    element->value = kv.value().as<String>();
            }
            fs.close();
        }
        else
        {
            Serial.println("[AC+] SPIFFS open for write failed: " + fileName);
        }
        return true;
    }

    static String saveParams(AutoConnectAux &aux, PageArgument &args)
    {
        saveParamsToSpiffs(args, String("/save") + aux.uri() + ".json");
        return "";
    }

    AutoConnectAux *loadConfig(String pageName)
    {
        AutoConnectAux *auxPage = aux("/" + pageName);
        if (auxPage == nullptr)
            return nullptr;
        loadParamsFromSpiffs(*auxPage, "/save/" + pageName + ".json");
        //auxPage->on(saveParams);
        return auxPage;
    }
    AutoConnectAux *loadConfigAuxFromSpiffs(String pageName)
    {
        loadFromSpiffs("/" + pageName + ".json");
        return loadConfig(pageName);
    }
    AutoConnectAux *loadConfigAux(String pageName, const __FlashStringHelper *auxString)
    {
        load(auxString);
        return loadConfig(pageName);
    }
    AutoConnectAux *loadConfigAux(String pageName, const char *auxString)
    {
        load(auxString);
        return loadConfig(pageName);
    }
};

class RoboMowRCPortal : public AutoConnectSpiffs
{
    using AutoConnectSpiffs::AutoConnectSpiffs;

public:
    AutoConnectAux *settings;

    String getSetting(String setting)
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

    bool begin()
    {
        // Start periodic timer used for sampling ADC etc.
        // needs to be befone startOTA as startOTA uses timer
        AutoConnectConfig config;
        config.autoReconnect = true;
        config.title = "RoboMow RC";
        config.apid = "RoboMowRC";
        config.psk = "12345678";
        config.bootUri = AC_ONBOOTURI_ROOT;
//        config.portalTimeout = 10;
//        config.retainPortal = false;
//        config.autoReset = false;
        config.cssExtra = "label+input,label+select { position:sticky; left: 250px; width:300px!important; box-sizing: border-box;}";
        AutoConnect::config(config);
        settings = loadConfigAux("settings", FPSTR(AuxSettings));
        if (settings == nullptr)
        {
            Serial.println("[AC+] Could not load settings");
            while (1)
                ;
        }
        return AutoConnectSpiffs::begin(nullptr, nullptr, 5000);
    }
};

RoboMowRCPortal Portal;

#endif