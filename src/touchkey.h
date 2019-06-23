/*
 * FaMe IT ECG Filter for Cardiac Booster
 * 
 * Copyright (c) 2019 FaMe IT
 * 
 * Created : Saturday May 25th 2019 09:25:10 by Arjan Mels <admin@fame-it.net>
 * Modified: Saturday May 25th 2019 09:25:10 by Arjan Mels <admin@fame-it.net>
 * 
 * Revisions:
 * 28-05-2019	AM	Initial Version
 */

#ifndef _TOUCHKEY_H_
#define _TOUCHKEY_H_

#include <Arduino.h>
#include "filter.h"

class TouchKey
{
private:
    int pin;
    FilterAvgAndLocalVariance filter;
    FilterAvgAndLocalVariance filterShort;
    FilterMedian filterMedian;

    uint32_t startTouch = 0;
    uint32_t endTouch = 0;
    uint32_t lastStep = 0;

public:
    bool print = false;
    TouchKey(int pin) : filter(0.1, touchRead(pin)), filterShort(10, touchRead(pin)), filterMedian(touchRead(pin))
    {
        this->pin = pin;
        this->print = print;
        lastStep = millis();
    }

    bool isTouching()
    {
        return filterShort.getAverage() < filter.getAverage() - 3 * sqrt(filter.getVariance());
    }

    void step()
    {
        bool prevTouching = isTouching();

        double value = filterMedian.step(touchRead(pin));

        uint32_t now = millis();
        filter.step(value, 0.001 * (now - lastStep));
        filterShort.step(value, 0.001 * (now - lastStep));
        lastStep = now;

        bool touching = isTouching();
        if (prevTouching != touching)
        {
            if (touching)
            {
                startTouch = now;
                endTouch = 0;
            }
            else
            {
                endTouch = now;
            }
        }
    }

    uint32_t touchDuration()
    {
        if (endTouch != 0 || startTouch == 0)
            return 0;
        else
            return millis() - startTouch;
    }

    uint32_t prevTouchDuration()
    {
        if (endTouch == 0)
            return 0;
        else
            return endTouch - startTouch;
    }

    void resetPrevTouchDuration()
    {
        endTouch = 0;
    }

    bool checkLastTouchDuration(uint32_t min, uint32_t max)
    {
        if (endTouch == 0)
        {
            return false;
        }
        else
        {
            uint32_t duration = endTouch - startTouch;
            endTouch = 0;
            return duration >= min && duration <= max;
        }
    }
};

#endif