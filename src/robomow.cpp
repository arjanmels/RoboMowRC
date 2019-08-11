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
 * Created : Wednesday July 10th 2019 02:40:17 by Arjan Mels <github@mels.email>
 * Modified: Wednesday July 10th 2019 02:40:17 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#include "robomow.h"
#include "robomowble.h"
#include "robomowbase.h"
#include "robomowrs.h"
#include "robomowrc.h"
#include "robomowrx.h"
#include "mqtt.h"

void RoboMow::updateType()
{
    mqttPublish("robomowversion/type", mFamily == RS ? "RS" : mFamily == RC ? "RC" : mFamily == RX ? "RX" : "Unknown");
    mqttPublish("robomowversion/swversion", String(mSoftwareVersion));
    mqttPublish("robomowversion/swrevision", String(mSoftwareRelease));
    mqttPublish("robomowversion/hwversion", String(mMainboardVersion));
}

void RoboMow::updateMessage()
{
    uint32_t textMessageId = mTextMsgId;
    if (mTextMsgType == 2 || mTextMsgType == 3 || mTextMsgType == 6 || mTextMsgType == 7)
        textMessageId = mSystemStopId;
    const RoboMowBase::MessageText &msgText = connection.getMessageHandler()->getMessageText(textMessageId);
    mMsgText = msgText.name;
    mMsgDescription = msgText.description;

    mqttPublish("robomow/status", mMsgText);
    mqttPublish("robomow/statusdescription", mMsgDescription);
}

void RoboMow::updateState()
{
    mqttPublish("robomow/mode", mSystemMode == Idle ? "Idle" : mSystemMode == Charging ? "Charging" : mSystemMode == Automatic ? "Automatic Operation" : mSystemMode == RemoteControlled ? "RemoteControlled" : mSystemMode == Bit ? "Bit" : "Unknown");
    mqttPublish("robomow/status", mMsgText);
    mqttPublish("robomow/statusdescription", mMsgDescription);
    mqttPublish("robomow/battery", String(mBatteryCapacity));
    mqttPublish("robomow/theftprotection", String(mAntiTheftEnabled));
    mqttPublish("robomow/theftprotectionactive", String(mAntiTheftActive));
    mqttPublish("robomow/theftprotectiontempdisable", String(mAntiTheftTempDisable));

    mqttPublish("robomow/mowmotoractive", String(mMowMotorActive));
    mqttPublish("robomow/searchdockstation", String(mSearchDockStation));
    mqttPublish("robomow/automaticedge", String(mAutomaticOperationEdge));
    mqttPublish("robomow/automaticscan", String(mAutomaticOperationScan));

    mqttPublish("robomow/nextdepart", getGMT(mMinutesTillNextDepart * 60));
    mqttPublish("robomow/automaticoperationsince", getGMT(-60 * mMinutesAutomaticOperation));

    //    mqttPublish("robomow/childprotection", String(mBatteryCapacity));
}

RoboMow::RoboMow(RoboMowConnection &connection) : connection(connection)
{
    handler = new RoboMowBase(*this);
    connection.setMessageHandler(handler);
}

bool RoboMow::changeFamily(uint8_t family)
{
    if (family == 0 || family > 3)
    {
        log_e("Invalid RoboMow family: %d", family);
        return false;
    }

    if (mFamily == family)
        return true;

    log_i("Changing from family %d to %d", mFamily, family);
    mFamily = (Family)family;
    delete handler;

    switch (mFamily)
    {
    case RS:
        handler = new RoboMowRS(*this);
        break;
    case RC:
        handler = new RoboMowRC(*this);
        break;
    case RX:
        handler = new RoboMowRX(*this);
        break;
    default:
        return false;
    }
    connection.setMessageHandler(handler);
    return true;
}