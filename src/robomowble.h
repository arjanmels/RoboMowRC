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
 * Created : Saturday July 6th 2019 04:44:02 by Arjan Mels <github@mels.email>
 * Modified: Saturday July 6th 2019 04:44:02 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __ROBOMOWBLE_H_
#define __ROBOMOWBLE_H_

#include <BLERemoteCharacteristic.h>
#include "RoboMowBase.h"
#include "general.h"

class RoboMowBase;

class RoboMowConnection
{
protected:
    RoboMowBase *msgHandler;

public:
    virtual bool sendPacket(uint8_t *data, size_t length) = 0;
    void setMessageHandler(RoboMowBase *msgHandler)
    {
        this->msgHandler = msgHandler;
    }
    RoboMowBase *getMessageHandler()
    {
        return msgHandler;
    }
};

class RoboMowBLE : public RoboMowConnection
{
    static const size_t MAX_PACKET_BUFFER = 3000;

    uint8_t pb[MAX_PACKET_BUFFER];
    size_t pbstartidx;
    size_t pbendidx;

    BLEClient *pClient;
    BLERemoteCharacteristic *pCharDataOut;

protected:
    uint8_t calculateChecksum(uint8_t *data, size_t length)
    {
        uint32_t sum = 0;
        for (int i = 0; i < length - 1; i++)
            sum += data[i];

        return (uint8_t)(-1 ^ sum);
    }

    void updateChecksum(uint8_t *data, size_t length)
    {
        data[length - 1] = calculateChecksum(data, length);
    }

    bool writeValue(uint8_t *buf, size_t length)
    {
        if (!pCharDataOut)
            return false;
        else
        {
            pCharDataOut->writeValue(buf, length, true);
            return true;
        }
    }

    bool sendPacket(uint8_t *data, size_t length)
    {
        updateChecksum(data, length);
        return writeValue(data, length);
    }

    size_t parseMessage(uint8_t *data, size_t max_length);

public:
    RoboMowBLE()
    {
        pbstartidx = pbendidx = 0;
        pCharDataOut = nullptr;
    }
    void connect(BLEClient *pClient, BLERemoteCharacteristic *pCharDataOut);
    void disconnect();
    bool isConnected() { return pClient != NULL; }
    uint8_t getSignalStrength()
    {
        if (pClient == NULL)
            return 0;
        else
            return (pClient->getRssi() + 100) * 100 / 75;
    }

    void parsePacket(uint8_t *data, size_t length);
};

#endif