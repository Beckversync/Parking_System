# 🚗 ESP32 License Plate Detection and WebApp with Firebase 🚗

## 🌐 Introduction
This project integrates the **ESP32** microcontroller with a **web application** to manage an **automated smart parking system**. The system uses ESP32 to connect various sensors, perform **🔍 License Plate Detection**, and update parking status to a **🔥 Firebase Realtime Database**. A web app is developed to monitor and manage parking information in **⏱️ Real Time**.
<p align="center">
<img width=800 src="https://github.com/user-attachments/assets/00080578-97aa-42df-8451-d7cf5b82c06d"/>
</p>
---

## ✨ Key Features

### ⚙️ ESP32
- 🛠️ **Sensor Integration**: Reads data from RFID, infrared sensors, and camera modules.
- 📸 **License Plate Detection**: Uses specialized libraries or APIs for vehicle plate recognition.
- ☁️ **Firebase Communication**: Sends and receives data to/from Firebase for real-time updates.

### 💻 Web Application
- 📊 **Real-time Parking Status Display**: Shows available slots, occupied slots, and vehicle details.
- 🛑 **Barrier Control**: Updates and manages entry/exit barriers based on real-time vehicle detection.

### 🔥 Firebase
- 🚗 **Vehicle Entry/Exit Management**: Logs vehicle information and time of entry/exit.
- 🅿️ **Slot and Barrier State Management**: Tracks parking slot availability and barrier positions.

---

## 🏗️ System Architecture

### 🧰 Hardware
- 🧠 **ESP32 (ESP-WROOM-32)**: Central microcontroller for data acquisition and control.
- 🎥 **Infrared Sensors, RFID, and Camera**: Detects vehicle presence and captures license plates.
- 🚧 **Servo-controlled Barrier**: Controls the physical entry and exit points of the parking lot.

### 💾 Software
- 🌐 **Web Application**: Developed with **C#**, integrated with **Firebase Realtime Database**.
- ☁️ **Firebase**: Cloud-based data storage and synchronization platform.
- 🔄 **ESP32 FreeRTOS**: Manages multiple concurrent tasks efficiently.

---

🚀 This system provides an **efficient, automated solution** for smart parking management, enhancing real-time monitoring and control capabilities.
