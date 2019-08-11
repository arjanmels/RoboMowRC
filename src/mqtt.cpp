
#include <Arduino.h>
#include <PubSubClient.h>
#include <config.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <info.h>
#include "RoboMowWebServer.h"

PubSubClient Mqtt;
String clientPath;

void mqttPublish(String topic, String value)
{
    Mqtt.publish((clientPath + topic).c_str(), value.c_str(), true);
}

String getGMT(long offset = 0)
{
    time_t now;
    time(&now);
    now += offset;
    struct tm timeinfo;
    if (gmtime_r(&now, &timeinfo))
    {
        char buff[20];
        strftime(buff, 20, "%FT%TZ", &timeinfo);
        return String(buff);
    }
    return "";
}

void mqttPublishStats()
{
    mqttPublish("$stats/interval", "60");
    mqttPublish("$stats/uptime", String((long)(esp_timer_get_time() / 1000000)));

    mqttPublish("$localip", WiFi.localIP().toString());
    mqttPublish("stats/ip", WiFi.localIP().toString());
    mqttPublish("stats/wifistrength", String(WiFi.RSSI() + 100));
    mqttPublish("stats/currenttime", getGMT());
}

void mqttPublishLocation()
{
    mqttPublish("location/source", info.source.length() ? info.source : "none");
    mqttPublish("location/timezone", info.timezone);
    mqttPublish("location/latitude", String(info.latitude, 5));
    mqttPublish("location/longitude", String(info.longitude, 5));
    mqttPublish("location/altitude", String(info.altitude, 2));
    mqttPublish("location/satellites", String(info.satellites));
}

void mqttPublishNode(String node, String name, String type, String properties)
{
    mqttPublish(node + "/$name", name);
    mqttPublish(node + "/$type", type);
    mqttPublish(node + "/$properties", properties);
}

void mqttPublishProperty(String property, String name, String datatype = "string", String unit = "", String format = "", boolean setable = false)
{
    mqttPublish(property + "/$name", name);
    if (datatype.length() > 0)
        mqttPublish(property + "/$datatype", datatype);
    if (unit.length() > 0)
        mqttPublish(property + "/$unit", unit);
    if (format.length() > 0)
        mqttPublish(property + "/$format", format);
    if (setable)
        mqttPublish(property + "/$settable", "true");

    mqttPublish(property, "");
}

void onMqttConnect(bool sessionPresent)
{
    clientPath = "homie/" + Portal.getSetting(CFG_MQTTCLIENTID) + "/";

    Serial.println(String("Mqtt server connected; session present: ") + sessionPresent);

    mqttPublish("$homie", "4.0");
    mqttPublish("$name", "RoboMow RC");
    mqttPublish("$state", "init");
    mqttPublish("$implementation", "RoboMowRC");
    mqttPublish("$extensions", "org.homie.legacy-stats,org.homie.legacy-firmware");
    mqttPublish("$nodes", "stats,location,robomow,robomowversion");

    mqttPublish("$mac", WiFi.macAddress());
    mqttPublish("$fw/name", "RoboMowRC");
    mqttPublish("$fw/version", info.firmwareMD5);
    mqttPublish("$stats", "");
    mqttPublish("$stats/interval", "60");

    mqttPublishNode("stats", "Statistics", "statistics", "starttime,currenttime,ip,mac,fwversion,wifistrength,blestrength,bleconnected");
    mqttPublishProperty("stats/starttime", "Connected since");
    mqttPublishProperty("stats/currenttime", "Last update");
    mqttPublishProperty("stats/ip", "IP address");
    mqttPublishProperty("stats/mac", "MAC address");
    mqttPublishProperty("stats/fwversion", "Firmware version");
    mqttPublishProperty("stats/wifistrength", "WiFi signal strength", "integer", "%", "0-100");
    mqttPublishProperty("stats/blestrength", "BLE signal strength", "integer", "%", "0-100");
    mqttPublishProperty("stats/bleconnected", "BLE connected", "boolean");

    mqttPublishNode("location", "Location", "location", "source,latitude,longitude,altitude,timezone,satellites");
    mqttPublishProperty("location/source", "Location source", "enum", "", "none,ip,lorawan,gps");
    mqttPublishProperty("location/latitude", "Latitude", "float", "°", "-90:90");
    mqttPublishProperty("location/longitude", "Longitude", "float", "°", "-180:180");
    mqttPublishProperty("location/altitude", "Altitude", "float", "m");
    mqttPublishProperty("location/timezone", "Timezone");
    mqttPublishProperty("location/satellites", "Satellites", "integer", "#");

    mqttPublishNode("robomow", "RoboMow ", "robomow", "mode,status,statusdescription,battery,theftprotection,theftprotectionactive,"
                                                      "theftprotectionactive,theftprotectiontempdisable,mowmotoractive,searchdockstation,automaticedge,automaticscan,"
                                                      "nextdepart,automaticoperationsince,childprotection");

    mqttPublishProperty("robomow/mode", "Mode", "enum", "", "Idle,Charging,Automatic,RemoteControlled,Bit");
    mqttPublishProperty("robomow/status", "Status");
    mqttPublishProperty("robomow/statusdescription", "Status Description");
    mqttPublishProperty("robomow/battery", "Remaining battery", "integer", "%", "0:100");
    mqttPublishProperty("robomow/theftprotection", "Theft Protection Enabled", "boolean", "", "", true);
    mqttPublishProperty("robomow/theftprotectionactive", "Theft Protection Active", "boolean", "", "", true);
    mqttPublishProperty("robomow/theftprotectiontempdisable", "Theft Protection Temporarily Disabled", "boolean", "", "", true);

    mqttPublishProperty("robomow/mowmotoractive", "Mow Motor Active", "boolean", "", "", true);
    mqttPublishProperty("robomow/searchdockstation", "Searching Docking Station", "boolean", "", "", true);
    mqttPublishProperty("robomow/automaticedge", "Automatic Operation Edge", "boolean", "", "", true);
    mqttPublishProperty("robomow/automaticscan", "Automatic Operation Scan", "boolean", "", "", true);

    mqttPublishProperty("robomow/nextdepart", "Next Departure");
    mqttPublishProperty("robomow/automaticoperationsince", "Automatic Operation Since");

    mqttPublishProperty("robomow/childprotection", "Child Protection Enabled", "boolean", "", "", true);

    mqttPublishNode("robomowversion", "RoboMow Version", "robomowversion", "family,swversion,swrevision,hwversion");
    mqttPublishProperty("robomowversion/family", "Product Family", "enum", "", "RS,RC,RX");
    mqttPublishProperty("robomowversion/swversion", "Software Version", "integer");
    mqttPublishProperty("robomowversion/swrevision", "Software Revision", "integer");
    mqttPublishProperty("robomowversion/hwversion", "Hardware Version", "integer");

    mqttPublish("stats/mac", WiFi.macAddress());
    mqttPublish("stats/fwversion", info.firmwareMD5);
    mqttPublish("stats/starttime", getGMT());
    mqttPublish("stats/blestrength", "0");
    mqttPublish("stats/bleconnected", "false");

    mqttPublishStats();

    mqttPublishLocation();

    mqttPublish("$state", "ready");
    Serial.println(String("All published"));
}

