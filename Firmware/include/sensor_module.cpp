#include "sensor_module.h"
#include "config.h"
#include "global_state.h"
#include <Arduino.h>

// Sử dụng LCD instance được khai báo trong global_state.h
void CheckSlotEmpty() {
    int occupiedSlots = 0;
    float distance1 = measureDistance(TRIG_PIN_1, ECHO_PIN_1);
    float distance2 = measureDistance(TRIG_PIN_2, ECHO_PIN_2);
    if (distance1 < 50) occupiedSlots++;
    if (distance2 < 50) occupiedSlots++;
    availableSlots = totalSlots - occupiedSlots;

    if (availableSlots != lastAvailableSlots) {
        lcd.clear();
        if (availableSlots == 0) {
            lcd.setCursor(3, 1);
            lcd.print("FULL SLOT!");
        } else {
            lcd.setCursor(0, 0);
            lcd.print("Empty slots: ");
            lcd.setCursor(12, 0);
            lcd.print(availableSlots);
        }
        lastAvailableSlots = availableSlots;
    }
}

float measureDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 30000);
    return duration * 0.0344 / 2;
}
