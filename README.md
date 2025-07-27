# Fire Detection and Prevention System

A real-time fire and gas detection system using ESP32, various sensors, and a custom-designed PCB. Built as a course project at HCMUTE, this system aims to detect hazardous conditions and trigger alarms and responses automatically.

## Overview

This project monitors:
- Gas concentration (MQ-5)
- Temperature & humidity (DHT11)
- Flame detection (IR sensor)

If danger is detected, the system:
- Activates a buzzer
- Displays alerts on an LCD
- Triggers fan and water pump
- Sends data to cloud (Adafruit IO via MQTT)

## Hardware Design

- Controller: ESP32-WROOM-32  
- Sensors: MQ-5, DHT11, LM393 flame sensor  
- Actuators: Fan, water pump, buzzer  
- Display: LCD 16x2 with I2C  
- Power: 2x 18650 + LM2596 buck converter  
- PCB: Single-layer, designed in KiCad

## Schematic and PCB Layout

### Schematic Diagram

<img width="1433" height="657" alt="image" src="https://github.com/user-attachments/assets/64ef14bc-3c6b-4761-894a-3218624ba9f8" />


### PCB Layout

<img width="788" height="795" alt="image" src="https://github.com/user-attachments/assets/75b89a89-03fa-4bdd-ba57-88c444c91ed0" />

<img width="780" height="734" alt="image" src="https://github.com/user-attachments/assets/d0054280-d217-4645-81d0-582601c932f6" />




