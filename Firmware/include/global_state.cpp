#include "global_state.h"

// Khởi tạo các biến trạng thái
int totalSlots = 4;
int availableSlots = totalSlots;
int available1 = totalSlots;
bool gateOpen_R = false;
bool gateOpen_L = false;
int vehicleCount = 0;
int lastAvailableSlots = -1;
std::vector<String> rightUidList;
std::vector<String> CarlicenseRight;
String carlicenseRight = "";
String carlicenseLeft = "";
String uid_R = "";
String uid_L = "";
String uid_R2 = "";
bool leftStatus = false;
bool rightStatus = false;
bool lastGateState_R = false;
bool lastGateState_L = false;
String lastUid_R = "";
int currentNode = 0;

// Các biến thời gian
unsigned long previousMillis = 0;
const long interval = 2000;

// Khởi tạo LCD (instance dùng chung – khởi tạo trong main.cpp cũng được tham chiếu)
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
