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
 * Created : Tuesday July 2nd 2019 04:43:43 by Arjan Mels <github@mels.email>
 * Modified: Tuesday July 2nd 2019 04:43:43 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __BLE_H_
#define __BLE_H_

#include <BLEDevice.h>
#include <Arduino.h>
#include <RoboMowWebServer.h>
#include "config.h"
#include "robomowble.h"
#include "robomow.h"
#include "mqtt.h"

// The remote service we wish to connect to.
static BLEUUID SERVICE_UUID("ff00a501-d020-913c-1234-56d97200a6a6");

// The characteristics of the remote service we are interested in.
static BLEUUID CHAR_AUTHENTICATE_UUID("ff00a502-d020-913c-1234-56d97200a6a6");
static BLEUUID CHAR_DATA_OUT_UUID("ff00a503-d020-913c-1234-56d97200a6a6");
static BLEUUID CHAR_DATA_IN_UUID("ff00a506-d020-913c-1234-56d97200a6a6");

static bool doConnect = false;
static bool connected = false;
static bool doScan = true;
static BLERemoteCharacteristic *pCharAuthenticate;
static BLERemoteCharacteristic *pCharDataOut;
static BLERemoteCharacteristic *pCharDataIn;
static BLEAdvertisedDevice *myDevice;

static RoboMowBLE robomowble;
static RoboMow robomow(robomowble);

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    robomowble.parsePacket(pData, length);
}

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient)
    {
        log_i("onConnect");
    }

    void onDisconnect(BLEClient *pclient)
    {
        log_i("onDisconnect");
        connected = false;
        doScan = true;
        robomowble.disconnect();
        mqttPublish("stats/blestrength", "0");
        mqttPublish("stats/bleconnected", "false");
    }
};

bool connectToServer()
{
    log_i("BLE connecting to: %s", myDevice->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
    pClient->connect(myDevice);

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
    if (pRemoteService == nullptr)
    {
        log_e("Failed to find our service UUID: %s", SERVICE_UUID.toString().c_str());
        pClient->disconnect();
        return false;
    }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pCharAuthenticate = pRemoteService->getCharacteristic(CHAR_AUTHENTICATE_UUID);
    if (pCharAuthenticate == nullptr)
    {
        log_e("Failed to find our characteristic UUID: %s", CHAR_AUTHENTICATE_UUID.toString().c_str());
        pClient->disconnect();
        return false;
    }

    pCharDataIn = pRemoteService->getCharacteristic(CHAR_DATA_IN_UUID);
    if (pCharDataIn == nullptr)
    {
        log_e("Failed to find our characteristic UUID: %s", CHAR_DATA_IN_UUID.toString().c_str());
        pClient->disconnect();
        return false;
    }

    pCharDataOut = pRemoteService->getCharacteristic(CHAR_DATA_OUT_UUID);
    if (pCharDataOut == nullptr)
    {
        log_e("Failed to find our characteristic UUID: %s", CHAR_DATA_OUT_UUID.toString().c_str());
        pClient->disconnect();
        return false;
    }

    if (pCharAuthenticate->canWrite())
    {
        String mbserial = Portal.getSetting(CFG_MBSERIAL);
        mbserial.reserve(15);
        pCharAuthenticate->writeValue((uint8_t *)mbserial.c_str(), 15, true);
        if (false)
        {
            log_e("Failed to write to authentication characteristic");
            pClient->disconnect();
            return false;
        }
    }
    else
    {
        log_e("Not allowed to write to authentication characteristic");
        pClient->disconnect();
        return false;
    }

    if (pCharAuthenticate->canRead())
    {
        std::string value = pCharAuthenticate->readValue();
        if (value.length() != 15)
        {
            log_e("Authentication failed");
            pClient->disconnect();
            return false;
        }
        else
        {
            for (int i = 0; i < 15; i++)
            {
                if (value[i] != 0x01)
                {
                    log_e("Authentication failed");
                    pClient->disconnect();
                    return false;
                }
            }
        }
    }
    else
    {
        log_e("Not allowed to read from authentication characteristic");
        pClient->disconnect();
        return false;
    }

    if (pCharDataIn->canNotify())
        pCharDataIn->registerForNotify(notifyCallback);
    else
    {
        log_e("Failed to register for data in characteristic");
        pClient->disconnect();
        return false;
    }

    connected = true;
    robomowble.connect(pClient, pCharDataOut);
    return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        std::string name = std::string();
        if (advertisedDevice.haveName())
        {
            std::string name = advertisedDevice.getName();
        }
        else
        {
            // workaround for missing parsing of short name
            uint8_t *payload = advertisedDevice.getPayload();
            uint8_t sizeConsumed = 0;
            uint8_t ad_type;
            do
            {
                uint8_t length = *payload;  // Retrieve the length of the record.
                payload++;                  // Skip to type
                sizeConsumed += 1 + length; // increase the size consumed.

                if (length != 0)
                { // A length of 0 indicates that we have reached the end.
                    ad_type = *payload;
                    payload++;
                    length--;
                    if (ad_type == ESP_BLE_AD_TYPE_NAME_SHORT)
                        name = std::string(reinterpret_cast<char *>(payload), length);

                    payload += length;
                }
            } while (sizeConsumed < advertisedDevice.getPayloadLength());
            // end workaround
        }

        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(SERVICE_UUID))
        {
            log_d("BLE matching device found");
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = false;
        }
    }
};

void scanComplete(BLEScanResults results)
{
    log_i("scanComplete");
    doScan = true;
}

void setupBLE()
{
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    BLEDevice::init("");
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_MITM);
    BLESecurity security;
    security.setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
    security.setCapability(ESP_IO_CAP_NONE);
    security.setKeySize(16);
    security.setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    security.setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
}

uint8_t nopData[] = {0xAA, 0x05, 0x1F, 0x1B, 0x16};

void BLEHandle()
{
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnect == true)
    {
        doConnect = false;
        if (connectToServer())
        {
            Serial.println("Connected to RoboMow Device");
            /*
            log_i("A");
            RoboMowBLE &handler = (RoboMowBLE &)(robomow.getHandler());
            log_i("A: %d", handler);
            uint8_t signalstrength = handler.getSignalStrength();
            log_i("A");
            mqttPublish("stats/blestrength", String(signalstrength));
            log_i("A");
            */
            mqttPublish("stats/bleconnected", "true");
            log_i("A");
        }
        else
        {
            doScan = true;
            log_w("Failed to connect to RoboMow Device");
        }
    }

    // If we are connected to a peer BLE Server, update the characteristic each time we are reached
    // with the current time since boot.
    if (connected)
    {
        robomow.getHandler().handle();

        static uint32_t prevRSSICheck = 0;
        if (millis() - prevRSSICheck > 1000)
        {
            prevRSSICheck = millis();
            /* TODO: solve problem with getSignalStrength
            static uint8_t prevSignalStrength = 0;
            uint8_t newSignalStrength = ((RoboMowBLE &)robomow.getHandler()).getSignalStrength();
            if (newSignalStrength != prevSignalStrength)
            {
                prevSignalStrength = newSignalStrength;
                mqttPublish("stats/blestrength", String(newSignalStrength));
            }
            */
        }
    }
    else if (doScan)
    {
        log_i("BLE start scanning...");

        doScan = false;
        if (!BLEDevice::getScan()->start(30, scanComplete))
        {
            log_w("BLE scanning could not be started.");
            doScan = true;
        }
        else
        {
            log_i("BLE scanning started.");
        }
    }
}

#endif