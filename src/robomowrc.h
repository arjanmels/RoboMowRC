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
 * Created : Monday July 8th 2019 10:49:00 by Arjan Mels <github@mels.email>
 * Modified: Monday July 8th 2019 10:49:00 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#ifndef __ROBOMOWRC_H_
#define __ROBOMOWRC_H_

#include "robomowbase.h"

class RoboMowRC : public RoboMowBase
{
    enum MSGTYPEMISC
    {
        MISC_ROBOTSTATE = 4,
    };

public:
  //  virtual void handleMessage(uint8_t *data, size_t length);
    using RoboMowBase::RoboMowBase;
};

#endif
