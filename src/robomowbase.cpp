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
 * Created : Wednesday July 10th 2019 02:43:28 by Arjan Mels <github@mels.email>
 * Modified: Wednesday July 10th 2019 02:43:28 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#include "robomowble.h"
#include "robomowbase.h"

RoboMowBase::MessageText RoboMowBase::unknownMessageText = {"Unknown", "Unknown message", false};

bool RoboMowBase::sendSimpleMsg(uint8_t type)
{
    log_i("Sending simple message: %d", type);
    uint8_t buf[] = {0xAA, 5, 0x1F, type, 0x00};
    return main.connection.sendPacket(buf, sizeof(buf));
}

static uint16_t counter = 0;

bool RoboMowBase::sendMiscMsg(uint16_t misctype)
{
    log_i("Sending misc message: %d", misctype);
    uint8_t buf[] = {0xAA, 9, 0x1F, 22, (uint8_t)(counter >> 8), (uint8_t)(counter & 0xff), (uint8_t)(misctype >> 8), (uint8_t)(misctype & 0xff), 0x00};
    counter++;
    return main.connection.sendPacket(buf, sizeof(buf));
}

bool RoboMowBase::sendGetEEpromParam(uint16_t param)
{
    log_i("Sending get eeprom param message: %d", param);
    uint8_t buf[] = {0xAA, 9, 0x1F, 32, (uint8_t)(param >> 8), (uint8_t)(param & 0xff), (uint8_t)(param >> 8), (uint8_t)(param & 0xff), 0x00};
    return main.connection.sendPacket(buf, sizeof(buf));
}

void RoboMowBase::connect()
{
    sendGetRobotConfig();
}

void RoboMowBase::disconnect()
{
}

void RoboMowBase::handleMessage(Message const &msg)
{
    switch (msg.getId())
    {
    case MSGTYPE::GETCONFIG:
    {
        if (!msg.checkLength(5))
            break;

        if (main.getFamily() != msg.int8(0))
            main.changeFamily(msg.int8(0));
        main.mSoftwareVersion = msg.int8(1);
        main.mSoftwareRelease = msg.int16(2);
        main.mMainboardVersion = msg.int8(4);
        log_i("RoboMow:GETCONFIG Family %u; Version %u; Revision %u, Mainboard %u", main.getFamily(), main.mSoftwareVersion, main.mSoftwareRelease, main.mMainboardVersion);
        main.updateType();
        break;
    }
    case MSGTYPE::USER:
    {
        if (!msg.checkLength(7))
            break;

        uint8_t textMsgType = msg.int8(0);
        uint16_t textMsgId = msg.int16(1);
        uint16_t systemStopId = msg.int16(3);
        uint16_t systemFailureId = msg.int16(5);
        log_i("RoboMow:USER Type: %u; TextId: %u; SystemStopId: %u, SystemFailureId: %u", textMsgType, textMsgId, systemStopId, systemFailureId);
        if (main.mTextMsgType != textMsgType || main.mTextMsgId != textMsgId || main.mSystemStopId != systemStopId || main.mSystemFailureId != systemFailureId)
        {
            bool doUpdate = textMsgType != 0 || main.mTextMsgId == 0;

            main.mTextMsgType = textMsgType;
            main.mTextMsgId = textMsgId;
            main.mSystemStopId = systemStopId;
            main.mSystemFailureId = systemFailureId;

            if (doUpdate)
            {
                main.updateMessage();
                doClearUserMessage = true;
            }
        }
        break;
    }
    default:
    {
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
        log_e("Unknown MsgId: %d, Length: %d", msg.getId(), msg.getLength());
        hexDump(msg.getData(), msg.getLength());
#endif
        break;
    }
    }
}
