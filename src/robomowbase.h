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
 * Created : Tuesday July 9th 2019 07:17:35 by Arjan Mels <github@mels.email>
 * Modified: Tuesday July 9th 2019 07:17:35 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __ROBOMOWBASE_H_
#define __ROBOMOWBASE_H_

#include "RoboMow.h"
#include <map>

class RoboMowBase
{
public:
    struct MessageText
    {
        const char *name;
        const char *description;
        boolean popup;
    };

    enum MSGTYPE
    {
        GETCONFIG = 15,
        MISCELLANEOUS = 22,
        USER = 27,
        READEEPROM = 32
    };

    /*
Package:
0xAA <datalength> <messagetype> <messageid> <payload>… <checksum>

Multiple packets in one notification. Notification always starts with new packet, unless very long?

0xAA <datalength> <messagetype> <messageid> <packetId LSB> <packetId MSB> <payload>… <checksum>
Or:
0xAA 0x01 <datalen LSB>  <datalen MSB> <messagetype> <messageid> <packetId LSB> <packetId MSB> <payload>… <checksum>

e.g.:
0xAA 0x05 <messagetype> <messageid> <checksum>

Messagetype always 0x1e

*/
    class Message
    {
        uint8_t *data;
        uint16_t length;

    public:
        Message(uint8_t *data, uint16_t length)
        {
            this->data = data;
            this->length = length;
        }

        MSGTYPE getId() const
        {
            return (MSGTYPE)data[0];
        }

        uint8_t *getData() const
        {
            return data;
        }
        uint16_t getLength() const
        {
            return length;
        }

        bool checkLength(uint16_t expectedLength) const
        {
            if (length - 1 != expectedLength)
            {
                log_e("Message too short (received %d != expected %d) for MsgId: %d", length - 1, expectedLength, data[0]);
                return false;
            }
            else
            {
                return true;
            }
        }

        bool checkMinimumLength(uint16_t expectedLength) const
        {
            if (length - 1 < expectedLength)
            {
                log_e("Message too short (received %d < expected %d) for MsgId: %d", length - 1, expectedLength, data[0]);
                return false;
            }
            else
            {
                return true;
            }
        }

        uint8_t int8(size_t x) const
        {
            return data[x + 1];
        }

        uint16_t int16(size_t x) const
        {
            return (data[x + 1] << 8) | data[x + 2];
        }

        uint32_t int32(size_t x) const
        {
            return (data[x + 1] << 24) | (data[x + 2] << 16) | (data[x + 3] << 8) | data[x + 4];
        }
    };

protected:
    RoboMow &main;
    bool nopEnabled = true;
    uint32_t prevNop = 0;

    size_t parseMessage(uint8_t *data, size_t max_length);

    bool sendSimpleMsg(uint8_t type);
    bool sendMiscMsg(uint16_t misctype);
    bool sendGetEEpromParam(uint16_t param);
    bool doClearUserMessage = false;

public:
    RoboMowBase(RoboMow &main) : main(main)
    {
    }
    virtual ~RoboMowBase()
    {
    }

    bool sendGetRobotConfig()
    {
        return sendSimpleMsg(MSGTYPE::GETCONFIG);
    }

    bool sendGetUserMessage()
    {
        return sendSimpleMsg(MSGTYPE::USER);
    }

    virtual bool sendGetRobotState() { return false; }

    virtual bool sendClearUserMessage() { return false; }

    virtual bool sendGetProgramEnabledState() { return false; }
    virtual bool sendGetChildLockState() { return false; }

    virtual void handle()
    {
        if (nopEnabled)
        {
            if (millis() - prevNop > 2000)
            {
                prevNop = millis();
                if (main.getFamily() == RoboMow::Unknown)
                    sendGetRobotConfig();
                else
                {
                    sendGetUserMessage();
                    sendGetRobotState();
                    sendGetProgramEnabledState();
                    sendGetChildLockState();
                }
            }
        }
        if (doClearUserMessage)
        {
            sendClearUserMessage();
            doClearUserMessage = false;
        }
    }

    virtual void
    handleMessage(Message const &msg);
    void connect();
    void disconnect();

    static MessageText unknownMessageText;

    virtual const MessageText &getMessageText(uint16_t id)
    {
        return unknownMessageText;
    }
};

#endif