# STM32 Bluetooth Multi-Sensor Telemetry & Control Robot

A smartphone-controlled robot car using STM32 with Bluetooth UART for real-time motion control and bidirectional sensor telemetry.

## Hardware
- STM32F407 microcontroller
- HC-05 Bluetooth module (UART)
- I2C temperature sensor
- ADC potentiometer
- DC motors

## How it works
The robot receives motion commands from a smartphone via Bluetooth UART. Simultaneously, it reads temperature via I2C and analog data via ADC, sending sensor data back to the phone in real time — full bidirectional communication.

## Tech Stack
- STM32 Bare-Metal (UART, I2C, ADC)
- C
- Keil uVision

## Demo
[Add photo or video here]
