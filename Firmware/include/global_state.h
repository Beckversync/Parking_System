#ifndef GLOBAL_STATE_H
#define GLOBAL_STATE_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <vector>

// Các biến trạng thái toàn cục
extern int totalSlots;
extern int availableSlots;
extern int available1;
extern bool gateOpen_R;
extern bool gateOpen_L;
extern int vehicleCount;
extern int lastAvailableSlots;
extern std::vector<String> rightUidList;
extern std::vector<String> CarlicenseRight;
extern String carlicenseRight;
extern String carlicenseLeft;
extern String uid_R;
extern String uid_L;
extern String uid_R2;
extern bool leftStatus;
extern bool rightStatus;
extern bool lastGateState_R;
extern bool lastGateState_L;
extern String lastUid_R;
extern int currentNode;

// Các biến thời gian
extern unsigned long previousMillis;
extern const long interval;

// Khai báo instance LCD dùng chung trong toàn dự án
extern LiquidCrystal_I2C lcd;

#endif
