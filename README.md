#  Smart Parking System: License Plate Recognition with ESP32, Firebase & SVM

This project integrates **ESP32-based IoT hardware**, a **web application**, and **machine learning (SVM)** to create an intelligent license plate recognition system for **automated smart parking**. It merges embedded technology, cloud services, and image processing to automate access control in a secure and efficient way.

---

##  System Overview

###  ESP32 Integration
The ESP32 microcontroller acts as the edge device, handling:
-  Sensor Input: RFID, infrared sensors, and camera modules.
-  License Plate Detection: Captures vehicle images using camera and sends them to a backend or image processor.
-  Firebase Communication: Updates vehicle status to Firebase Realtime Database in real-time.

###  Web Application
Built using **C#**, the WebApp allows:
-  Real-time monitoring of parking slots
-  Control of servo-based barrier gates
-  Display of recognized license plates and timestamps

###  Firebase
Used for cloud storage and live synchronization:
-  Logs entry/exit data
-  Manages slot availability and barrier control status
<p align="center">
<img width=800 src="https://github.com/user-attachments/assets/764a6b8b-dde8-49b3-a5c0-491303782a92"/>
</p>

##  License Plate Recognition with SVM

A machine learning algorithm was developed using MATLAB to process vehicle license plate images based on the following pipeline:

###  7-Step Processing Algorithm:
1. **Input Image**: `.jpg` images of license plates.
2. **Pre-processing**: Grayscale conversion, binarization, noise reduction.
3. **License Plate Localization**: Detecting character regions.
4. **Character Segmentation**: Isolating individual characters.
5. **Character Enhancement**: Binarization and cropping of characters.
6. **Character Recognition using SVM**:
   - Linear kernel SVM
   - Trained on 36 classes (A-Z, 0-9)
7. **Store Output**: Recognized plate numbers saved in a `log.txt` file.

 **Performance** (on 26 test images):
- License Plate Detection: 88%
- Character Segmentation: 84%
- Recognition Accuracy: 77%

<p align="center">
<img width=800 src="https://github.com/user-attachments/assets/b331f112-9282-41c3-a638-6690be473f76"/>
</p>

<p align="center">
<img width=800 src="https://github.com/user-attachments/assets/7831c842-78b7-40ea-9f77-c799f05470c7"/>
</p>

---



## 🔮 Future Enhancements
- Improve image quality and plate recognition under various lighting/weather conditions.
- Integrate deep learning (CNN) for higher accuracy.
- Add mobile app support for user interaction.
- Use edge AI on ESP32-CAM for real-time recognition at the edge.

