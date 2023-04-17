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

void DccMessage::write(Writer *w)
{
        payload.mlen = strlen(payload.msg);             // brutto grösse
        size_t s = ps - (MAX_PLMSG_LEN - payload.mlen); // netto grösse
        unsigned char *pl = reinterpret_cast<unsigned char *>(&payload);
        // INFO(F("before write payload "));
        w->writePayload(pl, s);
        // INFO(F("after write payload "));
};

void DccMessage::read(Reader *r)
{
        INFO(F("Deserializing Message ... \n"));
        // unsigned char *pl = reinterpret_cast<unsigned char *>(&payload);
        // r->readPayload(pl);
};
