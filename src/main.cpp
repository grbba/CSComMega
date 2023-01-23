// #define MSGPACKETIZER_DEBUGLOG_ENABLE
#include <MsgPacketizer.h>
#include <DccExInterface.h>
#include <DIAG.h>

void foofunc(DccMessage dmsg) {
    INFO(F("Recieved:[%d:%d:%d]: %s"), dmsg.mid, dmsg.client, dmsg.p, dmsg.msg.c_str());
}

auto simpleSetup() -> void {
    Serial1.begin(115200);
    MsgPacketizer::subscribe(Serial1, DCCI.recv_index, &foofunc);
}

auto dcciSetup() -> void {
     DCCI.setup(&Serial1, 115200); // sending/recieving data to/from the Olimex
}

void setup() {
    Serial.begin(115200);
    Log.begin(LOG_LEVEL_VERBOSE, &Serial, false);
    delay(2000);

    // simpleSetup();
    dcciSetup();
}

void loop() {
    // must be called to trigger callback and publish data
    // MsgPacketizer::update();
    DCCI.loop();

    // or you can call parse() and post() separately
    // MsgPacketizer::parse();  // must be called to trigger callback
    // MsgPacketizer::post(); // must be called to publish
}