#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseESP32.h>

// WiFi thông tin
#define WIFI_SSID "Beckhcmut"
#define WIFI_PASSWORD "12345678"

// Firebase thông tin
#define FIREBASE_HOST "parkingsystem-9d434-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "066bluyAcyeQySyX6W4RTaUw5SkC0FZPGiUOaBr2"

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
int totalSlots = 2;
int availableSlots = totalSlots;
bool gateOpen_R = false, gateOpen_L = false;
int vehicleCount = 0;
bool barrierState = false;
int lastAvailableSlots = -1; // Trạng thái trước đó

// kiểm tra UID
String uid_R = ""; // UID bên phải
String uid_L = ""; // UID bên trái

// Thời gian cho loop
unsigned long lastMillis = 0;

// Biến lưu trữ trạng thái trước đó
bool lastGateState_R = false;
bool lastGateState_L = false;

// Thời gian kiểm tra
unsigned long previousMillis = 0;
const long interval = 2000;  // 2 giây

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

void loop() {
    // Đảm bảo kết nối Firebase và WiFi
    ensureFirebaseConnection();
    handleFirebaseStream();
    checkIRSensor();

    updateFirebase(availableSlots, gateOpen_L, gateOpen_R, uid_R, uid_L);

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

    // Kiểm tra và cập nhật trạng thái Firebase
 if (availableSlots != lastAvailableSlots || gateOpen_R != lastGateState_R || gateOpen_L != lastGateState_L) {
    updateFirebase(availableSlots, gateOpen_L, gateOpen_R, uid_R, uid_L);
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

    // Mở cổng
    openGate(servo, ledPin, gateState);

    // Cập nhật Firebase với UID
    updateFirebase(availableSlots, gateOpen_L, gateOpen_R, uid_R, uid_L);

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

void checkIRSensor() {
    int irSensorValue = digitalRead(IR_SENSOR_PIN);
    if (irSensorValue == 1) {
       Serial.println(irSensorValue);
        // delay(3000);  // Có tín hiệu từ cảm biến (xe đã đi qua)
        if (gateOpen_R) {
            closeGate(servo1, gateOpen_R);  // Đóng barrier bên phải
        }
        if (gateOpen_L) {
            closeGate(servo2, gateOpen_L);  // Đóng barrier bên trái
        }
    }
}

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

// Cập nhật trạng thái vào Firebase
void updateFirebase(int available, bool gate_L, bool gate_R, String uid_R, String uid_L) {
     FirebaseJson json;
    json.set("/availableSlots", available);
    json.set("/openGate_L", gate_L ? "true" : "false");
    json.set("/openGate_R", gate_R ? "true" : "false");
    json.set("/UID_Right", uid_R);
    json.set("/UID_Left", uid_L);

    if (Firebase.updateNode(firebaseData, path, json)) {
        Serial.println("Firebase updated successfully.");
    } else {
        Serial.println("Error updating Firebase: " + firebaseData.errorReason());
    }
}
