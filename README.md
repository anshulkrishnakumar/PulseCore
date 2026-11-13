# PulseCore — Embedded Telemetry & Diagnostics Platform

## Overview

PulseCore is an embedded monitoring system that collects environmental sensor data, processes it using a real-time operating system, and sends telemetry to a Linux host application for analysis and storage.

The system uses an NXP FRDM-MCXN947 development board running Zephyr RTOS and a BME280 environmental sensor.

The Linux host application is written in C++17 and uses SQLite to persist sensor readings and diagnostic events.

## System Architecture

```text
BME280 Sensor
     |
     | I2C
     v
FRDM-MCXN947
     |
     | Zephyr RTOS
     |
     +-- Sensor Acquisition Thread
     |
     +-- Message Queue
     |
     +-- Telemetry Thread
     |
     +-- Monitoring Thread
     |
     | USB Serial (115200 baud)
     v
Linux Host Application (C++17)
     |
     +-- Telemetry Parser
     |
     +-- Diagnostic Engine
     |
     +-- SQLite Database
```

## Hardware

- NXP FRDM-MCXN947 development board
- Bosch BME280 environmental sensor
- USB cable
- Linux PC

### BME280 Wiring

| BME280 | FRDM-MCXN947 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | J2 pin 18 |
| SCL | J2 pin 20 |

The BME280 communicates with the microcontroller over I2C using address `0x76`.

## Software and Technologies

- C
- C++17
- Zephyr RTOS
- I2C
- USB Serial / UART
- CMake
- SQLite
- GoogleTest
- Linux

## Firmware

The embedded firmware runs three Zephyr threads:

1. **Sensor Acquisition Thread:** Reads temperature, humidity, and pressure from the BME280 every two seconds.
2. **Telemetry Thread:** Receives sensor readings through a message queue and sends structured telemetry over USB serial.
3. **Monitoring Thread:** Periodically reports the number of produced and consumed samples.

A mutex protects shared monitoring counters.

### Failure Detection

The firmware tracks consecutive sensor communication failures. After three consecutive failures, it reports a communication error.

The sensor is retried automatically on subsequent acquisition cycles. Successful sensor readings reset the failure counter.

## Host Application

The Linux host application:

- Opens the USB serial interface.
- Receives telemetry at 115200 baud.
- Parses structured sensor records.
- Validates sensor measurements.
- Evaluates diagnostic rules.
- Stores sensor readings and diagnostic events in SQLite.

### Diagnostic Rules

The current demonstration thresholds are:

| Measurement | Warning condition |
|---|---|
| Temperature | Below 0°C or above 50°C |
| Humidity | Below 20% or above 80% |
| Pressure | Below 80 kPa or above 110 kPa |

These are project demonstration thresholds, not manufacturer safety limits.

## Database

PulseCore uses SQLite with two tables:

### sensor_data

Stores:
- Record ID
- Timestamp
- Temperature
- Humidity
- Pressure

### diagnostic_events

Stores:
- Event ID
- Timestamp
- Severity
- Diagnostic code
- Diagnostic message

## Building the Firmware

Activate the Zephyr environment:

```bash
source ~/zephyrproject/.venv/bin/activate
export PATH="/usr/local/LinkServer:$PATH"
cd ~/zephyrproject
```

Build:

```bash
west build -p always \
-b frdm_mcxn947/mcxn947/cpu0 \
pulsecore -d build/pulsecore
```

## Flashing the Board

```bash
cd ~/zephyrproject

LinkServer flash MCXN947:FRDM-MCXN947 \
load build/pulsecore/zephyr/zephyr.bin:0x10000000
```

## Running the Host Application

Build the host application:

```bash
cd ~/zephyrproject/pulsecore/host
cmake --build build
```

Run:

```bash
./build/pulsecore_host
```

The application connects to `/dev/ttyACM0`.

## Running Unit Tests

```bash
cd ~/zephyrproject/pulsecore/host

ctest --test-dir build --output-on-failure
```

Latest recorded result: **25 tests passed, 0 failed.**

## Database Inspection

Count stored sensor readings:

```bash
sqlite3 pulsecore.db \
"SELECT COUNT(*) FROM sensor_data;"
```

Inspect recent readings:

```bash
sqlite3 pulsecore.db \
"SELECT * FROM sensor_data ORDER BY id DESC LIMIT 5;"
```

Count diagnostic events:

```bash
sqlite3 pulsecore.db \
"SELECT COUNT(*) FROM diagnostic_events;"
```

## Project Status

Implemented and tested:
- BME280 sensor acquisition over I2C
- Three-thread Zephyr RTOS architecture
- Message queue communication
- Mutex-protected monitoring counters
- Structured serial telemetry
- Sensor communication failure detection
- Sensor reconnection testing
- C++ telemetry parsing
- Diagnostic engine
- SQLite persistence
- 25 passing host-side unit tests
- End-to-end hardware-to-host data flow

Remaining work includes additional reliability testing and final demonstration preparation.