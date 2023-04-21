/**
 * @file DccExInterface.cpp
 * @author Gregor Baues
 * @brief Implementation of a communication protocol between two MCU type devices.
 * Developped for the purpose of offloading Ethernet, Wifi etc communictions
 * which are ressource consuming from one of the devices esp UNO, Megas etc.
 * This has been developped for the DCC-EX commandstation running eiher on an Uno or Mega.
 * Although serial, Ethernet and Wifi code exists all the CS functionality leaves
 * little to no space for the communication part.
 * By offloading this into a separate MCU also other evolutions could be imagined.
 * the communication to avoid unnecessary overhead concentrates on UART Serial first as this
 * is build in I2C and/or SPI could be imagined at a future state
 * @version 0.1
 * @date 2023-01-13
 *
 * @copyright Copyright (c) 2022, 2023
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
 */

#include <Arduino.h>
#include <DCSIconfig.h>
#include <DCSIlog.h>
#ifndef DCCI_CS
#include "NetworkInterface.h"
#include "Transport.h"
DCCNetwork *network = NetworkInterface::getDCCNetwork();
#endif
#include "DccExInterface.h"
#include "DCSICommand.h"

#include <PSR.h>
#include <PSRMessage.h>

// void recvMessage(Serializable *msgptr)
// {
//     DccMessage *dm = reinterpret_cast<DccMessage *>(msgptr);
//     INFO(F("[%d] Recieved: "), freeMemory());
//     dm->print();
// }

void foofunc2(Serializable *msg);
PSR<DccMessage, PSR_BUFFER_SIZE> proxy(&Serial1, 57600, &foofunc2); // create a proxy for DccMessages over Serial1 at 115200 baud and a 128 byte r/w buffer size

/**
 * @brief callback function upon reception of a DccMessage. Adds the message into the incomming queue
 * Queue elements will be processed then in the recieve() function called form the loop(). This function is on both sides
 * of the com link i.e. on the commandstation and the networkstation. It will not be used for messages comming from
 * Wifi or the ethernet connection( or serial )
 * @param msg DccMessage object deliverd by MsgPacketizer
 */
void foofunc2(Serializable *msg)
// void foofunc2(DccMessage msg)
{
    DccMessage *dm = reinterpret_cast<DccMessage *>(msg);

    // test if queue isn't full
    if (!DccExInterface::getMsgQueue()->isFull())
    {
        // allocate a DccMessage from the pool and copy the contents recieved
        DccMessage *m = DccExInterface::getMsg();
        *m = *dm; // DccMessage::copy(m, &msg);
        m->process = DccExInterface::recieve;

        TRC(F("Callback recieved from [%s]:[%d:%d:%d:%d]: %s" CR),
            DccExInterface::decode(static_cast<comStation>(m->payload.sta)),
            DccExInterface::getMsgQueue()->size(),
            m->payload.mid, m->payload.client, m->payload.p, m->payload.msg);

        // push the message into the message queue
        DccExInterface::getMsgQueue()->push(m);
    }
    else
    {
        ERR(F("Message queue is full; Message has not been processed" CR));
    }
}
/**
 * @brief           init the serial com port with the command/network station as well as the
 *                  queues if needed
 *
 * @param _s        HardwareSerial port ( this depends on the wiring between the two boards
 *                  which Hw serial are hooked up together)
 * @param _speed    Baud rate at which to communicate with the command/network station
 */
auto DccExInterface::_isetup(HardwareSerial *_s, uint32_t _speed) -> void
{

    INFO(F("Setting up DccEx Network interface connection ..." CR));

    s = _s;         // Serial port used for com depends on the wiring == ignored
    speed = _speed; // speed of the connection == ignored as only needed in the PSR
    // s->begin(speed); // start the serial port at the given baud rate == done in the PSR

    proxy.setup();

    // MsgPacketizer::subscribe(*s, recv_index, &foofunc2);

    init = true; // interface has been initatlized
    INFO(F("Setup of %s done ..." CR), DccExInterface::decode(static_cast<comStation>(sta)));
}
/**
 * @brief process all that is in the incomming queue and reply
 *
 */
auto DccExInterface::_iRecieve(DccMessage *m) -> void
{
    TRC(F("Recieved from [%s]:[%d:%d:%d:%d]: %s" CR), DccExInterface::decode(comStation(m->payload.sta)), DccExInterface::getMsgQueue()->size(), m->payload.mid, m->payload.client, m->payload.p, m->payload.msg);
    // if recieved from self then we have an issue
    if (m->payload.sta == sta)
    {
        ERR(F("Wrong sender; Msg seems to have been send to self; Msg has been ignored" CR));
        return;
    }
    INFO(F("Sending to handler" CR));
    MEMC(handlers[m->payload.p](*m););
    return;
}

