/*
 * FaMe IT ECG Filter for Cardiac Booster
 * 
 * Copyright (c) 2019 FaMe IT
 * 
 * Created : Thursday May 16th 2019 09:24:23 by Arjan Mels <admin@fame-it.net>
 * Modified: Thursday May 16th 2019 09:24:23 by Arjan Mels <admin@fame-it.net>
 * 
 * Revisions:
 * 28-05-2019	AM	Initial version
 */


#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <Arduino.h>

template <class T, size_t SIZE, size_t FRACTION>
class SamplingBuffer
{
private:
    T raw[SIZE] = {0};
    size_t _index = 0;

    int wrap(int a) const
    {
        int r = a % SIZE;
        return r < 0 ? r + SIZE : r;
    }

public:
    void add(T value)
    {
        raw[_index / FRACTION] = value;
        _index++;
        if (_index / FRACTION >= SIZE)
            _index = 0;
    }
    bool isStep() const
    {
        return _index % FRACTION == 0;
    }

    T &operator[](size_t index)
    {
        return raw[wrap(index)];
    }

    T operator[](size_t index) const
    {
        return raw[wrap(index)];
    }
    size_t size() const
    {
        return SIZE;
    }
    size_t index() const
    {
        return _index;
    }
};

template <class T, int SIZE>
class CircularBuffer
{
private:
    T raw[SIZE] = {0};
    int index = -1;
    int wrap(int a) const
    {
        int r = a % SIZE;
        return r < 0 ? r + SIZE : r;
    }

public:
    CircularBuffer() {}

    void next()
    {
        index++;
        if (index >= SIZE)
            index = 0;
    }

    void add(T value)
    {
        next();
        raw[index] = value;
    }

    T &operator[](int index)
    {
        return raw[wrap(this->index + index)];
    }

    T operator[](int index) const
    {
        return raw[wrap(this->index + index)];
    }

    size_t size() const
    {
        return SIZE;
    }

    T average() const
    {
        T res = 0;
        for (int i = 0; i < SIZE; i++)
            res += raw[i];
        return res / SIZE;
    }

    T max() const
    {
        T res = raw[0];
        for (int i = 1; i < SIZE; i++)
            if (raw[i] > res)
                res = raw[i];
        return res;
    }
};

#endif