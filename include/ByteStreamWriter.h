
/**
 * @file ByteStreamWriter.h
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
#ifndef ByteStreamWriter_h
#define ByteStreamWriter_h

#if defined(ESP32) || defined(ARDUINO)
#include <Arduino.h>
#include <StreamUtils.h>
#else
#include <iostream>
#include <string>
typedef std::string String;
#endif

// #include "Cantor.h"
#include "PSRWriter.h"
#include "PSRMessage.h"

#define MAX_MESSAGE_BUFFER 128 // overall length of the serilaized object
#define MAX_ATTR_STRLEN 20     // TODO check for this ! max length of a string type value of the serialized object

/**
 * @brief writes values as strings to the @param buffer and then writes out everything at once
 * @todo write immediatly to the @param _ws Stream provided either at construction or set later. The buffer here$
 * carries the transport used ( HW Serial, SW Serial, WiFi, Ethernet, etc see bblanchon repo at github for more info )
 *
 */

template <typename BackEnd>
class ByteStreamWriter : public Writer
{

        char *begin = (char *)"[[";
        char *end = (char *)"]]";
        bool buffered; // write to the provided buffer before sending
                       // in case of false writes will be executed directly to the backend
        BackEnd *_ws; // Backend to write to

        char tbuffer[MAX_MESSAGE_BUFFER] = {0};
        unsigned char buffer[MAX_MESSAGE_BUFFER] = {0};
        int bpos = 0;

        virtual void writeBegin();               // start of a message block
        virtual void writeEnd();                 // end of a message block
        virtual void writePayload(unsigned char*, size_t);
        virtual void writeRoot(Serializable *);

public:
        ByteStreamWriter(){};
        ByteStreamWriter(BackEnd *stream, bool b = true)
        {
                _ws = stream;
                buffered = b;
        }
        void setStream(BackEnd *stream)
        {
                _ws = stream;
        }
        void setBehaviour(bool behaviour)
        {
                buffered = behaviour;
        }
        void printBuffer()
        {
               INFO(F("Serialized buffer %s\n"), buffer);  
        }
};

// start of a message block
template <typename BackEnd>
void ByteStreamWriter<BackEnd>::writeBegin()
{
        // INFO(F("%s"),begin);
        if (buffered)
        {
                bpos = 0; // reset buffer pos
                memset(buffer, 0, MAX_MESSAGE_BUFFER); // reset the buffer to 0
                strcat((char *) &buffer[bpos], begin);
                bpos = bpos + strlen(begin);
        }
        else
        {
#if defined(ESP32) || defined(ARDUINO)
                _ws->write(begin);
#else
                *_ws << begin;
#endif
        }
};
// end of a message block
template <typename BackEnd>
void ByteStreamWriter<BackEnd>::writeEnd()
{
        // INFO(F("%s\n"),end);
        if (buffered)
        {
                strcat((char *) &buffer[bpos], end);
                bpos = bpos + strlen(end);
#if defined(ESP32) || defined(ARDUINO)
                _ws->write((char *)buffer);
#else
                *_ws << buffer;
#endif
        }
        else
        {
#if defined(ESP32) || defined(ARDUINO)
                _ws->write(end);
#else
                *_ws << end;
#endif
        }
        _ws->flush(); // in all cases write out at the end of the serialized object
};

template <typename BackEnd>
void ByteStreamWriter<BackEnd>::writePayload( unsigned char *pl, size_t s) {
      

        // for(unsigned int k = 0; k < s; k++) { INFO(F(" %d"), *(pl+k)); }

        // INFO(F("\n"));
        if(buffered) {
                memcpy(buffer+bpos,pl,s); // write all to a buffer 
        } else {
                // int s1 = 
                _ws->write(pl,s);   // write direct 
                // INFO(F(" written: %d/%d bytes\n"), s1,s);
        }
}

template <typename BackEnd>
void ByteStreamWriter<BackEnd>::writeRoot(Serializable *obj)
{
        writeBegin();
        obj->write(this);
        writeEnd();
};

#endif