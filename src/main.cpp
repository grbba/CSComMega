// #define MSGPACKETIZER_DEBUGLOG_ENABLE

#include <Arduino.h>
#include <MsgPacketizer.h>
#include <DccExInterface.h>
#include <DCSIlog.h>
#include <DCSIconfig.h>
#include <DIAG.h>

auto dcciSetup() -> void
{
    dccLog.begin(LOG_LEVEL_VERBOSE, &Serial, false);
    DccExInterface::setup(_DCCSTA); // setup with default values as CommandStation
}

void setup()
{
    Serial.begin(115200);
    delay(2000);
    MEMC(dcciSetup());
}

void loop()
{
    DccExInterface::loop();
}