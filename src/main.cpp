// #define MSGPACKETIZER_DEBUGLOG_ENABLE
#include <MsgPacketizer.h>
#include <DccExInterface.h>
#include <DCSIlog.h>
#include <DCSIconfig.h>
#include <DIAG.h>

void foofunc(DccMessage dmsg) {
    INFO(F("Recieved:[%d:%d:%d]: %s"), dmsg.mid, dmsg.client, dmsg.p, dmsg.msg.c_str());
}

auto simpleSetup() -> void {
    Serial1.begin(115200);
    MsgPacketizer::subscribe(Serial1, DCCI.recv_index, &foofunc);
}

auto dcciSetup() -> void {
     DCCI.setup(_DCCSTA);  // setup with default values as CommandStation
     // DCCI.setup(&Serial1, 115200); // sending/recieving data to/from the Olimex
}

void setup() {
    Serial.begin(115200);
    dccLog.begin(LOG_LEVEL_VERBOSE, &Serial, false);
    delay(2000);
    // simpleSetup();
    dcciSetup();
}

long count = 5; // delay number of times to send diag messages
unsigned long d = 5000; // every 5 sec issue one 
unsigned long start = millis();
void testDiag() {

    unsigned long current = millis();
    if ( count >=0 && current - start >= d ) {
        DIAG(F("DIAG Message %d"), count);
        count--;
        REL_WARN(101);
    }
}


void loop() {
    DCCI.loop();
    testDiag();
    // use below for testing in case things seemingly are broken;
    // must be called to trigger callback and publish data
    // MsgPacketizer::update();
    // or you can call parse() and post() separately
    // MsgPacketizer::parse();  // must be called to trigger callback
    // MsgPacketizer::post(); // must be called to publish
}