void mqttSetup()
{
}

WiFiClient wifiClient;
WiFiClientSecure wifiClientSecure;

void mqttConnect(String const &server, int port, bool tls, String const &clientid, String const &user, String const &passwd)
{
    if (Mqtt.connected())
    {
        Serial.println("Disconnecting mqtt server");
        mqttPublish("$state", "disconnected");
        Mqtt.disconnect();
    }

    IPAddress ipaddress;
    if (ipaddress.fromString(server))
        Mqtt.setServer(ipaddress, port);
    else
        Mqtt.setServer(server.c_str(), port);

    if (tls)
        Mqtt.setClient(wifiClientSecure);
    else
        Mqtt.setClient(wifiClient);

    Serial.println(F("Connecting to mqtt server"));
    Mqtt.connect(clientid.c_str(), user.c_str(), passwd.c_str(), (String("homie/") + clientid + "/$state").c_str(), 1, true, "lost");
    Serial.println(F("Connected to mqtt server"));
    onMqttConnect(false);
}

void mqttReconnect()
{
    static int prevPort;
    static String prevServer;
    static bool prevTls;
    static String prevClientid;
    static String prevUser;
    static String prevPasswd;
    int port = Portal.getSetting(CFG_MQTTPORT).toInt();
    String server = Portal.getSetting(CFG_MQTTSERVER);
    bool tls = Portal.getSetting(CFG_MQTTTLS).toInt();
    String clientid = Portal.getSetting(CFG_MQTTCLIENTID);
    String user = Portal.getSetting(CFG_MQTTUSER);
    String passwd = Portal.getSetting(CFG_MQTTPASSWD);

    if (!Mqtt.connected() || port != prevPort || server != prevServer || tls != prevTls || clientid != prevClientid || user != prevUser || passwd != prevPasswd)
    {
        prevPort = port;
        prevServer = server;
        prevTls = tls;
        prevClientid = clientid;
        prevUser = user;
        prevPasswd = passwd;

        if (server.length() > 0 && port > 0)
            mqttConnect(server, port, tls, clientid, user, passwd);
    }
}

void mqttHandle()
{
    static uint32_t prevReconnect = -2000;
    if (millis() - prevReconnect > 2000 && WiFi.isConnected() && !Mqtt.connected())
    {
        prevReconnect = millis();
        mqttReconnect();
    }
    Mqtt.loop();
}
