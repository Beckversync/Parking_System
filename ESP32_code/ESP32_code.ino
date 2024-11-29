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

void updateFirebase(int available, bool gate_L, bool gate_R, String uid_R, String uid_L) {
    FirebaseJson jsonLeft;
    FirebaseJson jsonRight;

    // Thiết lập JSON cho cổng trái
    jsonLeft.set("/availableSlots", available);
    jsonLeft.set("/openGate_L", gate_L ? "true" : "false");
    jsonLeft.set("/UID_Left", uid_L);

    // Thiết lập JSON cho cổng phải
    jsonRight.set("/availableSlots", available);
    jsonRight.set("/openGate_R", gate_R ? "true" : "false");
    jsonRight.set("/UID_Right", uid_R);

    // Đường dẫn Firebase
    String pathLeft = path + "/Left";
    String pathRight = path + "/Right";

    // Kiểm tra xem node RIGHT_1 đã có đủ thông tin chưa
    char uidRight[32] = {0};  // Dùng char[] thay vì String
    if (Firebase.getString(firebaseData, pathRight + "/UID_Right", uidRight)) {
        Serial.println("Right UID already exists: " + String(uidRight));

        // Nếu RIGHT_1 đã có thông tin, tạo node RIGHT_2
        String pathRight2 = path + "/Right2";
        FirebaseJson jsonRight2;
        jsonRight2.set("/availableSlots", available);
        jsonRight2.set("/openGate_R", gate_R ? "true" : "false");
        jsonRight2.set("/UID_Right", uid_R);  // UID mới cho RIGHT_2

        // Cập nhật node RIGHT_2
        if (Firebase.updateNode(firebaseData, pathRight2, jsonRight2)) {
            Serial.println("Firebase Right2 updated successfully.");
        } else {
            Serial.println("Error updating Firebase Right2: " + firebaseData.errorReason());
        }

    } else {
        Serial.println("No Right UID found, updating Right1...");

        // Nếu không có thông tin, cập nhật RIGHT_1
        if (Firebase.updateNode(firebaseData, pathRight, jsonRight)) {
            Serial.println("Firebase Right updated successfully.");
        } else {
            Serial.println("Error updating Firebase Right: " + firebaseData.errorReason());
        }
    }

    // Cập nhật JSON cho cổng trái
    if (Firebase.updateNode(firebaseData, pathLeft, jsonLeft)) {
        Serial.println("Firebase Left updated successfully.");
    } else {
        Serial.println("Error updating Firebase Left: " + firebaseData.errorReason());
    }
}


void checkAndOpenLeftBarrierFromFirebase() {
    char platenumber_L[32] = {0}; 
    char platenumber_R[32] = {0};
    char platenumber_R1[32] = {0};

    // Lấy biển số từ Firebase cho cổng trái
    if (Firebase.getString(firebaseData, path + "/Left/platenumber", platenumber_L)) {
        Serial.println("Left Plate Number: " + String(platenumber_L));
    } else {
        Serial.println("Error reading Left Plate Number: " + firebaseData.errorReason());
        return;
    }

    // Lấy biển số từ Firebase cho cổng phải (Right)
    if (Firebase.getString(firebaseData, path + "/Right/platenumber", platenumber_R)) {
        Serial.println("Right Plate Number: " + String(platenumber_R));
    } else {
        Serial.println("Error reading Right Plate Number: " + firebaseData.errorReason());
        return;
    }

    // Lấy biển số từ Firebase cho cổng Right_1 (nếu có)
    if (Firebase.getString(firebaseData, path + "/Right1/platenumber", platenumber_R1)) {
        Serial.println("Right1 Plate Number: " + String(platenumber_R1));
    } else {
        Serial.println("No Right1 Plate Number found.");
    }

    // So sánh chuỗi giữa UID và biển số
    bool matchFound = false;

    // So sánh với Right
    if (uid_L == uid_R && strcmp(platenumber_L, platenumber_R) == 0) {
        matchFound = true;
        Serial.println("Match found with Right! Opening left barrier...");
        openGate(servo2, LED_PIN_L, gateOpen_L);
    }
    // So sánh với Right_1 nếu có
    else if (strcmp(platenumber_L, platenumber_R1) == 0) {
        matchFound = true;
        Serial.println("Match found with Right1! Opening left barrier...");
        openGate(servo2, LED_PIN_L, gateOpen_L);
    }

    // Nếu không có sự trùng khớp
    if (matchFound) {
        // Xóa UID và cập nhật lại Firebase
        uid_L = "";
        uid_R = "";
        updateFirebase(availableSlots, gateOpen_L, gateOpen_R, uid_R, uid_L);
        Serial.println("UIDs cleared. Barrier opened.");
    } else {
        Serial.println("No match found between Left and Right or Right1.");
    }
}




