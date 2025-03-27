#include "rfid_module.h"
#include "firebase_module.h"
#include "barrier_module.h"
#include "global_state.h"
#include <Arduino.h>

// Xử lý thẻ RFID khi được quét
void handleCard(MFRC522 &reader, Servo &servo, int ledPin, bool &gateState, const char *side) {
    String uid = "";
    for (byte i = 0; i < reader.uid.size; i++) {
        if (reader.uid.uidByte[i] < 0x10)
            uid += "0";
        uid += String(reader.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    Serial.print("UID (");
    Serial.print(side);
    Serial.print("): ");
    Serial.println(uid);

    int nodeToUse = findEmptyNode();
    String carLicense = "";

    if (strcmp(side, "Right") == 0) {
        if (nodeToUse == -1) {
            currentNode++;
            nodeToUse = currentNode;
        }
        if (uid != lastUid_R || available1 == totalSlots) {
            uid_R = uid;
            lastUid_R = uid;
            rightUidList.push_back(uid);
            if (vehicleCount < totalSlots && available1 > 0) {
                rightStatus = true;
                updateFirebase(nodeToUse, gateOpen_L, gateOpen_R, uid_R, uid_L, available1, vehicleCount, carLicense, leftStatus, rightStatus, carlicenseLeft);
                delay(1000);
                rightStatus = false;
                updateFirebase(nodeToUse, gateOpen_L, gateOpen_R, uid_R, uid_L, available1, vehicleCount, carLicense, leftStatus, rightStatus, carlicenseLeft);
                delay(10000);
                String nodePath = "ParkingData/" + String(nodeToUse) + "/CarlicenseRight";
                String carLicenseRight = "";
                if (Firebase.getString(firebaseData, nodePath)) {
                    carLicenseRight = firebaseData.stringData();
                    Serial.println("CarlicenseRight fetched.");
                } else {
                    Serial.println("Error fetching CarlicenseRight: " + firebaseData.errorReason());
                }
                
                if (carLicenseRight != "") {
                    gateOpen_R = true;
                    openGate(servo, ledPin, gateState);
                    
                    if (gateOpen_R == true) {
                        vehicleCount++;
                        available1--;
                        Serial.println("Available slots decreased. Current count: " + String(available1));
                        updateFirebase(nodeToUse, gateOpen_L, gateOpen_R, uid_R, uid_L, available1, vehicleCount, carLicense, leftStatus, rightStatus, carlicenseLeft);
                        delay(3000);
                    }
                } else {
                    Serial.println("Not opening gate because CarlicenseRight is empty.");
                    rightStatus = false;
                }
            }
            // Sau khi xử lý, đóng barrier
            closeGate(servo, gateState);
            gateOpen_R = false;
            updateFirebase(nodeToUse, gateOpen_L, gateOpen_R, uid_R, uid_L, available1, vehicleCount, carLicense, leftStatus, rightStatus, carlicenseLeft);
        } else {
            Serial.println("UID Right is duplicate, not processed.");
        }
    } 
    else if (strcmp(side, "Left") == 0) {
        leftStatus = true;
        uid_L = uid;
        Serial.println("Card scanned at Left gate.");
        
        for (size_t i = 0; i < rightUidList.size(); i++) {
            if (rightUidList[i] == uid) {
                nodeToUse = i + 1;
                updateFirebase(nodeToUse, gateOpen_L, gateOpen_R, uid_R, uid_L, available1, vehicleCount, carLicense, leftStatus, rightStatus, carlicenseLeft);
                delay(1000);
                leftStatus = false;
                updateFirebase(nodeToUse, gateOpen_L, gateOpen_R, uid_R, uid_L, available1, vehicleCount, carLicense, leftStatus, rightStatus, carlicenseLeft);
                delay(10000);
                String pathNode = "ParkingData/" + String(nodeToUse) + "/CarlicenseLeft";
                String carLicenseLeft = "";
                if (Firebase.getString(firebaseData, pathNode)) {
                    carLicenseLeft = firebaseData.stringData();
                    Serial.println("CarlicenseLeft at node " + String(nodeToUse) + ": " + carLicenseLeft);
                } else {
                    Serial.println("Error fetching CarlicenseLeft: " + firebaseData.errorReason());
                    continue;
                }

                if (carLicenseLeft != "") {
                    if (vehicleCount > 0 && available1 <= totalSlots) {
                        available1++;
                        vehicleCount--;
                    }

                    gateOpen_L = true;
                    updateFirebase(nodeToUse, gateOpen_L, gateOpen_R, rightUidList[i], uid_L, available1, vehicleCount, carlicenseRight, leftStatus, rightStatus, carLicenseLeft);
                    Serial.println("Conditions met, opening Left gate.");
                    openGate(servo, ledPin, gateState);

                    rightUidList[i] = "DELETED";
                    uid_L = "DELETED";
                    delay(7000);
                    gateOpen_L = false;
                    updateFirebase(nodeToUse, gateOpen_L, gateOpen_R, rightUidList[i], uid_L, available1, vehicleCount, carlicenseRight, leftStatus, rightStatus, carLicenseLeft);
                    closeGate(servo, gateState);
                    delay(2000);
                    deleteNodeFromFirebase(nodeToUse);
                    break;
                } else {
                    Serial.println("Cannot open Left gate because CarlicenseLeft is empty.");
                }
            }
        }
    }

    reader.PICC_HaltA();
}

// Hàm tìm node trống trong Firebase
int findEmptyNode() {
    for (int i = 1; i <= currentNode; i++) {
        String pathNode = "ParkingData/" + String(i);
        if (Firebase.get(firebaseData, pathNode + "/UID_Right") && firebaseData.stringData() == "") {
            return i;
        } else if (Firebase.get(firebaseData, pathNode + "/UID_Right") && firebaseData.stringData() == "DELETED") {
            continue;
        }
    }
    return -1;
}
