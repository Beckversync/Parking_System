  #include <SPI.h>
  #include <MFRC522.h>
  #include <ESP32Servo.h>
  #include <LiquidCrystal_I2C.h>
  #include <FirebaseESP32.h>

  // WiFi thông tin
  #define WIFI_SSID "Beckhcmut"
  #define WIFI_PASSWORD "12345678"

  // Firebase thông tin
  #define FIREBASE_HOST "webs-943c5-default-rtdb.firebaseio.com"
  #define FIREBASE_AUTH "AyUxXwk7jGSO6rJ4nDyGwSEK7Pwiuwy9qxdeaNg0"

  // Firebase config
  FirebaseData firebaseData;
  FirebaseConfig config;
  FirebaseAuth auth;
  FirebaseJson json;
  String path = "/ParkingData";

  // RFID
  #define RST_PIN_R 4
  #define SS_PIN_R 5
  #define RST_PIN_L 16
  #define SS_PIN_L 17

  // LEDs
  #define LED_PIN_R 32
  #define LED_PIN_L 2

  // Cảm biến
  #define IR_SENSOR_PIN 33
  #define TRIG_PIN_1 12
  #define ECHO_PIN_1 14
  #define TRIG_PIN_2 27
  #define ECHO_PIN_2 26

  // Servo
  Servo servo1, servo2;
  #define SERVO_PIN_R 13
  #define SERVO_PIN_L 15

  // LCD
  LiquidCrystal_I2C lcd(0x27, 16, 2);

  // Biến trạng thái
  MFRC522 mfrc522_R(SS_PIN_R, RST_PIN_R);
  MFRC522 mfrc522_L(SS_PIN_L, RST_PIN_L);
  MFRC522 mfrc522_R2(SS_PIN_R, RST_PIN_R);
  int totalSlots = 2;
  int availableSlots = totalSlots;
  bool gateOpen_R = false, gateOpen_L = false;
  int vehicleCount = 0;
  bool barrierState = false;
  int lastAvailableSlots = -1; // Trạng thái trước đó

  // kiểm tra UID
  String uid_R = ""; // UID bên phải
  String uid_L = ""; // UID bên trái
  String uid_R2 = "";


  // Biến lưu trữ trạng thái trước đó
  bool lastGateState_R = false;
  bool lastGateState_L = false;

// *** CẤU HÌNH THỜI GIAN ***
  unsigned long previousMillis = 0;
  const long interval = 2000;  // 2 giây
  // Thời gian cho loop
  unsigned long lastMillis = 0;


// *** HÀM SETUP ***

  void setup() {
      Serial.begin(9600);
      SPI.begin();

      // WiFi
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
      }
      Serial.println("\nWiFi connected! IP address: " + WiFi.localIP());

      // Firebase
      config.host = FIREBASE_HOST;
      config.signer.tokens.legacy_token = FIREBASE_AUTH;
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);

      if (Firebase.beginStream(firebaseData, path)) {
          Serial.println("Firebase stream started successfully.");
      } else {
          Serial.println("Failed to start Firebase stream: " + firebaseData.errorReason());
      }

      // RFID
      mfrc522_R.PCD_Init();
      mfrc522_L.PCD_Init();

      // Servo và LCD
      servo1.attach(SERVO_PIN_R);
      servo2.attach(SERVO_PIN_L);
      servo1.write(0);
      servo2.write(0);
      lcd.init();
      lcd.backlight();

      // Pin cảm biến
      pinMode(TRIG_PIN_1, OUTPUT);
      pinMode(ECHO_PIN_1, INPUT);
      pinMode(TRIG_PIN_2, OUTPUT);
      pinMode(ECHO_PIN_2, INPUT);

      Serial.println("Setup hoàn tất.");
  }


