#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include "config.h"
#include "global_state.h"
#include "firebase_module.h"
#include "rfid_module.h"
#include "barrier_module.h"
#include "sensor_module.h"
#include <MFRC522.h>
#include <ESP32Servo.h>

// Khởi tạo các đối tượng phần cứng
MFRC522 mfrc522_R(SS_PIN_R, RST_PIN_R);
MFRC522 mfrc522_L(SS_PIN_L, RST_PIN_L);
// MFRC522 mfrc522_R2(SS_PIN_R, RST_PIN_R); // Nếu cần sử dụng
Servo servo1, servo2;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP address: " + WiFi.localIP());

  // Cấu hình Firebase
  firebaseSetup();

  // Khởi tạo module RFID
  mfrc522_R.PCD_Init();
  mfrc522_L.PCD_Init();

  // Gán servo
  servo1.attach(SERVO_PIN_R);
  servo2.attach(SERVO_PIN_L);
  servo1.write(0);
  servo2.write(0);

  // Khởi tạo LCD (instance đã được khởi tạo trong global_state.cpp)
  lcd.init();
  lcd.backlight();

  // Cấu hình chân cho cảm biến
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);

  Serial.println("Setup complete.");
}

void loop() {
  ensureFirebaseConnection();
  handleFirebaseStream();
  checkAndControlBarrier(servo1, servo2);
  CheckSlotEmpty();

  // Xử lý RFID cho cổng phải
  if (mfrc522_R.PICC_IsNewCardPresent() && mfrc522_R.PICC_ReadCardSerial()) {
      handleCard(mfrc522_R, servo1, LED_PIN_R, gateOpen_R, "Right");
  }
  // Xử lý RFID cho cổng trái
  if (mfrc522_L.PICC_IsNewCardPresent() && mfrc522_L.PICC_ReadCardSerial()) {
      handleCard(mfrc522_L, servo2, LED_PIN_L, gateOpen_L, "Left");
  }

  // Cập nhật Firebase nếu có thay đổi trạng thái
  if (availableSlots != lastAvailableSlots || gateOpen_R != lastGateState_R || gateOpen_L != lastGateState_L || uid_R != lastUid_R) {
      if (uid_R != lastUid_R || available1 == totalSlots) {
          currentNode++;
          lastUid_R = uid_R;
      }
      updateFirebase(currentNode, gateOpen_L, gateOpen_R, uid_R, uid_L, available1, vehicleCount, carlicenseRight, leftStatus, rightStatus, carlicenseLeft);
      lastGateState_R = gateOpen_R;
      lastGateState_L = gateOpen_L;
  }

  // Hiển thị thông tin trạng thái mỗi khoảng thời gian định sẵn
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      Serial.print("Available Slots: ");
      Serial.println(availableSlots);
      Serial.print("Gate Left Open: ");
      Serial.println(gateOpen_L ? "true" : "false");
      Serial.print("Gate Right Open: ");
      Serial.println(gateOpen_R ? "true" : "false");
  }
  
  delay(100);
}
