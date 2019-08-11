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

protected:
    RoboMow &main;

    size_t parseMessage(uint8_t *data, size_t max_length);

    enum MSGTYPE
    {
        GETCONFIG = 15,
        MISCELLANEOUS = 22,
        USER = 27
    };

    class Data
    {
        uint8_t *data;

    public:
        Data(uint8_t *data)
        {
            this->data = data;
        }

        uint8_t int8(size_t x)
        {
            return data[x];
        }

        uint16_t int16(size_t x)
        {
            return (data[x] << 8) | data[x + 1];
        }
    };

    bool sendSimpleMsg(uint8_t type);
    bool sendMiscMsg(uint8_t misctype);

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

    bool sendNop()
    {
        return sendSimpleMsg(MSGTYPE::USER);
    }

    virtual bool sendGetRobotState()
    {
        return false;
    }

    void handleMessage(uint8_t *adata, size_t length);
    void connect();
    void disconnect();

    static MessageText unknownMessageText;

    virtual const MessageText &getMessageText(uint16_t id)
    {
        return unknownMessageText;
    }
};

#endif