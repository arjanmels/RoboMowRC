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
 * Created : Monday July 8th 2019 10:53:10 by Arjan Mels <github@mels.email>
 * Modified: Monday July 8th 2019 10:53:10 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

//#include <map.h>
#include "robomowrx.h"
#include "general.h"
#include <map>

const RoboMowBase::MessageText &RoboMowRX::getMessageText(uint16_t id)
{
    if (messages.count(id) > 0)
        return messages[id];
    else
        return unknownMessageText;
}

void RoboMowRX::handleMessage(uint8_t *adata, size_t length)
{
    MSGTYPE msgId = (MSGTYPE)adata[0];
    adata++;
    length--;

    Data data(adata);

    log_i("Handling Msg; Id: %d, Length: %d, ", msgId, length);
    hexDump(adata, length);

    switch (msgId)
    {
    case MSGTYPE::MISCELLANEOUS:
    {
        if (length < 4)
        {
            log_e("Message too short for MsgId: %d, Length: %d, Required: 5", msgId, length);
            break;
        }

        //uint16_t packetId = data.int16(0);
        uint16_t miscMsgId = data.int16(2);
        switch (miscMsgId)
        {
        case MSGTYPEMISC::MISC_ROBOTSTATE:
        {
            if (length < 10)
            {
                log_e("Message too short for MsgId: %d, MiscMsgId, Length: %d, Required: 5", msgId, miscMsgId, length);
                break;
            }
            uint8_t flags1 = data.int8(4);
            main.mAutomaticOperationEdge = (flags1 & 0x02) != 0;
            main.mAutomaticOperationScan = (flags1 & 0x04) != 0;
            main.mSearchDockStation = (flags1 & 0x08) != 0;
            main.mMowMotorActive = (flags1 & 0x10) != 0;
            main.mProgramEnabled = (flags1 & 0x20) != 0;
            main.mAntiTheftEnabled = (flags1 & 0x40) != 0;

            uint8_t flags2 = data.int8(5);
            main.mSystemMode = (RoboMow::SystemMode)(flags2 & 0x07);
            main.mAntiTheftActive = (flags2 & 0x80) != 0;

            uint8_t flags3 = data.int8(6);
            main.mBatteryCapacity = flags3 & 0x7f;
            main.mAntiTheftTempDisable = (flags3 & 0x80) != 0;

            main.mMinutesTillNextDepart = data.int16(7);
            main.mMinutesAutomaticOperation = data.int16(9);

            log_i("RoboMow:MISC_ROBOTSTATE SystemMode %u; Battery %u; MowMotorActive %u, SearchDockStation %u, AutomaticOperationEdge %u, AutomaticOperationScan %u, "
                  "AntiTheftEnabled %u, AntiTheftActive %u, AntiTheftTempDisable %u, MinutesTillNextDepart %u, MinutesAutomaticOperation %u\n",
                  main.mSystemMode, main.mBatteryCapacity, main.mMowMotorActive, main.mSearchDockStation, main.mAutomaticOperationEdge, main.mAutomaticOperationScan,
                  main.mAntiTheftEnabled, main.mAntiTheftActive, main.mAntiTheftTempDisable, main.mMinutesTillNextDepart, main.mMinutesAutomaticOperation);
            break;
        }
        default:
            RoboMowBase::handleMessage(adata, length);
            break;
        }
        break;
    }
    default:
    {
        RoboMowBase::handleMessage(adata, length);
        break;
    }
    }
}
std::map<uint16_t, RoboMowRX::MessageText> RoboMowRX::messages = {
    {1, MessageText{"Passed", "Operation Passed.", true}},
    {2, MessageText{"Operation Failed.", "The Test or Calibration performed has Failed.", true}},
    {3, MessageText{"Wait", "Please wait for the process to finish…", true}},
    {4, MessageText{"No Wire Signal", "Confirm power supply is plugged into the power outlet. Check power supply and perimter wire connection to the Base Station. Check the indication on the Base Station.", true}},
    {5, MessageText{"Recharge Battery", "Low battery voltage. Recharge the battery.", true}},
    {6, MessageText{"Wheels in The Air", "Drive wheels have lost their grip with the ground or the mower is lifted.", true}},
    {7, MessageText{"Key Pressed", "One of the operating panel buttons is constantly pressed.", true}},
    {8, MessageText{"Low Temperature", "Ambiance temperature is lower than 5ºC (41ºF). The mower will resume automatically.", true}},
    {9, MessageText{"Operation time is completed", "", false}},
    {10, MessageText{"Check Mowing Height", "Switch Power Off. Mowing motor is loaded. Check grass height and insure nothing is obstructing the blade rotation.", true}},
    {11, MessageText{"Check Power", "Confirm power supply is plugged into the power outlet. Check power supply connection to the Base Station. Check charging contacts.", true}},
    {12, MessageText{"Keep Charged", "Keep your mower charged at all times, if it is not being used.", true}},
    {13, MessageText{"Base Problem", "The mower is failing to enter the Base Station. Adjust position and clean charging contacts.", true}},
    {14, MessageText{"Start Elsewhere", "Check the ground and the drive wheels. Restart elsewhere. If persists, refer to Troubleshooting section of the User Guide.", true}},
    {15, MessageText{"Cross Outside", "Some issue was found along the perimeter. Check ground, increase cutting height, or move wire inward.", true}},
    {16, MessageText{"Incorrect Connection", "Swap (reverse) the perimeter wire connection at the Base Station.", true}},
    {17, MessageText{"Start Inside", "Place the mower inside the lawn and start it again.", true}},
    {18, MessageText{"Stuck in Place", "Check ground where stuck and drive wheel rotation is not blocked. Restart elsewhere.", true}},
    {19, MessageText{"Starting Point 1 Problem", "", false}},
    {20, MessageText{"Starting Point 2 Problem", "", false}},
    {21, MessageText{"Subzone 3 Entry Problem", "", false}},
    {22, MessageText{"Lift Calibration Required", "Press GO button on the mower to start lift sensor calibration process", true}},
    {23, MessageText{"Base search is disabled", "", false}},
    {24, MessageText{"Short Operation Time", "The actual operation time was shorter than expected.", true}},
    {25, MessageText{"Waiting for Signal…", "No signal is detected and operation has stopped. Check all power cable connections. The mower will resume automatically once power is restored.", true}},
    {26, MessageText{"Calibrate Wire Sensor 1", "Turn wire signal 'Off' then press GO.", true}},
    {27, MessageText{"Calibrate Wire Sensor 2", "Turn wire signal 'On' then press GO.", true}},
    {28, MessageText{"Check Mowing Motor", "Switch Power Off. Mowing motor is loaded. Check grass height and insure nothing is obstructing the blade rotation.", true}},
    {34, MessageText{"Test Wire Position", "Walk alongside your mower while it is following the edge to test the wire position.", true}},
    {35, MessageText{"Test Edge Mode", "Place the mower into the Base Station. Press GO to start. The mower will follow Edge back to Base Station.", true}},
    {36, MessageText{"Test Near Wire Follow", "Place mower near the edge. Press OK to start. The mower will follow Edge at maximum Near Wire Follow distance.", true}},
    {37, MessageText{"Demo Mode", "Mower is in Demo Mode.", true}},
    {40, MessageText{"Place Robomow in Base Station", "Place mower near the edge. Press OK to start. The mower will follow Edge at maximum Near Wire Follow distance.", true}},
    {41, MessageText{"Alarm will soon Activate.", "Press OK to deactivate theft protection alarm.", true}},
    {42, MessageText{"Mow Motor Overheat", "The mowing motor has been overloaded for too long. The mower will resume automatically.", true}},
    {43, MessageText{"Mow Overheat", "The mowing motor(s) are overheating. Wait for cooldown. Operation will restart automatically.", true}},
    {44, MessageText{"Warning! Motors will now be activated.", "", true}},
    {45, MessageText{"Calibrate Lift Sensor 1", "Place the mower on the ground then press GO", true}},
    {46, MessageText{"Calibrate Lift Sensor 2", "Lift mower then press GO.", true}},
    {47, MessageText{"Mower is Lifted", "For safety purposes, switch the power off before lifting the mower.", true}},
    {48, MessageText{"Theft protection is active", "It's impossible to switch the mower off as long as the theft protection is active.", true}},
    {50, MessageText{"Disabling Device Removed", "Insert the Disabling Device to operate the mower", true}},
    {51, MessageText{"Standby Mode", "Your mower is currently charging in standby mode. Switch ON for operation.", true}},
    {52, MessageText{"Adjust the Wire", "The mower has bumped into something along the edge and backed up. Move the wire slightly inward. Press OK to continue.", true}},
    {60, MessageText{"Intensity Error", "The Intensity set is too high for your lawn area.", true}},
    {61, MessageText{"Decrease Inactive Time", "Too many Inactive days/hours have been set for your lawn area or Mowing Frequency (Interval) is too high.", true}},
    {62, MessageText{"Alarm will soon Activate.", "Press OK to deactivate theft protection alarm.", true}},
    {63, MessageText{"Inactive Hours 2 modified via App", "Inactive hours cannot be set via the mower as long as \"Inactive Hours 2\" are enabled via the app.", true}},
    {64, MessageText{"No Base Station Found", "\"Searching Base Station\" operation cannot be performed in a zone without a Base Station.", true}},
    {73, MessageText{"Disabling Device Removed", "Insert the Disabling Device to operate the mower", true}},
    {85, MessageText{"Invalid system configuration", "The installed Software and Hardware configurations are not compatible.", true}},
    {86, MessageText{"Waiting for Signal…", "No signal is detected and operation has stopped. Check all power cable connections. The mower will resume automatically once power is restored.", true}},
    {87, MessageText{"Mow Motor Overheat", "The mowing motor has been overloaded for too long. The mower will resume automatically.", true}},
    {88, MessageText{"Drive Motor Overheat", "The drive motor(s) has been overloaded for too long. The mower will resume automatically.", true}},
    {89, MessageText{"Child Lock is activated", "To operate your mower, please press GO + STOP together, then select the desired command.", true}},
    {90, MessageText{"MessageText", "Starting Point 1 Problem", true}},
    {91, MessageText{"MessageText", "Starting Point 2 Problem", true}},
    {92, MessageText{"No Base Station Found", "Press OK to deactivate theft protection alarm.", true}},
    {93, MessageText{"Subzone 3 Entry Problem", "", false}},
    {94, MessageText{"Subzone 4 Entry Problem", "", false}},
    {95, MessageText{"Rain Sensing…", "", false}},
    {501, MessageText{"Wire signal off reading is higher than wire signal on reading", "", true}},
    {502, MessageText{"Wire reading indicates calibration failed because either the signal off amplitude readings or the signal on amplitude readings exceed their tolerance", "", true}},
    {503, MessageText{"Wire reading indicates calibration failed because we detect that robot is not inside the garden during the calibration with signal on", "", true}},
    {504, MessageText{"Wire reading indicates calibration failed because we detect that in/out readings are invalid", "", true}},
    {505, MessageText{"Wire reading indicates calibration failed because the difference between wire max signal threshold and wire amplitude set point is too big", "", true}},
    {506, MessageText{"Wire reading indicates calibration failed because the wire no signal gain is too small", "", true}},
    {507, MessageText{"Drive Motor Disconnected", "", true}},
    {508, MessageText{"Drive configuration is invalid", "", true}},
    {509, MessageText{"Tilt calibration failed", "", true}},
    {510, MessageText{"Tilt calibration failed because accelerometer readings are not in tolerance", "", true}},
    {511, MessageText{"Accelerometer failure", "", true}},
    {512, MessageText{"Battery voltage calibration failure", "", true}},
    {513, MessageText{"Error 513", "", false}},
    {514, MessageText{"Error 514", "", false}},
    {515, MessageText{"Error 515", "", false}},
    {516, MessageText{"Error 516", "", false}},
    {517, MessageText{"Error 517", "", false}},
    {518, MessageText{"Error 518", "", false}},
    {519, MessageText{"Error 519", "", false}},
    {520, MessageText{"Error 520", "", false}},
    {521, MessageText{"Disabling Device Removed", "Insert the Disabling Device to operate the mower", true}},
    {522, MessageText{"Lift Calibration Required", "Press GO button on the mower to start lift sensor calibration process", false}},
    {525, MessageText{"Lift Calibration Required", "Press GO button on the mower to start lift sensor calibration process", false}},
    {527, MessageText{"Mowing Motor Disconnected", "Open mower's cover and check mowing motor's connection", true}},
};
