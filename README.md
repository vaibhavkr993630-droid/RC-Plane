# Autonomous Aerial Payload Delivery System (Mechatronics Prototype)

![Project Status](https://img.shields.io/badge/Status-Prototype_Validated-success)
![Platform](https://img.shields.io/badge/Platform-Arduino_Uno-blue)
![Language](https://img.shields.io/badge/Code-C%2B%2B-orange)
![Domain](https://img.shields.io/badge/Domain-Mechatronics_%26_Embedded_Systems-red)

## 📌 Project Overview
This project involves the design, fabrication, and programming of a **Remote-Piloted Aerial Vehicle (RPAV)** capable of precise payload deployment. The system features a custom-built low-latency communication link (2.4GHz) and a stabilized control system designed to carry and release a 50g payload at a designated target coordinates.

Unlike standard hobbyist builds, this prototype focuses on **system reliability, failsafe logic, and signal processing**—mirroring safety-critical protocols used in industrial and medical device engineering.

---

## 🎯 Engineering Objectives
* **Precision Payload Delivery:** Engineered a servo-actuated release mechanism to deploy a 50g payload while maintaining flight stability.
* **Stable Flight Dynamics:** Optimized control surface throws to ensure stability during Center of Gravity (CG) shifts pre- and post-deployment.
* **Reliable Communication:** Implemented a high-fidelity 2.4GHz wireless link with packet loss handling and deadzone filtering.
* **Safety Protocols:** Integrated automated failsafe mechanisms to neutralize kinetic energy in case of signal loss.

---

## ⚙️ System Architecture & Hardware

### 1. Transmitter Unit (Ground Control)
* **MCU:** Arduino UNO (Atmega328P)
* **HMI (Human-Machine Interface):** Dual-axis analog joysticks for Pitch, Roll, Yaw, and Throttle control.
* **RF Module:** NRF24L01+ PA/LNA (2.4GHz) for extended range telemetry.
* **Signal Processing:** Implemented software-based input smoothing to eliminate ADC noise and prevent servo jitter.

### 2. Receiver Unit (Airframe)
* **MCU:** Arduino UNO
* **Actuation System:**
    * **Primary Flight Controls:** Servos for Elevator (Pitch), Rudder (Yaw), and Ailerons (Roll).
    * **Propulsion:** 1000KV BLDC Motor + Electronic Speed Controller (ESC).
    * **Payload Mechanism:** Dedicated high-torque servo for latch release.
* **Power Management:** 12V 2200mAh LiPo configuration with BEC (Battery Eliminator Circuit) for regulated 5V logic power.

---

## 🧠 Control Logic & Firmware Features

### Signal Processing & Jitter Reduction
Raw analog inputs from the joysticks are processed using a moving average filter to prevent mechanical wear on the servos caused by signal noise.

### Safety Failsafe Mode
To ensure operational safety, the receiver firmware includes a "Watchdog" function:
* **Trigger:** If no valid data packet is received for >1.0 seconds.
* **Action:** Immediate Throttle Cut (0%) and Control Surface Neutralization (Center).
* **Purpose:** Prevents "fly-away" scenarios and minimizes impact force, akin to medical device fail-safe states.

### Actuation Mapping
Non-linear mapping is used to convert joystick inputs into precise angular deflections, preventing mechanical stalling of the control surfaces.

| Control Surface | Min Angle (deg) | Neutral (deg) | Max Angle (deg) |
| :--- | :--- | :--- | :--- |
| **Elevator** | 80° | 115° | 150° |
| **Rudder** | 70° | 105° | 130° |
| **Left Wing** | 30° | 70° | 110° |
| **Right Wing** | 20° | 68° | 100° |

---

## 🛠️ Calibration & Setup Procedure

### ESC (Electronic Speed Controller) Calibration
The system requires a precise throttle range calibration to map the PWM signal (1000us - 2000us) to the motor's RPM.
1.  **Initialization:** Power off all units.
2.  **Max Throttle:** Set Transmitter throttle to 100%. Power on Receiver (ESC enters Calibration Mode).
3.  **Min Throttle:** After the confirmation tone, drop throttle to 0%.
4.  **Confirmation:** ESC emits final success tone, locking the linear throttle curve.

---

## 🧪 Testing & Validation Results
* **Communication Range:** Validated stable link connection up to **100 meters** (Line-of-Sight).
* **Latency:** Achieved control loop update rate of **~50Hz**, providing near real-time response.
* **Payload Test:** Successfully deployed 50g mass with minimal impact on flight trajectory due to high-lift airfoil selection.

---

## 🚀 Future Roadmap
* **Telemetry Integration:** Adding an OLED display to the transmitter for real-time voltage and signal strength monitoring (RSSI).
* **Autonomous Navigation:** Integration of GPS module for waypoint-based flight and "Return-to-Home" functionality.
* **PID Stabilization:** Implementation of an IMU (Inertial Measurement Unit) to provide active gyro-stabilization against wind gusts.

---
## 📁 Media

Check out *photos and a demo video* of our RC Plane in action:

🔗 [View Media Folder on Google Drive](https://drive.google.com/drive/folders/1uMWLwyu_kMYvRLL4WdzHgBq3qJ_EQbiB?usp=drive_link)

---
