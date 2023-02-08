/*
 *  © 2023, Gregor Baues. All rights reserved.
 *
 *  This file is part of the ESP Network Communications Framework for DCC-EX
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DCSIlog_h
#define DCSIlog_h

#include <Arduino.h>
#include <ArduinoLog.h>
#include <DCSIconfig.h>
#include "StringFormatter.h"

#if defined(__arm__)
extern "C" char *sbrk(int);
#elif defined(__AVR__)
extern char *__brkval;
extern char *__malloc_heap_start;
#elif defined(ARDUINO_ARCH_ESP32)
#else
#error Unsupported board type
#endif

#if !defined(__IMXRT1062__)
static inline int freeMemory()
{
    char top;
#if defined(__arm__)
    return &top - reinterpret_cast<char *>(sbrk(0));
#elif defined(__AVR__)
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#elif defined(ARDUINO_ARCH_ESP32)
    return  ESP.getFreeHeap();
#else
#error bailed out already above
#endif
}
#endif

/**
 * @brief encapsulation of the ArduinoLog classs to be able to add some more functionality without
 * changing the ArduinoLog library itself
 * DCSI : DCC-EX Command Station Contol Interface DCSI
 */
class DCSILog : public Logging, StringFormatter
{
private:
    Print *_logOutput;
    int ramLowWatermark = __INT_MAX__; // replaced on first loop
    char pBuffer[20];                  // print buffer
public:
    void begin(int level, Print *output, bool showLevel = true)
    {
        _logOutput = output;
        Logging::begin(level, output, showLevel);
    }

    void printLogLevel(int logLevel)
    {
        /// Show log description based on log level
        switch (logLevel)
        {
        default:
        case 0:
            _logOutput->print(" [silent ]: ");
            break;
        case 1:
            _logOutput->print(" [fatal  ]: ");
            break;
        case 2:
            _logOutput->print(" [error  ]: ");
            break;
        case 3:
            _logOutput->print(" [warning]: ");
            break;
        case 4:
            _logOutput->print(" [info   ]: ");
            break;
        case 5:
            _logOutput->print(" [trace  ]: ");
            break;
        case 6:
            _logOutput->print(" [verbose]: ");
            break;
        }
    }
    void printTimestamp()
    {

        // Division constants
        const unsigned long MSECS_PER_SEC = 1000;
        const unsigned long SECS_PER_MIN = 60;
        const unsigned long SECS_PER_HOUR = 3600;
        const unsigned long SECS_PER_DAY = 86400;

        // Total time
        const unsigned long msecs = millis();
        const unsigned long secs = msecs / MSECS_PER_SEC;

        // Time in components
        const unsigned long MilliSeconds = msecs % MSECS_PER_SEC;
        const unsigned long Seconds = secs % SECS_PER_MIN;
        const unsigned long Minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
        const unsigned long Hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

        // Time as string

        sprintf(pBuffer, "%02lu:%02lu:%02lu.%03lu ", Hours, Minutes, Seconds, MilliSeconds);
        _logOutput->print(pBuffer);
    }
    void printFreeMem()
    {
        sprintf(pBuffer, "[ram:%5db]", freeMemory());
        _logOutput->print(pBuffer);
    }

#ifdef DCCI_CS // only for the CS
     static void diag(const FSH *input...); // send operational Diagnostics from the CS to the NetworkStation
     static void flow(char t, int ec);    // send errors and warnings from the Interface code to the networkstation
#endif

    DCSILog(){};
    ~DCSILog(){};
};

//--------------------
// Set compilation mode

#define __DEV__
// #define __RELEASE__

//--------------------
// Set if file, line etc information shall be shown

#define FLNAME true
#define FREEMEM true

#define EH_DW(code) \
    do              \
    {               \
        code;       \
    } while (0) // wraps in a do while(0) so that the syntax is correct.

#define EH_IFLL(LL, code)        \
    if (dccLog.getLevel() >= LL) \
    {                            \
        code;                    \
    }
#define EH_IFFL(FL, code) \
    if (FL == true)       \
    {                     \
        code;             \
    } // File and Line number

#define EH_IFMEM(MEM, code) \
    if (MEM == true)        \
    {                       \
        code;               \
    }

