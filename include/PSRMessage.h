/**
 * @file PSRMessage.h
 * @author Gregor Baues
 * @brief  TODO
 * @version 1.0
 * @date 2023-02-09
 *
 * @copyright Copyright (c) 2023
 *
 * This is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * See the GNU General Public License for more details <https://www.gnu.org/licenses/>
 *
 */
#ifndef psr_message_h
#define psr_message_h

#if defined(ESP32) || defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstdint>
#include <string>
typedef std::string String;
#include <memory.h>
#endif

#include "PSRWriter.h"
#include "PSRReader.h"
#include "DCSIlog.h"

/**
 * @brief Abstract class; The message to send needs to implement the write & read functions
 *
 */
// class Serializable
// {
// public:
//     virtual void read(Reader *) = 0;
//     virtual void write(Writer *) = 0;
//     // static Serializable *newByName(char *); // needed for objects within objects
// };

// max no of charsin a text based attribute
#define MAX_PLMSG_LEN 32

class DccMessage // : public Serializable
{
private:
    struct _sPayload
    {
        uint8_t sta = 0;              // station allowed values are comming from the comStation enum
                                      // but as msgpack doesn't really work on enums(?)
        uint8_t client = 0;           // client id ( socket number from Wifi or Ethernet to be checked if we need to also have the original channel)
        uint8_t p = 0;                // either JMRI or WITHROTTLE in order to understand the content of the msg payload
        uint8_t mlen = MAX_PLMSG_LEN; // used to not copy the complete length i.e. the write functions for this will
                                      // just write the strleng of msg and set the number of bytes writtent here
        uint16_t mid = 0;             // message id; sequence number
        char msg[MAX_PLMSG_LEN];      // contains the main payload to be send as a string
    };

public:
    // function pointer to either the write function(sending the message to the otherside over the com link) or the recieve function(i.e.processing the recieved message)
    using _tcsProcessHandler = void (*)(DccMessage *);
    _tcsProcessHandler process;

    static const size_t ps = sizeof(_sPayload);
    _sPayload payload;

    void read(Reader *r);
    void write(Writer<DccMessage> *w);

    // Assignment Operator overload
    // DccMessage Dest = DccMessage Src
    // creates a copy
    void operator=(const DccMessage &src)
    {
        payload.sta = src.payload.sta;
        payload.mid = src.payload.mid;
        payload.client = src.payload.client;
        payload.p = src.payload.p;
        memcpy(payload.msg, src.payload.msg, src.payload.mlen);
    }

    // static void copy(DccMessage *dest, DccMessage *src)
    // {
    //     dest->payload.sta = src->payload.sta;
    //     dest->payload.mid = src->payload.mid;
    //     dest->payload.client = src->payload.client;
    //     dest->payload.p = src->payload.p;
    //     memcpy(dest->payload.msg, src->payload.msg, src->payload.mlen);
    // }

    void print()
    {
        char pbuf[128];
        sprintf(pbuf, "MessageId:%d Station:%d Client:%d Protocol: %d Payload: %s\n", (unsigned int)payload.mid, (unsigned char)payload.sta, (unsigned char)payload.client, (unsigned char)payload.p, payload.msg);
        Serial.print(pbuf);
        // Serial.println(payload.mid);
        // INFO(F("DccMessage: MessageId:%d Station:%d Client:%d Protocol: %d Payload: %s\n"), payload.mid, payload.sta, payload.client, payload.p, payload.msg);
    }

    DccMessage() {}
    ~DccMessage() {}
};

#endif