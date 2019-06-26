#ifndef _MQTT_H_
#define _MQTT_H_

#include <Arduino.h>
#include <AsyncMqttClient.h>

AsyncMqttClient Mqtt;

void MqttPublish(String topic, String value)
{
    Mqtt.publish((String("homie/") + Portal.getSetting("mqttclientid") + "/" + topic).c_str(), 1, true, value.c_str(), value.length());
}

void mqttPublishStats()
{
    MqttPublish("$stats/uptime", String(millis() / 1000));
    MqttPublish("$stats/signal", String(WiFi.RSSI() + 100));
}

void mqttPublishLocation()
{
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10))
    {
        char buff[20];
        strftime(buff, 20, "%c", &timeinfo);

        MqttPublish("location/starttime", buff);
    }
    MqttPublish("location/timezone", info.timezone);
    MqttPublish("location/source", "ip");
    MqttPublish("location/latitude", info.latitude);
    MqttPublish("location/longitude", info.longitude);
}

void MqttNode(String node, String property, String name, String datatype = "string", String unit = "", String format = "")
{

    MqttPublish(node + "/" + property + "/$name", name);
    if (datatype.length() > 0)
        MqttPublish(node + "/" + property + "/$type", datatype);
    if (unit.length() > 0)
        MqttPublish(node + "/" + property + "/$unit", unit);
    if (format.length() > 0)
        MqttPublish(node + "/" + property + "/$format", format);
}

void onMqttConnect(bool sessionPresent)
{
    Serial.println(String("Mqtt server connected; session present: ") + sessionPresent);

    MqttPublish("$homie", "4.0");
    MqttPublish("$name", "RoboMow RC");
    MqttPublish("$state", "init");
    MqttPublish("$implementation", "RoboMowRC");

    /*
  MqttPublish("$localip", WiFi.localIP().toString());
  MqttPublish("$mac", WiFi.macAddress());
  MqttPublish("$fw/name", "RoboMowRC-firmware");
  MqttPublish("$fw/version", info.firmwareMD5);
  MqttPublish("$stats", "uptime,signal");
  MqttPublish("$stats/interval", "60");
*/

    MqttPublish("$extensions", "");
    MqttPublish("$nodes", "location");

    MqttPublish("location/$name", "Location");
    MqttPublish("location/$type", "Location information");
    MqttPublish("location/$properties", "starttime,timezone,source,latitude,longitude");

    MqttNode("location", "starttime", "Connected since");
    MqttNode("location", "timezone", "Timezone");
    MqttNode("location", "source", "Location source", "enum", "", "none,ip,lorawan,gps");
    MqttNode("location", "latitude", "Latitude", "float", "°", "-90:90");
    MqttNode("location", "longitude", "Longitude", "float", "°", "-180:180");

    mqttPublishStats();
    mqttPublishLocation();
    MqttPublish("$state", "ready");
    Serial.println(String("All published"));
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println(String("Mqtt server disconnected; reason: ") + (int8_t)reason);
}

void mqttSetup()
{
    Mqtt.onConnect(onMqttConnect);
    Mqtt.onDisconnect(onMqttDisconnect);
}

void mqttConnect(String const &server, int port, bool tls, String const &clientid, String const &user, String const &passwd)
{
    if (Mqtt.connected())
    {
        Serial.println("Disconnecting mqtt server");
        MqttPublish("$state", "disconnected");
        Mqtt.disconnect();
    }

    IPAddress ipaddress;
    if (ipaddress.fromString(server))
        Mqtt.setServer(ipaddress, port);
    else
        Mqtt.setServer(server.c_str(), port);

   // Mqtt.setSecure(tls);

    Mqtt.setClientId(clientid.c_str());
    Mqtt.setCredentials(user.c_str(), passwd.c_str());
    Mqtt.setWill((String("homie/") + clientid + "/$state").c_str(), 1, true, "lost");

    Serial.println("Connecting to mqtt server");
    Mqtt.connect();
}

#endif