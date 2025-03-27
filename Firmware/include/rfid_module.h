#ifndef RFID_MODULE_H
#define RFID_MODULE_H

#include <MFRC522.h>
#include <ESP32Servo.h>

void handleCard(MFRC522 &reader, Servo &servo, int ledPin, bool &gateState, const char *side);
int findEmptyNode();

#endif
