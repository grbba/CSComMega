/**
 * @file PSRWriter.h
 * @author Gregor Baues (grbaues@gmail.com)
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

#ifndef PSRWriter_h
#define PSRWriter_h

#if defined(ESP32) || defined(ARDUINO)
#include <Arduino.h>
#else
#include <iostream>
#include <string>
#include <cstdint>
typedef std::string String;
#endif

class Serializable;

template <typename M>
class Writer
{
public:
    virtual void writeBegin() = 0; // start of a serialzed object
    virtual void writeEnd() = 0;   // end of a serialzed object
    virtual void writePayload(unsigned char *, size_t) = 0;
    virtual void writeRoot(M *) = 0; // write out the entire message M
};

#endif