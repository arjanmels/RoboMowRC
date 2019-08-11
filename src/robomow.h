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
 * Created : Tuesday July 9th 2019 07:24:01 by Arjan Mels <github@mels.email>
 * Modified: Tuesday July 9th 2019 07:24:01 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __ROBOMOW_H_
#define __ROBOMOW_H_

#include <Arduino.h>

#undef ARDUHAL_LOG_LEVEL
#define ARDUHAL_LOG_LEVEL ARDUHAL_LOG_LEVEL_INFO

class RoboMowConnection;
class RoboMowBase;
class RoboMowRx;
class RoboMowRc;
class RoboMowRs;

class RoboMow
{
public:
    enum Family : uint8_t
    {
        RS = 1,
        RC = 2,
        RX = 3,
    };

    enum SystemMode : uint8_t
    {
        Idle = 1,
        Charging = 2,
        Automatic = 3,
        RemoteControlled = 4,
        Bit = 5
    };

    uint8_t mSoftwareVersion;
    uint16_t mSoftwareRelease;
    uint8_t mMainboardVersion;

    void updateType();

    uint8_t mTextMsgType;
    uint16_t mTextMsgId;
    uint16_t mSystemStopId;
    uint16_t mSystemFailureId;

    const char *mMsgText;
    const char *mMsgDescription;

    void updateMessage();

    bool mAutomaticOperationEdge;
    bool mAutomaticOperationScan;
    bool mSearchDockStation;
    bool mMowMotorActive;
    bool mProgramEnabled;
    bool mAntiTheftEnabled;
    bool mAntiTheftTempDisable;
    bool mAntiTheftActive;

    SystemMode mSystemMode;
    uint8_t mBatteryCapacity;
    uint16_t mMinutesTillNextDepart;
    uint16_t mMinutesAutomaticOperation;

    void updateState();

private:
    Family mFamily;

protected:
    RoboMowConnection &connection;
    RoboMowBase *handler;

public:
    RoboMow(RoboMowConnection &connection);
    bool changeFamily(uint8_t family);
    Family getFamily() { return mFamily; }
    RoboMowBase &getHandler() { return *handler; }

    friend RoboMowBase;
};

#endif