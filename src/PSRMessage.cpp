/**
 * @file PSRMessage.cpp
 * @author Gregor Baues
 * @brief Holds the main entry for reading and writing the object to be serialized
 * and defined in PSRMessage.h
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

#if defined(ESP32) || defined(ARDUINO)
#include <Arduino.h>
#endif

#include "PSRMessage.h"
#include "PSRWriter.h"
#include "DCSIlog.h"

// A DccMessage instance knows how to write/serialize itself
void DccMessage::write(Writer<DccMessage> *w)
{
        TRC(F("DccMessage::write" CR));
        payload.mlen = strlen(payload.msg);             // brutto grösse
        size_t s = (ps - MAX_PLMSG_LEN) + payload.mlen; // netto grösse
        TRC(F("payload size to write %d - msg size %d" CR), s, payload.mlen);
        unsigned char *pl = reinterpret_cast<unsigned char *>(&payload);
        TRC(F("before write payload %s" CR), payload.msg);
        w->writePayload(pl, s);
        TRC(F("DccMessage::written" CR));
};

void DccMessage::read(Reader *r)
{
        INFO(F("Deserializing Message ... \n"));
        // unsigned char *pl = reinterpret_cast<unsigned char *>(&payload);
        // r->readPayload(pl);
};

// <R 1 1 1> :: payload.mlen = 9
// s = 38 - (32-9)