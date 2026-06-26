# Hexapod Spider Robot 🕷️

## Graduation Project | Communications Engineering | KMA 2026

This repository contains the full design, code, and documentation for a six-legged spider robot (hexapod) built for surveillance and monitoring applications. The project was developed as a graduation requirement for the BSc in Communication Engineering at the Higher Institute of Engineering and Technology - King Marriott (KMA).

---

## Abstract

This project presents the design and implementation of a six-legged spider robot (hexapod) dedicated to surveillance and monitoring applications. The proposed system is built around an ESP32-S3 N16R8 UNO microcontroller, which serves as the central processing unit, responsible for data acquisition, locomotion control, and wireless communication. The robot's movement is actuated using eighteen MG996R servo motors, managed by a PCA9685 16-channel PWM driver to ensure precise and stable control of joint angles.

The mechanical structure is a lightweight, 3D-printed or laser-cut hexapod frame, designed for agility and low power consumption. Unlike conventional hexapods used for heavy lifting or manipulation, this robot focuses exclusively on passive surveillance, carrying no active payloads beyond sensors. Real-time video and telemetry data are transmitted wirelessly via the ESP32-S3 N16R8 UNO built-in Wi-Fi to a remote monitoring station.

---

## Project Overview

In the rapidly evolving field of robotics, biomimetic designs have emerged as a powerful solution to the limitations of traditional wheeled and tracked systems. Among these, the Hexapod Robot, commonly referred to as the Spider Robot, stands out as a sophisticated multi-legged platform inspired by the biological structure of insects.

The primary advantage of a six-legged configuration is its exceptional stability and maneuverability. Unlike wheeled robots that require flat surfaces, a hexapod can navigate highly irregular terrains, climb over obstacles, and maintain a stable center of gravity even when one or more legs are off the ground. This capability is known as statically stable gait, which allows the robot to halt at any point during its movement without falling.

The robot is built around an **ESP32-S3 N16R8 UNO** microcontroller, which handles locomotion, sensor data, and wireless communication. Movement is driven by **18x MG996R** servo motors controlled via a **PCA9685** PWM driver.

### System Features

- **Real-time Video Streaming** using a TP-Link Tapo C110 2K camera via RTSP.
- **AI Object Detection** using the YOLOv8 model (integrated with OpenCV).
- **Multi-Interface Control**: Web dashboard (FastAPI), Mobile app (Flutter/BLE), and Serial commands.
- **Autonomous Obstacle Avoidance** using an HC-SR04 ultrasonic sensor.
- **Statically Stable Tripod Gait**: Smooth, coordinated walking using Bézier curve trajectory interpolation.
- **Inverse Kinematics (IK)**: Real-time calculation of joint angles for precise foot placement.
- **Power Distribution System**: Isolated power rails for logic (3.3V) and high-torque servos (5V/20A).

---

## Objectives

The main objectives of designing and implementing a six-legged spider robot are as follows:

- **To achieve stable locomotion on uneven terrain**: Developing control algorithms (e.g., tripod gait) that maintain static and dynamic stability even when one or more legs lose contact.
- **To enable obstacle negotiation**: Allowing the robot to climb over obstacles up to twice its leg length using adaptive leg lifting and swinging motions.
- **To provide fault tolerance**: Ensuring the robot can continue moving even after a leg malfunction by redistributing support polygons and adjusting gait.
- **To minimize ground pressure**: Reducing environmental impact compared to wheeled vehicles, useful for delicate terrains like sand or vegetation.
- **To serve as a research platform**: Creating a modular system for testing artificial intelligence, reinforcement learning, and sensor fusion techniques.

---

## Applications

Hexapod spider robots are used across multiple domains:

| Application | Description |
|-------------|-------------|
| **Search and Rescue** | Locate victims under rubble after earthquakes by crawling through narrow spaces |
| **Military Reconnaissance** | Perform silent surveillance in forest or mountain terrain |
| **Agriculture** | Perform weeding and crop monitoring in uneven fields, reducing soil compaction |
| **Mining Inspection** | Walk through narrow, unstable tunnels to inspect for gas leaks |
| **Education and Research** | Serve as platforms for teaching robotics, kinematics, and control theory |
| **Hazardous Material Handling** | Inspect nuclear waste sites with minimal ground pressure |
| **Environmental Monitoring** | Data collection in sensitive ecosystems with minimal disturbance |

---

## Repository Structure


---

## Hardware Components

| Component                | Model / Specs                         |
|--------------------------|---------------------------------------|
| **Microcontroller**      | ESP32-S3 N16R8 UNO (Dual-core, 240MHz)|
| **Actuators**            | 18 x MG996R Pro Servos (11 kg/cm)     |
| **Servo Driver**         | 2 x PCA9685 (I2C, 16 channels each)   |
| **Camera**               | TP-Link Tapo C110 (2K, RTSP)          |
| **Sensors**              | HC-SR04 Ultrasonic (Obstacle Avoidance)|
| **Battery**              | LiPo 3S 11.1V 3300mAh 35C             |
| **Power Regulation**     | 20A DC-DC Buck Converter (5V) & LM2596 |
| **Charger**              | iMAX B33 LiPo Balance Charger (2S-3S) |

---

## Mechanical Design

### Leg Configuration

The insectoid configuration (similar to ants/stick insects) was selected due to its superior terrain adaptability and energy efficiency during tripod gait. Each leg consists of three links:

- **Coxa** - Horizontal rotation (yaw), determining the direction of the stride
- **Femur** - Vertical lift and extension
- **Tibia** - Final reach and height of the foot contact point

