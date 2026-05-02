# Vehicle Fault Detection System

A distributed embedded system designed to monitor critical vehicle subsystems, detect abnormal conditions in real-time, and log **Diagnostic Trouble Codes (DTCs)** into persistent storage.

## System Architecture: Dual-ECU Design
The system utilizes two **ATmega32** microcontrollers (8 MHz) communicating via a custom **UART protocol**:
*   **HMI ECU**: Manages the driver interface, including a $4 \times 4$ Keypad for commands and a $4 \times 16$ LCD for real-time data visualization and fault scrolling.
*   **Control ECU**: The system core that interfaces with sensors (LM35, Ultrasonic), controls actuators (Window DC Motors), and manages fault logging via an external I2C EEPROM.

---

## Key Features
*   **Real-Time Monitoring**: Continuous tracking of engine temperature and obstacle distance.
*   **Persistent Fault Logging**: Diagnostic Trouble Codes (DTCs) are saved to an external **24C16 EEPROM** via **I2C**, ensuring logs remain even after power loss.
*   **Diagnostic Trouble Codes (DTCs)**:
    *   **P001**: Obstacle distance < 10 cm (Accident warning).
    *   **P002**: Engine temperature > 90°C (Overheat warning).
*   **Interactive Control**: A menu-driven interface allows the user to Start/Stop monitoring, view live values, and retrieve stored fault history.

---

## Technical Specifications

### Communication Protocols
*   **UART**: Inter-ECU communication with dynamic configuration (Baud rate, Parity, Stop bits).
*   **I2C (TWI)**: High-speed communication with external EEPROM.

### Driver Implementation
*   **ADC Driver**: Polling-based design with internal/external voltage reference configuration.
*   **Timer Driver**: Advanced driver supporting Normal and Compare modes with a callback-based interrupt technique.
*   **PWM**: Timer0 used in non-inverting mode to drive the Window Control Unit.
*   **ICU**: Utilized for precise distance measurement through the Ultrasonic driver.
