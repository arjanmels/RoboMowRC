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
 * Created : Monday July 8th 2019 10:49:10 by Arjan Mels <github@mels.email>
 * Modified: Monday July 8th 2019 10:49:10 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __ROBOMOWRX_H_
#define __ROBOMOWRX_H_

#include "robomowbase.h"
#include <map>

class RoboMowRX : public RoboMowBase
{
    static std::map<uint16_t, MessageText> messages;

    enum MSGTYPEMISC : uint16_t
    {
        MISC_ROBOTSTATE = 11,
        MISC_CLEARUSERMESSAGE = 14,
    };

    enum EEPROMPARAM : uint16_t
    {
        PROGRAMENABLED = 140,
        CHILDLOCK = 188,
    };

public:
    using RoboMowBase::RoboMowBase;
    virtual void handleMessage(Message const &msg);
    virtual const MessageText &getMessageText(uint16_t id);

    virtual bool sendGetRobotState() { return sendMiscMsg(MISC_ROBOTSTATE); }
    virtual bool sendClearUserMessage() { return sendMiscMsg(MISC_CLEARUSERMESSAGE); }
    virtual bool sendGetProgramEnabledState() { return sendGetEEpromParam(PROGRAMENABLED); }
    virtual bool sendGetChildLockState() { return sendGetEEpromParam(CHILDLOCK); }
};

#endif
