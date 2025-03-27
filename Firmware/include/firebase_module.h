#ifndef FIREBASE_MODULE_H
#define FIREBASE_MODULE_H

#include <FirebaseESP32.h>
#include "config.h"

// Cho phép các module khác truy cập đối tượng Firebase
extern FirebaseData firebaseData;
extern FirebaseConfig config;
extern FirebaseAuth auth;

void firebaseSetup();
void ensureFirebaseConnection();
void handleFirebaseStream();
void updateFirebase(int nodeNumber, bool gate_L, bool gate_R, String uid_R, String uid_L, int available, int count, String carLicenseRight, bool leStatus, bool riStatus, String carLicenseLeft);
void deleteNodeFromFirebase(int nodeNumber);
void fetchCarLicenseRight(int nodeID);

#endif
