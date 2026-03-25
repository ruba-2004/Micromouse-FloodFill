<div align="center">
  
# Autonomous Micromouse Maze Solver
**ESP32-Based Pathfinding & Sensor Fusion Architecture**

[![C++](https://img.shields.io/badge/Language-C++-00599C.svg?style=flat-square&logo=c%2B%2B)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-ESP32-E7352C.svg?style=flat-square&logo=espressif)](https://www.espressif.com/)
[![License](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](#)

*A comprehensive embedded systems project featuring a dynamic Flood-Fill BFS algorithm, 3-layer PID sensor fusion, and real-time telemetry via an asynchronous web server.*

</div>

---

## 📌 Abstract
This repository contains the hardware documentation and C++ firmware for an autonomous Micromouse. [cite_start]The system is engineered to navigate an unknown 8x8 physical maze, dynamically map its environment using Time-of-Flight (ToF) LiDAR arrays [cite: 3][cite_start], and calculate the optimal path to the center using the Flood-Fill algorithm [cite: 93-101]. 

[cite_start]A core focus of this architecture is movement precision, achieved through a custom multi-layered PID control loop that fuses data from a 6-axis IMU, wheel odometry, and absolute LiDAR distance tracking [cite: 60-81].

## ⚙️ Core Software Architecture

### 1. Dynamic Pathfinding (Flood-Fill)
[cite_start]The navigation engine assumes an open grid and utilizes a Breadth-First Search (BFS) queue to calculate Manhattan distances from the target center [cite: 93-101]. [cite_start]As the robot discovers walls via LiDAR (`wallDetectDist < 150mm`)[cite: 28], the internal `mazeWalls` bitmask is updated, and the BFS queue dynamically recalculates the optimal escape route, preventing exhaustive dead-end searches.

### 2. Multi-Sensor PID Fusion
[cite_start]Straight-line movement without mechanical drift is maintained via the `fusedStraightCorrectionFull()` function, which dynamically weights three distinct error layers [cite: 60-81]:
* [cite_start]**Layer 1 (Gyroscope):** Maintains absolute heading lock (`currentYaw - headingLock`)[cite: 60].
* [cite_start]**Layer 2 (Encoders):** Synchronizes left and right wheel odometry to prevent slipping (`relLeft - relRight`)[cite: 63].
* [cite_start]**Layer 3 (LiDAR):** Provides absolute physical centering between maze walls using dual-lateral measurements [cite: 65-76].

### 3. Finite State Machine (FSM)
[cite_start]To prevent blocking code and maintain high-frequency sensor polling, the main `loop()` is governed by a switch-case FSM[cite: 314]. [cite_start]This dictates discrete operational states including `MAZE_SCAN`, `MAZE_DECIDE`, `MAZE_FORWARD`, and complex 3-phase kinematic turns (`TURN_LEFT_BACKUP` -> `TURN_LEFT_PIVOT` -> `TURN_LEFT_FORWARD`) [cite: 324-329].

### 4. Real-Time Telemetry Dashboard
[cite_start]The ESP32 hosts an asynchronous web server (`WebServer server(80)`)[cite: 2]. [cite_start]This provides a live HTML/JS UI dashboard to stream sensor telemetry, visualize the 8x8 maze array state, and adjust PID weights (`gyroKp`, `encKp`, `wallKp`) without reflashing the firmware [cite: 8, 11, 13, 149-258].

---

## 🛠️ Hardware Specifications

### Component Matrix
| Subsystem | Component | Function |
| :--- | :--- | :--- |
| **Microcontroller** | ESP32 Development Board | Central processing, FSM, and Web Server. |
| **Spatial Awareness** | 3x VL53L0X ToF LiDARs | Forward collision avoidance & lateral wall mapping. |
| **Inertial Tracking** | Adafruit MPU6050 | 6-axis Gyroscope/Accelerometer for heading lock. |
| **Odometry** | 2x DC Motors w/ Quadrature Encoders | Interrupt-driven pulse counting for linear distance. |
| **Motor Control** | Dual H-Bridge Module | PWM logic translation to high-current delivery. |

### Microcontroller Pin Mapping
[cite_start]The system utilizes a dual-I2C bus architecture to prevent sensor polling conflicts [cite: 262-265]. 

| Component | Pins | Protocol / Assignment |
| :--- | :--- | :--- |
| **MPU6050 (IMU)** | 19 (SDA), 18 (SCL) | [cite_start]`Wire` I2C Bus [cite: 262] |
| **VL53L0X LiDARs** | 5 (SDA), 17 (SCL) | [cite_start]`Wire1` I2C Bus [cite: 264] |
| **LiDAR XSHUT** | 15 (L), 4 (F), 16 (R) | Digital Out (Dynamic Address Assignment) |
| **Left Encoder** | 33 (A), 32 (B) | [cite_start]Hardware Interrupts [cite: 42, 260] |
| **Right Encoder** | 34 (A), 35 (B) | [cite_start]Hardware Interrupts [cite: 43, 260] |
| **Motor L / R** | 13, 27 / 25, 26 | [cite_start]PWM Output [cite: 2, 50-56] |

---

## ⚠️ Engineering Challenges & Resolutions

1. **Power Isolation & Thermal Runaway:** Initial prototypes suffered I/O expander failure due to voltage spikes exceeding the 3.3V logic tolerance. **Resolution:** Implemented strict electrical isolation between the motor power distribution network and the logic network, utilizing a dedicated buck converter to guarantee a clean 3.3V logic supply.
2. **I2C Bus Saturation:** Running three ToF sensors and an IMU on the default ESP32 I2C pins resulted in bus lockups and continuous polling failures. [cite_start]**Resolution:** Shifted to a dual-I2C architecture utilizing the `<Wire.h>` library [cite: 1, 262-265]. [cite_start]The IMU operates independently on `Wire0`, while the LiDAR array operates on `Wire1`, utilizing `XSHUT` pins to assign sequential hex addresses (0x30, 0x31, 0x29) during boot [cite: 263-265].

---

## ⚙️ Dependencies & Installation
[cite_start]To compile this firmware, the following libraries are required[cite: 1]:
* `Wire.h` / `WiFi.h` / `WebServer.h` (Native ESP32)
* `Adafruit_VL53L0X.h`
* `Adafruit_MPU6050.h`
* `Adafruit_Sensor.h`

Compile using the Arduino IDE or PlatformIO with the board set to **ESP32 Dev Module**. [cite_start]Ensure the baud rate for the Serial Monitor is set to `115200`[cite: 259].
