// This sketch demonstrates the IrReceiverSampler together with Pronto::toHexString.
// It requires a demodulating sensor connected to pin RECEIVE_PIN.
// For this reason, we "guess" the modulation frequency to be
// IrSignal::defaultFrequency.

#include <Arduino.h>
#include <IrReceiverSampler.h>
#include <Pronto.h>

#define RECEIVE_PIN 5U
#define BUFFERSIZE 200U
#define BAUD 115200

IrReceiver *receiver;

void setup() {
    Serial.begin(BAUD);
    receiver = IrReceiverSampler::newIrReceiverSampler(BUFFERSIZE, RECEIVE_PIN);
    receiver->setEndingTimeout(100);
}

void loop() {
    receiver->receive(); // combines enable, loop, disable

    if (receiver->isEmpty())
        Serial.println(F("timeout"));
    else {
        //receiver->dump(Serial);
        IrSequence* irSequence = receiver->toIrSequence();
        const char* hex = Pronto::toProntoHex(*irSequence);
        Serial.println(hex);
        delete[] hex;
        delete irSequence;
    }
}