// *** HÀM VÒNG LẶP CHÍNH ***
  void loop() {
      // Đảm bảo kết nối Firebase và WiFi
      ensureFirebaseConnection();
      handleFirebaseStream();
      // checkIRSensor();
        // updateFirebase(availableSlots, gateOpen_L, gateOpen_R, uid_R, uid_L,uid_R2);
      checkAndOpenLeftBarrierFromFirebase();
      // Kiểm tra khoảng cách bãi trống
      CheckSlotEmpty();

      // Kiểm tra RFID bên phải
      if (mfrc522_R.PICC_IsNewCardPresent() && mfrc522_R.PICC_ReadCardSerial()) {
          handleCard(mfrc522_R, servo1, LED_PIN_R, gateOpen_R, "Right");
      }

      // Kiểm tra RFID bên trái
      if (mfrc522_L.PICC_IsNewCardPresent() && mfrc522_L.PICC_ReadCardSerial()) {
          handleCard(mfrc522_L, servo2, LED_PIN_L, gateOpen_L, "Left");
      }
      // Kiểm tra RFID bên phải 2
      if (mfrc522_R2.PICC_IsNewCardPresent() && mfrc522_R2.PICC_ReadCardSerial()) {
           handleCard(mfrc522_R2, servo1, LED_PIN_R, gateOpen_R, "Right2");
      }

      // Kiểm tra và cập nhật trạng thái Firebase
  if (availableSlots != lastAvailableSlots || gateOpen_R != lastGateState_R || gateOpen_L != lastGateState_L) {
      updateFirebase(availableSlots, gateOpen_L, gateOpen_R, uid_R, uid_L,uid_R2);
      lastAvailableSlots = availableSlots;
      lastGateState_R = gateOpen_R;
      lastGateState_L = gateOpen_L;
  }

      // Hiển thị thông tin mỗi 2 giây
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
          // Lưu thời gian hiện tại
          previousMillis = currentMillis;

          // Hiển thị các giá trị hiện tại
          Serial.print("Available Slots: ");
          Serial.println(availableSlots);
          Serial.print("Gate Left Open: ");
          Serial.println(gateOpen_L ? "true" : "false");
          Serial.print("Gate Right Open: ");
          Serial.println(gateOpen_R ? "true" : "false");
      }

      delay(100);  // Tránh quá tải CPU
  }

  // Đảm bảo kết nối WiFi và Firebase
  void ensureFirebaseConnection() {
      if (WiFi.status() != WL_CONNECTED) {
          Serial.println("WiFi disconnected. Reconnecting...");
          WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
          while (WiFi.status() != WL_CONNECTED) {
              delay(500);
              Serial.print("...");
          }
          Serial.println("\nWiFi reconnected.");
      }
      if (!Firebase.ready()) {
          Serial.println("Firebase not ready. Reinitializing...");
          Firebase.begin(&config, &auth);
      }
  }

  // Xử lý luồng Firebase
  void handleFirebaseStream() {
      if (Firebase.readStream(firebaseData)) {
          if (firebaseData.streamTimeout()) {
              Serial.println("Stream timeout, reconnecting...");
              Firebase.beginStream(firebaseData, path);
          } else if (firebaseData.streamAvailable()) {
              Serial.println("Stream available");
              String key = firebaseData.dataPath();
              String value = firebaseData.stringData();
              // In ra chỉ những dữ liệu bạn muốn
              if (key == "/availableSlots") {
                  availableSlots = value.toInt();
                  Serial.println("Available Slots: " + String(availableSlots));
              } 
              else if (key == "/openGate_L") {
                  gateOpen_L = value == "true";
                  Serial.println("Open Gate Left: " + String(gateOpen_L));
              }
              else if (key == "/openGate_R") {
                  gateOpen_R = value == "true";
                  Serial.println("Open Gate Right: " + String(gateOpen_R));
              }
          }
      } else {
          Serial.println("Error in Firebase stream: " + firebaseData.errorReason());
      }
  }
  // Xử lý quét thẻ RFID
  void handleCard(MFRC522 &reader, Servo &servo, int ledPin, bool &gateState, const char *side) {
      String uid = "";
      for (byte i = 0; i < reader.uid.size; i++) {
          if (reader.uid.uidByte[i] < 0x10) uid += "0"; // Đảm bảo luôn có 2 chữ số
          uid += String(reader.uid.uidByte[i], HEX);
      }
      uid.toUpperCase(); // Chuyển UID sang chữ in hoa
      Serial.print("UID (");
      Serial.print(side);
      Serial.print("): ");
      Serial.println(uid);

      // Lưu UID vào biến phù hợp
      if (strcmp(side, "Right") == 0) {
          uid_R = uid;
      } else if (strcmp(side, "Left") == 0) {
          uid_L = uid;
      
      } 
       else if (strcmp(side, "Right2") == 0) {
        
        uid_R2 = uid;
}

      // Mở cổng
      openGate(servo, ledPin, gateState);

      // Cập nhật Firebase với UID
      updateFirebase(availableSlots, gateOpen_L, gateOpen_R, uid_R, uid_L,uid_R2);

          // Chờ 3 giây trước khi đóng barrier
    delay(3000);

    // Đóng barrier
    closeGate(servo, gateState);



      reader.PICC_HaltA();
  }


  // Mở và đóng cổng
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
// void checkIRSensor() {
//     // Đọc tín hiệu cảm biến hồng ngoại
//     int irSensorValue = digitalRead(IR_SENSOR_PIN);

//     // Nếu cảm biến phát hiện vật cản
//     if (irSensorValue == HIGH) { 
//         Serial.println("Phát hiện xe qua cảm biến hồng ngoại.");
//         delay(3000);  // Đợi 3 giây để xe đi qua hoàn toàn
        
//         // Đóng barrier bên phải nếu đang mở
//         if (gateOpen_R) {
//             Serial.println("Đang đóng barrier bên phải...");
//             closeGate(servo1, gateOpen_R);
//         }

//         // Đóng barrier bên trái nếu đang mở
//         if (gateOpen_L) {
//             Serial.println("Đang đóng barrier bên trái...");
//             closeGate(servo2, gateOpen_L);
//         }

