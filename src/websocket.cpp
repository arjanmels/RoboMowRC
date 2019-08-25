
#include <Arduino.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{

    switch (type)
    {
    case WStype_DISCONNECTED:
        log_i("[%u] Disconnected!", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        log_i("[%u] Connected from %d.%d.%d.%d url: %s", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "C:Connected");
        webSocket.broadcastTXT("C:Client Connected");
    }
    break;
    case WStype_TEXT:
        log_i("[%u] get Text: %s", num, payload);

        // send message to client
        // webSocket.sendTXT(num, "message here");

        // send data to all connected clients
        // webSocket.broadcastTXT("message here");
        break;
    case WStype_BIN:
        log_i("[%u] get binary length: %u", num, length);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
        break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    case WStype_PING:
    default:
        break;
    }
}

void websocketSetup()
{
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void websocketHandle()
{
    static unsigned long prevBeat = 0;
    if (millis() - prevBeat > 1000)
    {
        prevBeat = millis();
        webSocket.broadcastTXT(String("U:heartbeat:") + prevBeat);
        webSocket.broadcastTXT(String("U:freeheap:") + ESP.getFreeHeap());
        webSocket.broadcastTXT(String("U:freepsram:") + ESP.getFreePsram());
        webSocket.broadcastTXT(String("U:freestack:") + uxTaskGetStackHighWaterMark(NULL));
    }

    webSocket.loop();
}