/**
 * @brief creates a DccMessage and adds it to the outgoing queue
 *
 * @param c  client from which the message was orginally recieved
 * @param p  protocol for the CS DCC(JMRI), WITHROTTLE etc ..
 * @param msg the messsage ( outgoing i.e. going to the CS i.e. will mostly be functional payloads plus diagnostics )
 */
void DccExInterface::_iqueue(uint16_t c, csProtocol p, char *msg)
{

    // MsgPack::str_t s = MsgPack::str_t(msg);
    DccMessage *m = DccExInterface::getMsg(); // get a message from the pool

    if (m == nullptr)
    {
        ERR(F("No more messages available in the pool" CR));
        return;
    } // TODO test for null messgage i.e. no more elemens available in the pool

    m->payload.sta = static_cast<uint8_t>(sta);
    m->payload.client = c;
    m->payload.p = static_cast<uint8_t>(p);
    strcpy(m->payload.msg, msg); //  = MsgPack::str_t(msg);
    m->payload.mid = seq++;
    m->process = DccExInterface::write;

    m->print();

    INFO(F("Queuing#1" CR)); //  [%d:%d:%s]:[%s]" CR), m->payload.mid, m->payload.client, decode((csProtocol)m->payload.p), m->payload.msg);

    msgQueue.push(m);

    return;
}
void DccExInterface::_iqueue(DccMessage *m)
{

    INFO(F("Queuing#2 [%d:%d:%s]:[%s]" CR), m->payload.mid, m->payload.client, decode((csProtocol)m->payload.p), m->payload.msg);
    msgQueue.push(m);
    return;
}

/**
 * @brief write pending messages in the outgoing queue to the serial connection
 *
 */
void DccExInterface::_iWrite(DccMessage *m)
{
    TRC(F("Sending [%d:%d:%d]: %s" CR), m->payload.mid, m->payload.client, m->payload.p, m->payload.msg);
    proxy.send(m);
    // MsgPacketizer::send(*s, 0x34, *m);
    return;
}

void DccExInterface::_iLoop()
{
    if (!DccExInterface::getMsgQueue()->isEmpty())
    {
        DccMessage *m = DccExInterface::getMsgQueue()->pop();
        TRC(F("Processing :[%d:%d:%d:%d]: %s" CR), DccExInterface::getMsgQueue()->size(), m->payload.mid, m->payload.client, m->payload.p, m->payload.msg);
        m->process(m); // that will call either recieve() or write()
        DccExInterface::releaseMsg(m);
    }
    proxy.loop(); // MsgPacketizer::update(); // send back replies and get commands/trigger the callback
};

auto DccExInterface::_iDecode(csProtocol p) -> const char *
{
    // empty the buffer

    memset(decodeBuffer, 0, sizeof(decodeBuffer));
    // need to check if p is a valid enum value
    if ((p >= UNKNOWN_CS_PROTOCOL) || (p < 0))
    {
        ERR(F("Cannot decode csProtocol %d returning unkown"), p);
#ifdef ESP32
        strcpy(decodeBuffer, csProtocolNames[UNKNOWN_CS_PROTOCOL]);
#else
        strcpy_P(decodeBuffer, (char *)pgm_read_word(&(csProtocolNames[UNKNOWN_CS_PROTOCOL])));
#endif
        return decodeBuffer;
    }
#ifdef ESP32
    strcpy(decodeBuffer, csProtocolNames[p]);
#else
    strcpy_P(decodeBuffer, (char *)pgm_read_word(&(csProtocolNames[p])));
#endif
    return decodeBuffer;
}
auto DccExInterface::_iDecode(comStation s) -> const char *
{
    // empty the buffer
    memset(decodeBuffer, 0, sizeof(decodeBuffer));
    // need to check if p is a valid enum value
    decodeBuffer[0] = '\0';
    if ((s >= _UNKNOWN_STA) || (s < 0))
    {
        ERR(F("Cannot decode comStation %d returning unkown"), s);
#ifdef ESP32
        strcpy(decodeBuffer, comStationNames[_UNKNOWN_STA]);
#else
        strcpy_P(decodeBuffer, (char *)pgm_read_word(&(comStationNames[_UNKNOWN_STA])));
#endif
        return decodeBuffer;
    }
    // TRC(F("Decoding comStation:[%d]" CR), (int) s);
#ifdef ESP32
    strcpy(decodeBuffer, comStationNames[s]);
#else
    strcpy_P(decodeBuffer, (char *)pgm_read_word(&(comStationNames[s])));
#endif

    return decodeBuffer;
}

