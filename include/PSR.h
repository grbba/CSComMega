/**
 * @file PSR.h
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
#ifndef psr_h
#define psr_h
#include <Arduino.h>

#include <ByteStreamWriter.h>
#include <ByteStreamReader.h>
#include <PSRMessage.h>
#include <DCSIlog.h>
// #include <freeMemory.h>
#include <StreamUtils.h>

#define PSR_BUFFER_SIZE 256

template <typename T, size_t S> // T is the message object to serialize and send
class PSR
{
private:
  HardwareSerial *port = &Serial1; // serial1 by default
  size_t streamBufferSize = S;
  unsigned long baudRate = 57600;
  _tMsgCallback callback = NULL; // basically the callback will be callback(obj)
  // Create streams for reading and writing
  WriteBufferingStream _WbufferedSerial{*port, S};
  ReadBufferingStream _RbufferedSerial{*port, S};
  Writer *mw;
  Reader *mr;

public:
  PSR(HardwareSerial *p, unsigned long bRate, _tMsgCallback cb)
  {
    baudRate = bRate;
    port = p;
    callback = cb;
  };

  void send(Serializable *m)
  {                   // remember T must be a Serializable
    mw->writeRoot(m); // send the message
  }

  void setup()
  {
    INFO(F("Starting Serial\n"));
    port->begin(baudRate);
    delay(2000);
    INFO(F("Started port\n"));
    mw = new ByteStreamWriter<WriteBufferingStream>(&_WbufferedSerial, false);
    INFO(F("Created Writer\n")); // write directly to the backend
    mr = new ByteStreamReader<ReadBufferingStream, T>(&_RbufferedSerial);
    INFO(F("Created Reader\n")); // read the incomming data into the message structure
    mr->subscribe(callback);
    INFO(F("Registerd Callback\n"));
    INFO(F("Proxy setup done\n"));
  };

  void loop()
  {
    mr->loop();
  };
};

#endif