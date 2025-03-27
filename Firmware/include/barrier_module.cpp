#include "barrier_module.h"
#include "firebase_module.h"
#include "global_state.h"
#include "config.h"
#include <Arduino.h>

// Sử dụng LCD instance đã được khai báo trong global_state.h
// (không cần khởi tạo lại ở đây)

void openGate(Servo &servo, int led, bool &gate) {
    digitalWrite(led, HIGH);
    servo.write(90);
    gate = true;
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("OPEN GATE");
    delay(2000);
}

void closeGate(Servo &servo, bool &gate) {
    servo.write(0);
    gate = false;
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("CLOSE GATE");
}

void checkAndControlBarrier(Servo &servo1, Servo &servo2) {
    if (Firebase.getBool(firebaseData, "ParkingData/Status/Manual/StaL/barrierLeft")) {
        bool barrierLeft = firebaseData.boolData();
        if (barrierLeft == true) {
            Serial.println("Left barrier: Open");
            gateOpen_L = true;
            openGate(servo2, LED_PIN_L, gateOpen_L);
        } else {
            Serial.println("Left barrier: Close");
            gateOpen_L = false;
            closeGate(servo2, gateOpen_L);
        }
    }
    if (Firebase.getBool(firebaseData, "ParkingData/Status/Manual/StaR/barrierRight")) {
        bool barrierRight = firebaseData.boolData();
        if (barrierRight == true) {
            Serial.println("Right barrier: Open");
            gateOpen_R = true;
            openGate(servo1, LED_PIN_R, gateOpen_R);
        } else {
            Serial.println("Right barrier: Close");
            gateOpen_R = false;
            closeGate(servo1, gateOpen_R);
        }
    } else {
        Serial.println("Error reading Firebase barrier data: " + firebaseData.errorReason());
    }
}
