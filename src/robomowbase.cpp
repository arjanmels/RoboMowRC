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

bool RoboMowBase::sendMiscMsg(uint8_t misctype)
{
    log_i("Sending misc message: %d", misctype);
    uint8_t buf[] = {0xAA, 5, 0x1F, 22, misctype, 0x00};
    return main.connection.sendPacket(buf, sizeof(buf));
}

void RoboMowBase::connect()
{
    for (int i = 0; i < 30; i++)
    {
        sendGetRobotConfig();
    }
}

void RoboMowBase::disconnect()
{
}

void RoboMowBase::handleMessage(uint8_t *adata, size_t length)
{
    MSGTYPE msgId = (MSGTYPE)adata[0];
    adata++;
    length--;
    Data data(adata);

    log_i("Handling Msg; Id: %d, Length: %d, ", msgId, length);
    hexDump(adata, length);

    switch (msgId)
    {
    case MSGTYPE::GETCONFIG:
    {
        if (length < 5)
        {
            log_e("Message too short for MsgId: %d, Length: %d, Required: 5", msgId, length);
            break;
        }

        if (main.getFamily() != data.int8(0))
            main.changeFamily(data.int8(0));
        main.mSoftwareVersion = data.int8(1);
        main.mSoftwareRelease = data.int16(2);
        main.mMainboardVersion = data.int8(4);
        log_i("RoboMow:GETCONFIG Family %u; Version %u; Revision %u, Mainboard %u", main.getFamily(), main.mSoftwareVersion, main.mSoftwareRelease, main.mMainboardVersion);
        main.updateType();
        break;
    }
    case MSGTYPE::USER:
    {
        if (length < 7)
        {
            log_e("Message too short for MsgId: %d, Length: %d, Required: 5", msgId, length);
            break;
        }

        uint8_t textMsgType = data.int8(0);
        uint16_t textMsgId = data.int16(1);
        uint16_t systemStopId = data.int16(3);
        uint16_t systemFailureId = data.int16(5);
        log_i("RoboMow:USER Type: %u; TextId: %u; SystemStopId: %u, SystemFailureId: %u", textMsgType, textMsgId, systemStopId, systemFailureId);
        if (main.mTextMsgType != textMsgType || main.mTextMsgId != textMsgId || main.mSystemStopId != systemStopId || main.mSystemFailureId != systemFailureId)
        {
            main.mTextMsgType = textMsgType;
            main.mTextMsgId = textMsgId;
            main.mSystemStopId = systemStopId;
            main.mSystemFailureId = systemFailureId;

            if (textMsgType != 0)
            {
                main.updateMessage();
                //  sendClearUserMessage();
            }
        }
        break;
    }
    default:
    {
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
        log_e("Unknown MsgId: %d, Length: %d", msgId, length);
        hexDump(adata, length);
#endif
        break;
    }
    }
}
