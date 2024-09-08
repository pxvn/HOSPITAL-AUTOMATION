[![ViewCount](https://views.whatilearened.today/views/github/pxvn/ESP32-DHT-OLED-Arduino-Cloud-Monitoring.svg)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](#license)
[![Arduino Cloud](https://img.shields.io/badge/Arduino-Cloud-blue.svg)](#software-setup)
[![ESP32](https://img.shields.io/badge/ESP32-PlatformIO-blue.svg)](#hardware-required)
[![DHT11 Sensor](https://img.shields.io/badge/DHT11-Sensor-green.svg)](#hardware-required)
[![Adafruit GFX](https://img.shields.io/badge/Adafruit-GFX-orange.svg)](#software-setup)
[![SSD1306 OLED](https://img.shields.io/badge/SSD1306-OLED-yellow.svg)](#hardware-required)
[![Last Commit](https://img.shields.io/github/last-commit/pxvn/ESP32-DHT-OLED-Arduino-Cloud-Monitoring)](#)

# Automated Sanitation Facility for Bedridden

This was one of my capstone projects where I aimed to create an automated sanitation system for bedridden individuals. The goal was to improve their quality of life and maintain hygiene through a combination of sensors, actuators, and smart connectivity.

## Key Features

*   **Automatic Waste Detection and Disposal:** The system uses sensors to detect waste and trigger the cleaning process automatically.
*   **Integrated Cleaning:** It combines waste removal, water spraying, and hot air drying for thorough sanitation.
*   **IoT Connectivity:** The system connects to the Arduino IoT Cloud, enabling remote monitoring and notifications to caregivers.
*   **User-Friendly Interface:** A local LED screen and alarm provide instant feedback, and there's potential for a mobile app for remote interaction.

## Potential Applications

*   Hospitals and healthcare facilities
*   Home care for the elderly or people with disabilities
*   Rehabilitation centers

## Hardware Used

*   ESP32 development board
*   Moisture/Pressure Sensor (optional)
*   Waste Level Sensor (e.g., ultrasonic sensor)
*   Water Level Sensor
*   Suction Pump
*   Water Pump
*   Air Blower with Heating Element
*   Solenoid Valve
*   Relay Module (to control high-voltage components)
*   Relay Driver Circuit (for safety)
*   LED Screen and Buzzer/Alarm
*   (Optional) Components for a mobile app

## System Flow
```mermaid
%%{init: {'theme': 'default', 'flowchart': {'curve': 'linear', 'useMaxWidth': true}}}%%
flowchart TB
  %% Nodes
  subgraph User_or_Button_Trigger["User Button or Command"]
    A["Manual Button Press"]
    B["Start/Stop Automation"]
  end
  C["ESP32 Microcontroller"]
  subgraph Control_and_Operation["Control and Operation"]
    D["Water Pump (15s)"]
    E["Suction Pump (25s)"]
    F["Blower (40s)"]
  end
  subgraph Feedback_and_Monitoring["Feedback and Monitoring"]
    I["LCD Display Status"]
    J["Play Sound via Buzzer"]
    K["Component LED Indicators"]
    L["Send Notification (IoT)"]
  end

  %% Edges
  A --> C
  B --> C
  C --> D
  D --> E
  E --> F
  F --> I
  F --> J
  F --> K
  C --> L

  %% Styles
  classDef button fill:#f96,stroke:#333,stroke-width:2px;
  classDef component fill:#6c9,stroke:#333,stroke-width:2px;
  classDef notification fill:#69f,stroke:#333,stroke-width:2px;
  classDef feedback fill:#f0c30e,stroke:#333,stroke-width:2px;
  classDef subprocess fill:#d6e9c6,stroke:#333,stroke-width:2px;
  
  class A,B button;
  class D,E,F component;
  class I,J,K feedback;
  class L notification;
  class User_or_Button_Trigger,Control_and_Operation,Feedback_and_Monitoring subprocess;

