# Features

## Bluetooth interface

ESP Rowing Monitor provides several BLE interfaces (Bluetooth profiles) to get the data and metrics it measures.

It supports dual BLE connections, allowing simultaneous use of two clients, e.g. a smartwatch and the [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/) or other compatible apps (like Kinomap, EXR, etc.).

### Standard BLE profiles

ESP Rowing Monitor supports three standard BLE profiles that allows it to be connected to smart watches and 3rd party apps:

1. Cycling Speed and Cadence Sensor profile
2. Power Meter Sensor profile
3. Fitness Machine Rowing profile (currently experimental)

Switching between the profiles can be done through the WebGUI, via BLE Control Point using a specific OpCode (18). The implementation of these profiles complies with BLE standards, although the OpCode used is not standard. Alternatively all settings are settable through the [Settings Service profile](custom-ble-services.md#settings-service).

All the standard BLE profiles are fully compliant with BLE standards, making them accessible by compatible devices such as smartwatches and other clients like Kinomap, EXR etc. They have been tested with Garmin smartwatches and EXR, including FR235, FR645, and FR255.

**To obtain accurate speed and distance data with the Cycling Speed and Cadence or Cycling Power Sensor mode, the wheel circumference must be set to 10mm when pairing the device with ESP Rowing Monitor.**

Please note that in order for Garmin watches to work with ESP Rowing Monitor, a cycling activity must be selected. Otherwise, due to limitations of Garmin, the watch will connect but will not use the sensor data for metrics.

Please note that the FTMS profile currently does not support the full range of options (e.g. device control) and metrics. The supported metrics in addition to stroke and stroke rate are:

- Total distance
- Pace
- Resistance level
- Power measurement;

### Custom BLE Services

In addition to the implemented standard profiles it exposes certain custom profiles for additional metrics (fully supported by the official [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/)). These are:

1. Extended Metrics (metrics not included in the base profiles)
2. Handle Forces recorded during the last drive

Please see more details on their specifications and protocols under [Custom BLE Services](docs/custom-ble-services.md).

## Over-the-Air updates

Version 6 introduced over-the-air bluetooth update protocol. The protocol is implemented in the WebGUI so installation can be done from there. This makes updates much easier as the device may not need to be removed. As well as allows potential auto update feature in future.

More detailed on the specification of the protocol can be found [here](docs/custom-ble-services.md#over-the-air-updater)

## Logging

ESP Rowing Monitor implements a logging mechanism with different log levels (e.g. silent, error, info, trace, verbose, etc.). These logs are sent via serial (UART0) only, so the ESP32 MCU should be connected via USB to view the logs. The log level (0-6) can be set via the BLE Control Point using OpCode 17.

Trace level logging is useful during the initial calibration process as it prints the delta times that can be used for replay. Further details can be found in the [Calibration](settings.md#calibration)

It is possible to log deltaTimes (i.e. time between impulses) to an SD card (if connected and enabled). DeltaTimes are collected in a `vector` and written to SD card on every stroke (after the drive ends) or 4 seconds (which ever happens earlier). This incremental way of making deltaTimes available is to optimize performance.
