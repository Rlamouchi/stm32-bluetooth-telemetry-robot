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
[Video 1 -phone app ](4f603d04-ce4f-4a90-a6eb-a1fa5e1548c7.mp4)

[Video 2 - both](5ac1a772-7c3d-4e9a-acb7-c4f45f85f58d.mp4)

[Video 3 - Robot moving](66afb009-3d6d-4729-aad8-c85ca6d19989.mp4)

