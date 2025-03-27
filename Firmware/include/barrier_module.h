#ifndef BARRIER_MODULE_H
#define BARRIER_MODULE_H

#include <ESP32Servo.h>

void openGate(Servo &servo, int led, bool &gate);
void closeGate(Servo &servo, bool &gate);
void checkAndControlBarrier(Servo &servo1, Servo &servo2);

#endif