auto DccExInterface::dccexHandler(DccMessage &m) -> void
{
    INFO(F("Processing message from [%s]:[%s]" CR), DccExInterface::decode(static_cast<comStation>(m.payload.sta)), m.payload.msg);

    // send to the DCC part the commands and get the reply
    // TODO we need to check the length of the response comming back from the CS and ev send
    // TODO send as multiple packets ... ( function which doesn't exist yet function which could be used to balance between space & time)

    char buffer[MAX_MESSAGE_SIZE] = {0};

    sprintf(buffer, "reply from CS: %d:%d:%s", m.payload.client, m.payload.mid, m.payload.msg);
    DccExInterface::queue(m.payload.client, _REPLY, buffer); //
};

auto DccExInterface::wiThrottleHandler(DccMessage &m) -> void{};
auto DccExInterface::ctrlHandler(DccMessage &m) -> void
{
    // where does the message come from
    INFO(F("Recieved CTRL message from %s" CR), DccExInterface::decode(static_cast<comStation>(m.payload.sta)));
    switch (m.payload.sta)
    {
    case _DCCSTA:
    {
        // we are on the NW station handling a message from the commandstation
        break;
    }
    case _NWSTA:
    {
        TRC(F("Executing CTRL message %s" CR), m.payload.msg);
        Cmds.run(m.payload.msg);
        // we are on the CommandStation handling a message from the NetworkStation
        // so the message shall have the form <! opcode yy zz >
        // get the opcode and get the function to handle it from the opcode hashmap
        // opcodes is until the first whitespace in the command afetr the !
        // timming all leading spaces away

        break;
    }
    }
};
auto DccExInterface::notYetHandler(DccMessage &m) -> void
{
    if (m.payload.p == UNKNOWN_CS_PROTOCOL)
    {
        ERR(F("Unkown Message protocol; Message ignored" CR));
    }
    else
    {
        WARN(F("%s Message protocol not supported on %s; Message ignored" CR), DccExInterface::decode((csProtocol)m.payload.p), DccExInterface::decode((comStation)m.payload.sta));
    }
    return;
};
#ifndef DCCI_CS // only valid on the NW station
auto DccExInterface::replyHandler(DccMessage &m) -> void
{

    INFO(F("Processing reply from the CommandStation for client [%d]..." CR), m.payload.client);

    // search for the client in the network ... There must be a better way
    // and send the reply now to the connected client ...

    byte nt = network->getNumberOfTransports();              // #of networkinterfaces which have been instantiated
    transportType *tt = network->getArrayOfTransportTypes(); // for each of the interfaces we know the transport type

    for (byte i = 0; i < nt; i++)
    {
        switch (tt[i])
        {
        case WIFI:
        {
            WiFiTransport *wt = static_cast<WiFiTransport *>(network->transports[i]);
            if (wt->getActive() == 0)
                break; // nothing to be done no clients
            WiFiClient wtc = wt->getClient(m.payload.client);
            if (wtc.connected())
            {
                wtc.write(m.payload.msg);
                wtc.write(CR); // CR -> just so that we have a nl in the terminal ...
            }
            else
            {
                WARN(F("WiFi client not connected. Can't send reply" CR));
            }
            break;
        }
        case ETHERNET:
        {
            EthernetTransport *et = static_cast<EthernetTransport *>(network->transports[i]);
            if (et->getActive() == 0)
                break; // nothing to be done no clients
            EthernetClient etc = et->getClient(m.payload.client);
            if (etc.connected())
            {
                etc.write(m.payload.msg);
                etc.write(CR);
            }
            else
            {
                WARN(F("Ethernet client not connected. Can't send reply" CR));
            }
            break;
        }
        default:
        {
            ERR(F("Unknown transport protocol must be either WIFI or ETHERNET"));
            break;
        }
        }
    }
}
auto DccExInterface::diagHandler(DccMessage &m) -> void
{
    INFO(F("Recieved DIAG: %s" CR), m.payload.msg);
};
#endif

auto DccExInterface::_iSize() -> size_t
{
    return msgQueue.size();
}

DccExInterface &DccExInterface::GetInstance()
{
    static DccExInterface dcciInterface;
    return dcciInterface;
}