// Print Memory levels

#ifdef __RELEASE__
// Starting from the principle in release mode the interface will be used and we assume only Error messages
// and maybe warnings are issued and in this case to the MWstaton for further processing/forwarding/displaying 
// on the concole/lcd etc etc ...

#define INFO(message...) ;
#define TRC(message...) ;
#define WARN(message...) ;
#define FATAL(message...) ;
#define ERR(message...) EH_DW(EH_IFLL(LOG_LEVEL_ERROR, {    dccLog.printTimestamp();                                          \
                                                            EH_IFFL(FLNAME, dccLog.error("%s:%d:%s", __FILE__, __LINE__, __FUNCTION__));     \
                                                            EH_IFMEM(FREEMEM, dccLog.printFreeMem());                         \
                                                            dccLog.printLogLevel(LOG_LEVEL_ERROR);                         \
                                                            dccLog.error(message); \
                                                        } ))
#endif

#ifdef __DEV__
// for testing purposes send WARN/ERR ids to the nwstation as CTRL messages. The ids shall be preconfigured with the 
// relevant text and used
// TODO Parameter transfer yet to come if needed.

// Id is the number of the Error/warning in the DCSIconfig.h file
#define REL_WARN(id) EH_DW(dccLog.flow('W',id))
#define REL_ERR(id) EH_DW(dccLog.flow('E',id))

// print all full message to the local serial port dpending on the set log level in the begin call
#define INFO(message...) EH_DW(EH_IFLL(LOG_LEVEL_INFO, {    dccLog.printTimestamp();                                          \
                                                            EH_IFFL(FLNAME, dccLog.info("%s:%d:%s", __FILE__, __LINE__, __FUNCTION__));     \
                                                            EH_IFMEM(FREEMEM, dccLog.printFreeMem());                         \
                                                            dccLog.printLogLevel(LOG_LEVEL_INFO);                         \
                                                            dccLog.info(message); \
                                                        } ))

#define ERR(message...) EH_DW(EH_IFLL(LOG_LEVEL_ERROR, {    dccLog.printTimestamp();                                          \
                                                            EH_IFFL(FLNAME, dccLog.error("%s:%d:%s", __FILE__, __LINE__, __FUNCTION__));     \
                                                            EH_IFMEM(FREEMEM, dccLog.printFreeMem());                         \
                                                            dccLog.printLogLevel(LOG_LEVEL_ERROR);                         \
                                                            dccLog.error(message); \
                                                        } ))

#define WARN(message...) EH_DW(EH_IFLL(LOG_LEVEL_WARNING, { dccLog.printTimestamp();                                          \
                                                            EH_IFFL(FLNAME, dccLog.warning("%s:%d:%s", __FILE__, __LINE__, __FUNCTION__));     \
                                                            EH_IFMEM(FREEMEM, dccLog.printFreeMem());                         \
                                                            dccLog.printLogLevel(LOG_LEVEL_WARNING);                         \
                                                            dccLog.warning(message); \
                                                          } ))
                                                        

#define TRC(message...) EH_DW(EH_IFLL(LOG_LEVEL_TRACE,  { dccLog.printTimestamp();              \
                                                          EH_IFFL(FLNAME,dccLog.trace("%s:%d:%s", __FILE__, __LINE__, __FUNCTION__));       \
                                                          EH_IFMEM(FREEMEM, dccLog.printFreeMem());                         \
                                                          dccLog.printLogLevel(LOG_LEVEL_TRACE); \
                                                          dccLog.trace(message); \
                                                        } ))

#define FATAL(message...) EH_DW(EH_IFLL(LOG_LEVEL_FATAL, {  dccLog.printTimestamp();                                          \
                                                            EH_IFFL(FLNAME, dccLog.fatal("%s:%d:%s", __FILE__, __LINE__, __FUNCTION__));     \
                                                            EH_IFMEM(FREEMEM, dccLog.printFreeMem());                         \
                                                            dccLog.printLogLevel(LOG_LEVEL_FATAL);                         \
                                                            dccLog.fatal(message); \
                                                        } ))
#endif

extern DCSILog dccLog;

#endif
