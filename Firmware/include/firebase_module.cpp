#include "firebase_module.h"
#include "global_state.h"
#include <Arduino.h>

// Định nghĩa các đối tượng Firebase
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

String path = "/ParkingData";

void firebaseSetup() {
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (Firebase.beginStream(firebaseData, path)) {
      Serial.println("Firebase stream started successfully.");
  } else {
      Serial.println("Failed to start Firebase stream: " + firebaseData.errorReason());
  }
}

void ensureFirebaseConnection() {
  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Reconnecting...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
      }
      Serial.println("\nWiFi reconnected.");
  }
  if (!Firebase.ready()) {
      Serial.println("Firebase not ready. Reinitializing...");
      Firebase.begin(&config, &auth);
  }
}

void handleFirebaseStream() {
    if (Firebase.readStream(firebaseData)) {
        if (firebaseData.streamTimeout()) {
            Serial.println("Stream timeout, reconnecting...");
            Firebase.beginStream(firebaseData, path);
        } else if (firebaseData.streamAvailable()) {
            Serial.println("Stream available");
            String key = firebaseData.dataPath();
            String value = firebaseData.stringData();
            if (key == "/availableSlots") {
                availableSlots = value.toInt();
                Serial.println("Available Slots: " + String(availableSlots));
            } 
            else if (key == "/openGate_L") {
                gateOpen_L = (value == "true");
                Serial.println("Open Gate Left: " + String(gateOpen_L));
            }
            else if (key == "/openGate_R") {
                gateOpen_R = (value == "true");
                Serial.println("Open Gate Right: " + String(gateOpen_R));
            }
        }
    } else {
        Serial.println("Error in Firebase stream: " + firebaseData.errorReason());
    }
}

void updateFirebase(int nodeNumber, bool gate_L, bool gate_R, String uid_R, String uid_L, int available, int count, String carLicenseRight, bool leStatus, bool riStatus, String carLicenseLeft) {
    FirebaseJson jsonNode;
    jsonNode.set("/UID_Right", uid_R);
    jsonNode.set("/UID_Left", uid_L);
    jsonNode.set("/CarlicenseRight", carLicenseRight);
    jsonNode.set("/CarlicenseLeft", carLicenseLeft);

    String pathNode = "ParkingData/" + String(nodeNumber);

    if (Firebase.set(firebaseData, pathNode, jsonNode)) {
        Serial.println("Firebase node updated successfully at " + pathNode);
    } else {
        Serial.println("Error updating Firebase node: " + firebaseData.errorReason());
    }

    FirebaseJson jsonStatus;
    jsonStatus.set("/barrierLeft", gate_L);
    jsonStatus.set("/barrierRight", gate_R);
    jsonStatus.set("/leftStatus", leStatus);
    jsonStatus.set("/rightStatus", riStatus);

    String pathStatus = "ParkingData/Status";

    if (Firebase.set(firebaseData, pathStatus, jsonStatus)) {
        Serial.println("Firebase status updated successfully at " + pathStatus);
    } else {
        Serial.println("Error updating Firebase status: " + firebaseData.errorReason());
    }

    FirebaseJson jsonCarCount;
    jsonCarCount.set("/vehicleCount", count);
    jsonCarCount.set("/available slot", available);

    String pathCarCount = "ParkingData/carCount";

    if (Firebase.set(firebaseData, pathCarCount, jsonCarCount)) {
        Serial.println("Firebase carCount updated successfully at " + pathCarCount);
    } else {
        Serial.println("Error updating carCount: " + firebaseData.errorReason());
    }
}

void deleteNodeFromFirebase(int nodeNumber) {
    if (nodeNumber <= 0) {
        Serial.println("Invalid node number: " + String(nodeNumber));
        return;
    }

    String pathNode = "ParkingData/" + String(nodeNumber);
    FirebaseJson jsonUpdate;
    jsonUpdate.set("/UID_Right", "DELETED");
    jsonUpdate.set("/UID_Left", "DELETED");
    jsonUpdate.set("/CarlicenseRight", "DELETED");

    if (Firebase.updateNode(firebaseData, pathNode, jsonUpdate)) {
        Serial.println("Node " + String(nodeNumber) + " marked as deleted.");
    } else {
        Serial.println("Error marking node " + String(nodeNumber) + " as deleted: " + firebaseData.errorReason());
    }
}

void fetchCarLicenseRight(int nodeID) {
    String pathNode = "ParkingData/" + String(nodeID) + "/CarlicenseRight";
    if (Firebase.getString(firebaseData, pathNode)) {
        String carLicense = firebaseData.stringData();
        CarlicenseRight.push_back(carLicense);
        Serial.println("Fetched CarlicenseRight: " + carLicense + " for nodeID: " + String(nodeID));
    } else {
        Serial.println("Error fetching CarlicenseRight: " + firebaseData.errorReason());
    }
}