### Statically Stable vs. Statically Unstable

The robot is **statically stable** if the vertical projection of its center of gravity (COG) lies inside the polygon formed by its feet on the ground. This allows the robot to freeze in place safely. The tripod gait implementation used in this project ensures static stability by maintaining a constant stable base while the remaining three legs transition to the next position.

### Kinematics

Two types of kinematics are used:

- **Forward Kinematics (FK)**: Calculates foot position from known joint angles
- **Inverse Kinematics (IK)**: Calculates required joint angles from desired foot position

The IK solver uses the Law of Cosines to determine joint angles for each leg, enabling precise foot placement in 3D space.

---

## Locomotion Mechanism

### Tripod Gait

The tripod gait is the most common statically stable gait for six-legged robots. The six limbs are synchronized into two sets of alternating tripods:

- **Set A**: Front Left (FL), Middle Right (MR), Back Left (BL)
- **Set B**: Front Right (FR), Middle Left (ML), Back Right (BR)

### Trajectory Interpolation via Bézier Curves

Rather than using linear interpolation (LERP), which results in abrupt velocity changes, the project utilizes **Bézier curves** for smooth trajectory planning:

- **Quadratic Bézier Trajectories**: By defining control points for the start, peak (lift), and end of a stride, the trajectory follows a smooth, continuous curve
- **Formula**: B(t) = (1-t)²P₁ + 2(1-t)tP₂ + t²P₃
- **Benefit**: Ensures zero velocity at ground contact and lift-off moments, minimizing impact forces

### Movement Features

| Direction | Swing Phase | Stance Phase |
|-----------|-------------|--------------|
| **Forward** | Lifted legs move back → front | Ground legs move front → back |
| **Backward** | Lifted legs move front → back | Ground legs move back → front |
| **Turn Left** | Feet move in clockwise arc | Ground legs drag toward right |
| **Turn Right** | Feet move in counter-clockwise arc | Ground legs drag toward left |

---

## System Architecture

The system architecture consists of four core subsystems:

### 1. Central Processing and Control Unit (ESP32-S3)
- Receives and processes data from sensors
- Generates control signals for servo motors
- Handles wireless communication

### 2. Vision & Remote Control System
- **Camera (Tapo C110)**: Captures and streams video
- **Mobile App**: Provides user interface for manual control

### 3. Sensing Unit
- **Ultrasonic Sensor (HC-SR04)**: Distance measurement and obstacle avoidance

### 4. Actuation System
- **PCA9685 Drivers**: Convert digital commands to PWM signals
- **MG996R Servos**: High-torque servo motors for leg joints

### 5. Power Grid
- **Source**: 11.1V LiPo Battery (3300mAh)
- **LM2596S Converter**: Steps down to 3.3V for logic
- **Buck Converter (5V/20A)**: Provides stable 5V for all servos

---

## Getting Started

### 1. Prerequisites
- **ESP32**: Arduino IDE with ESP32 board package
- **Python**: Install `opencv-python`, `ultralytics` (YOLO), and `requests`
- **Flutter**: For building the mobile app

### 2. Setup
1. **Upload Firmware**: Flash `hexapod_esp32_main.cpp` to the ESP32
2. **Network**: Update the Wi-Fi credentials in the code
3. **Camera Stream**: Configure the RTSP URL in `camera_ai_pipeline.py`
4. **Mobile App**: Build and run the Flutter project on your phone

### 3. Commands

| Command | Action |
|---------|--------|
| `forward` | Move forward |
| `backward` | Move backward |
| `turnleft` | Turn left |
| `turnright` | Turn right |
| `stop` | Stop all movement |
| `home` | Return to home position |
| `h[level]` | Set height (-5 to +5) |
| `s[level]` | Set yaw (-5 to +5) |

---

## Future Enhancements

| Enhancement | Description |
|-------------|-------------|
| **Solar Power** | Add flexible solar panels for extended battery life |
| **Autonomous Navigation** | Use LiDAR for SLAM and mapping |
| **Night Vision** | Add thermal camera and LiDAR for complete darkness |
| **Swarm Communication** | Multiple robots working together using LoRa/Wi-Fi Mesh |
| **Full Autonomy** | AI decision-making without human intervention |
| **Cliff Detection** | Downward sensors to prevent falls |
| **Gesture & Voice Control** | Control via hand gestures or voice commands |
| **Fault Diagnosis** | AI to detect and recover from servo failures |
| **Water & Dust Resistance** | Environmental sealing for outdoor use |

---



## License

This project is open-source and available for educational and research purposes.

---

## References

1. Pfeifer et al., "Self-Organization and Biologically Inspired Robotics," *Science*, 2007.
2. Raibert, *Legged Robots That Balance*, MIT Press, 1986.
3. Saranli et al., "RHex: A Simple and Highly Mobile Hexapod Robot," *The International Journal of Robotics Research*, 2001.
4. Yang & Kim, "Fault-Tolerant Locomotion of a Hexapod Robot," *IEEE Transactions on Systems, Man, and Cybernetics*, 1999.
5. Belter & Skrzypczyński, "A Biologically Inspired Approach to Learning Visual Behaviors for a Hexapod Robot," *Journal of Intelligent & Robotic Systems*, 2013.
6. Murphy, "Disaster Robotics: A 10-Year Review," *IEEE Workshop on Safety, Security, and Rescue Robotics*, 2014.
7. Bechar & Vigneault, "Agricultural Robots for Field Operations," *Computers and Electronics in Agriculture*, 2016.
