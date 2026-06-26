# Hexapod Spider Robot 🕷️

## Graduation Project | Communications Engineering | KMA 2026

This repository contains the full design, code, and documentation for a six-legged spider robot (hexapod) built for surveillance and monitoring applications. The project was developed as a graduation requirement for the BSc in Communication Engineering at the Higher Institute of Engineering and Technology - King Marriott (KMA).


---

## Project Overview

The robot is built around an **ESP32-S3 N16R8 UNO** microcontroller, which handles locomotion, sensor data, and wireless communication. Movement is driven by **18x MG996R** servo motors controlled via a **PCA9685** PWM driver. The system features:

- **Real-time Video Streaming** using a TP-Link Tapo C110 2K camera via RTSP.
- **AI Object Detection** using the YOLOv8 model (integrated with OpenCV).
- **Multi-Interface Control**: Web dashboard (FastAPI), Mobile app (Flutter/BLE), and Serial commands.
- **Autonomous Obstacle Avoidance** using an HC-SR04 ultrasonic sensor.

---

## Repository Structure


---

## Hardware Components

| Component                | Model / Specs                         |
|--------------------------|---------------------------------------|
| **Microcontroller**      | ESP32-S3 N16R8 UNO (Dual-core)        |
| **Actuators**            | 18 x MG996R Pro Servos (11 kg/cm)     |
| **Servo Driver**         | 2 x PCA9685 (I2C, 16 channels each)   |
| **Camera**               | TP-Link Tapo C110 (2K, RTSP)          |
| **Sensors**              | HC-SR04 Ultrasonic (Obstacle Avoidance)|
| **Battery**              | LiPo 3S 11.1V 3300mAh 35C             |
| **Power Regulation**     | 20A DC-DC Buck Converter (5V) & LM2596 |

---

## Key Features

- **Statically Stable Tripod Gait**: Smooth, coordinated walking using Bézier curve trajectory interpolation.
- **Inverse Kinematics (IK)**: Real-time calculation of joint angles for precise foot placement.
- **Remote Control**:
    - **Web**: Full dashboard with sliders for height & yaw adjustments.
    - **Mobile**: Flutter app with Bluetooth (BLE) and directional buttons.
- **AI Vision**: Object detection pipeline running on a connected PC, capable of sending movement commands back to the robot.
- **Power Distribution System**: Isolated power rails for logic (3.3V) and high-torque servos (5V/20A).

---

## Getting Started

### 1. Prerequisites
- **ESP32**: Arduino IDE with ESP32 board package.
- **Python**: Install `opencv-python`, `ultralytics` (YOLO), and `requests`.
- **Flutter**: For building the mobile app.

### 2. Setup
1.  **Upload Firmware**: Flash `hexapod_esp32_main.cpp` to the ESP32.
2.  **Network**: Update the Wi-Fi credentials in the code.
3.  **Camera Stream**: Configure the RTSP URL in `camera_ai_pipeline.py`.
4.  **Mobile App**: Build and run the Flutter project on your phone.

---

## Future Enhancements

- **Solar Power Integration** for extended battery life.
- **SLAM & Autonomous Navigation** using LiDAR.
- **Swarm Communication** for multi-robot coordination.
- **Environmental Sealing** for outdoor and industrial use.

---
