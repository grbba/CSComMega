/**
 * @file ByteStreamReader.h
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
#ifndef ByteStreamReader_h
#define ByteStreamReader_h

#if defined(ESP32) || defined(ARDUINO)
#include <Arduino.h>
#include <StreamUtils.h>
#else
#include <iostream>
#include <string>
#endif

#include <PSRReader.h>
#include <PSRMessage.h>

#define MAX_BUFFER 256
#define SDEL        91 // 0x5B or [
#define EDEL        93 // 0x5D or ]
// #define NOP __asm__ __volatile__("nop\n\t") 

using _tMsgCallback = void (*)(Serializable *);

/**
 * @brief writes values as strings to the @param buffer and then writes out everything at once
 * @todo write immediatly to the @param _ws Stream provided either at construction or set later. The buffer here$
 * carries the transport used ( HW Serial, SW Serial, WiFi, Ethernet, etc see bblanchon repo at github for more info )
 *
 */
template <typename BackEnd, typename T>
class ByteStreamReader : public Reader
{
private:
    BackEnd *_rs;                  // backend from which to read the data
    Serializable *obj;             // this is where the deserialized object will be accessed and stored;
    static _tMsgCallback callback; // basically the callback will be callback(obj)
    static T tObject;              // one copy of the payload which gets filled; the callback just carries
                                   // the pointer to this instance; the client needs then to copy the info

    void readPayload(unsigned char);          // fill the tObject

public:
    
    void readRoot();
    void loop();
    void subscribe(_tMsgCallback cb)
    {
        callback = cb;
    };

    ByteStreamReader(){};
    ByteStreamReader(BackEnd *stream)
    {
        _rs = stream;
    }

    // only accessible directly but not through the base class
    void setStream(BackEnd *stream)
    {
        _rs = stream;
    }
};

template <typename BackEnd, typename T>
void ByteStreamReader<BackEnd, T>::readRoot()
{
};


template <typename BackEnd, typename T>
void ByteStreamReader<BackEnd, T>::readPayload(unsigned char br)
{
    static unsigned char buf[MAX_BUFFER] = {0};
    static int pos = 0;
    static int ms, me = 0;

    switch(br)
    {
    case SDEL:
    {
        buf[pos] = br;
        if( pos == 0 ) // dont do anything we are at the start of the buffer
        {
            pos++;
            break;
        }
        if (buf[pos-1] == SDEL) // they need to follow each other which is now the case
        {
            pos++;
            ms = pos; // set the start of the buffer
            // INFO(F("\nStart: %d\n"), ms);
            break;
        }
        pos++;
        break;
    }
    case EDEL:
    {
        if (buf[pos-1] == EDEL) // found the 2nd 93
        {
            me = pos-2; // last valid byte in the message

            // INFO(F("\nEnd: %d %d\n"), me, pos);

            // for (int k = ms; k <= me; k++)
            // {
            //     INFO(F("%d "), *(buf + k));
            // }
            // INFO(F("\n"));

            memset(&tObject.payload, 0, T::ps);
            memcpy(&tObject.payload, buf + ms, me - 1);
             
            // rpLastmid = tObject.payload.mid;
            // if(tObject.payload.mid > 20058) {
            //     for (int k = ms; k <= me; k++)
            //     {
            //         INFO(F("%d_"), *(buf + k));
            //     }
            //     INFO(F("\n"));
            // }

            callback((Serializable *)&tObject);

            memset(buf, 0, MAX_BUFFER);
            pos = 0;
            ms = 0;
            me = 0;
        }
        else
        {
            buf[pos] = br;
            pos++;
        }
        break;
    }
    default:
    {
        buf[pos] = br;
        pos++;
        break;
    }
    }
}

template <typename BackEnd, typename T>
void ByteStreamReader<BackEnd, T>::loop()
{
    unsigned char br = 0; 

    while(_rs->available())
    {
        br = _rs->read();
        readPayload(br);
    }
};

template <typename BackEnd, typename T>
T ByteStreamReader<BackEnd, T>::tObject;

template <typename BackEnd, typename T>
_tMsgCallback ByteStreamReader<BackEnd, T>::callback;

#endif