//         Serial.println("Barrier đã đóng.");
//     } else {
//         // Không phát hiện tín hiệu từ cảm biến
//         Serial.println("Không có xe qua cảm biến hồng ngoại.");
//     }
// }


  // Kiểm tra bãi trống
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

  // Đo khoảng cách cảm biến siêu âm
  float measureDistance(int trigPin, int echoPin) {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      long duration = pulseIn(echoPin, HIGH, 30000);
      return duration * 0.0344 / 2;  // Tính khoảng cách (cm)
  }

void updateFirebase(int available, bool gate_L, bool gate_R, String uid_R, String uid_L, String uid_R2) {
    // Thiết lập JSON cho cổng trái
    FirebaseJson jsonLeft;
    jsonLeft.set("/UID_Left", uid_L);

    // Thiết lập JSON cho cổng phải
    FirebaseJson jsonRight;
    jsonRight.set("/UID_Right", uid_R);

    // Thiết lập JSON cho cổng phải 2
    FirebaseJson jsonRight2;

    jsonRight2.set("/UID_Right2", uid_R2); 
    Serial.println("jsonRight2 contents:");
    Serial.println(jsonRight2.raw());

    // Đường dẫn Firebase
    String pathLeft = path + "/Left";
    String pathRight = path + "/Right";
    String pathRight2 = path + "/Right2";

    // Kiểm tra và cập nhật cổng phải (Right)
    char uidRight[32] = {0};
    if (Firebase.getString(firebaseData, pathRight + "/UID_Right", uidRight) && String(uidRight) != "") {
        Serial.println("Right UID already exists: " + String(uidRight));

        // Kiểm tra Right2
        char uidRight2[32] = {0};
        if (Firebase.getString(firebaseData, pathRight2 + "/UID_Right2", uidRight2) && String(uidRight2) != "") {
            Serial.println("Right2 UID already exists: " + String(uidRight2));
            // Nếu Right2 đã có dữ liệu, không cập nhật gì cả
        } else {
            Serial.println("No Right2 UID found, updating Right2...");
            // Cập nhật Right2 nếu chưa có dữ liệu
            if (Firebase.set(firebaseData, pathRight2, jsonRight2)) {
                Serial.println("Firebase Right2 updated successfully.");
            } else {
                Serial.println("Error updating Firebase Right2: " + firebaseData.errorReason());
            }
        }
    } else {
        Serial.println("No Right UID found, updating Right1...");
        // Cập nhật Right nếu chưa có dữ liệu
        if (Firebase.set(firebaseData, pathRight, jsonRight)) {
            Serial.println("Firebase Right updated successfully.");
        } else {
            Serial.println("Error updating Firebase Right: " + firebaseData.errorReason());
        }
    }
    // Cập nhật JSON cho cổng trái
    if (Firebase.set(firebaseData, pathLeft, jsonLeft)) {
        Serial.println("Firebase Left updated successfully.");
    } else {
        Serial.println("Error updating Firebase Left: " + firebaseData.errorReason());
    }
}

void checkAndOpenLeftBarrierFromFirebase() {
    char uid_L[32] = {0}; // UID của cổng Left
    char uid_R[32] = {0}; // UID của cổng Right
    char uid_R1[32] = {0}; // UID của cổng Right1

    // Lấy UID từ Firebase cho cổng trái
    if (Firebase.getString(firebaseData, path + "/Left/UID_Left", uid_L)) {
        Serial.println("Left UID: " + String(uid_L));
    } else {
        Serial.println("Error reading Left UID: " + firebaseData.errorReason());
        return;
    }

    // Lấy UID từ Firebase cho cổng phải (Right)
    if (Firebase.getString(firebaseData, path + "/Right/UID_Right", uid_R)) {
        Serial.println("Right UID: " + String(uid_R));
    } else {
        Serial.println("Error reading Right UID: " + firebaseData.errorReason());
        return;
    }

    // Lấy UID từ Firebase cho cổng Right1 (nếu có)
    if (Firebase.getString(firebaseData, path + "/Right1/UID_Right1", uid_R1)) {
        Serial.println("Right1 UID: " + String(uid_R1));
    } else {
        Serial.println("No Right1 UID found.");
    }

    // So sánh UID giữa Left và Right
    bool matchFound = false;

    // So sánh với Right
    if (String(uid_L) == String(uid_R)) {
        matchFound = true;
        Serial.println("Match found with Right! Opening left barrier...");
        openGate(servo2, LED_PIN_L, gateOpen_L);
    }
    // So sánh với Right1 nếu có
    else if (String(uid_L) == String(uid_R1)) {
        matchFound = true;
        Serial.println("Match found with Right1! Opening left barrier...");
        openGate(servo2, LED_PIN_L, gateOpen_L);
    }

    // Nếu không có sự trùng khớp
    if (matchFound) {
        // Xóa UID và cập nhật lại Firebase
        uid_L[0] = '\0';
        uid_R[0] = '\0';
        updateFirebase(availableSlots, gateOpen_L, gateOpen_R, String(uid_R), String(uid_L), String(uid_R1));
        Serial.println("UIDs cleared. Barrier opened.");
    } else {
        Serial.println("No match found between Left and Right or Right1.");
    }
}
