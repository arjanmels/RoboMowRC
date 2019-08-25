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
 * Created : Saturday July 6th 2019 05:12:33 by Arjan Mels <github@mels.email>
 * Modified: Saturday July 6th 2019 05:12:33 by Arjan Mels <github@mels.email>
 * 
 * Revisions:
 */

#include <Arduino.h>
#include "robomowble.h"

//TODO: check for messages in quick succesion
// TODO: Check Message too short messages
size_t RoboMowBLE::parseMessage(uint8_t *data, size_t max_length)
{
    if (max_length < 2)
        return 0; // message not yet complete wait for more bluetooth packets

    if (data[0] != 0xAA)
    {
        log_e("Message must start with 0xAA, found %02X instead.", data[0]);
        return -1;
    }
    size_t msgLength = data[1];
    size_t dataidx = 2;
    if (msgLength == 0x01)
    {
        if (max_length < 4)
            return 0; // message not yet complete wait for more bluetooth packets

        msgLength = data[3] << 8 | data[2];
        dataidx += 2;
    }

    if (msgLength > max_length)
        return 0; // message not yet complete wait for more bluetooth packets

    if (msgLength <= 4)
    {
        log_e("Message too short (message: %d, packet: %d).", msgLength, max_length);
        hexDump(data, max_length);
        return -1;
    }

    uint8_t calculatedChecksum = calculateChecksum(data, msgLength);
    uint8_t checksum = data[msgLength - 1];

    if (checksum != calculatedChecksum)
    {
        log_e("Checksum mismatch; received: %02X, calculated: %02X", checksum, calculatedChecksum);
        return -1;
    }

    uint8_t msgType = data[dataidx++];

    if (msgType != 0x1E)
    {
        log_e("Wrong message type (%02X).", msgType);
        return -1;
    }

    msgHandler->handleMessage(RoboMowBase::Message(data + dataidx, msgLength - dataidx - 1));

    return msgLength;
}

void RoboMowBLE::parsePacket(uint8_t *data, size_t length)
{
    if (length == 0)
    {
        log_i("Packet has zero length");
        return;
    }

    if (length > MAX_PACKET_BUFFER)
    {
        log_e("Packet longer (%d) then buffer (%d)", length, MAX_PACKET_BUFFER);
        return;
    }

    if (length + pbendidx > MAX_PACKET_BUFFER)
    {
        log_e("Message longer then buffer (%d) then buffer (%d)", length, MAX_PACKET_BUFFER);
        pbstartidx = pbendidx = 0;
    }

    if (pbendidx == 0) // nothing in buffer yet, no need to copy memory
    {
        size_t msgLength = parseMessage(data, length);
        if (msgLength == -1 || msgLength == length) // invalid packet drop current buffer content or completely handled
        {
            pbstartidx = pbendidx = 0;
            return;
        }
        else
        {
            data += msgLength;
            length -= msgLength;
        }
    }

    memcpy(pb + pbendidx, data, length);
    pbendidx += length;

    while (pbendidx > pbstartidx)
    {

        size_t msgLength = parseMessage(pb + pbstartidx, pbendidx - pbstartidx);
        if (msgLength == -1 || msgLength == pbendidx - pbstartidx) // invalid packet drop current buffer content or completely handled
            pbstartidx = pbendidx = 0;
        else if (msgLength == 0) // wait for more data
            break;
        else
            pbstartidx += msgLength;
    }
}

void RoboMowBLE::connect(BLEClient *pClient, BLERemoteCharacteristic *pCharDataOut)
{
    this->pClient = pClient;
    this->pCharDataOut = pCharDataOut;
    msgHandler->connect();
}

void RoboMowBLE::disconnect()
{
    this->pClient = NULL;
    this->pCharDataOut = NULL;
    msgHandler->disconnect();
}
