#ifndef CONFIG_H
#define CONFIG_H

#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseESP32.h>
#include <WiFi.h>

// WiFi config
#define WIFI_SSID "Beckhcmut"
#define WIFI_PASSWORD "12345678"

// Firebase config
#define FIREBASE_HOST "parkingsystem-9d434-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "066bluyAcyeQySyX6W4RTaUw5SkC0FZPGiUOaBr2"

// RFID Pins
#define RST_PIN_R 4
#define SS_PIN_R 5
#define RST_PIN_L 16
#define SS_PIN_L 17

// LED Pins
#define LED_PIN_R 32
#define LED_PIN_L 2

// Sensor Pins
#define IR_SENSOR_PIN 33
#define TRIG_PIN_1 12
#define ECHO_PIN_1 14
#define TRIG_PIN_2 27
#define ECHO_PIN_2 26

// Servo Pins
#define SERVO_PIN_R 13
#define SERVO_PIN_L 15

// LCD configuration
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#endif
