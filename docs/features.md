# Features

## Cyclic Error Filtering

As of version 7.0.0, ESP Rowing Monitor includes an advanced cyclic error filtering system that improves the accuracy of metrics by correcting timing variations caused by mechanical imperfections in the flywheel magnet placement.

**Benefits:**

- **Cleaner Force Curves**: The filter produces cleaner handle force curves by correcting impulse timing variations.
- **Improved Accuracy**: More accurate stroke detection and metric calculations due to cleaner input data.
- **Reduced Data Requirements**: The cyclic error correction potentially allows using a smaller `IMPULSE_DATA_ARRAY_LENGTH` while maintaining or improving metric accuracy.

The cyclic error filter works by learning the systematic timing errors in the sensor signal and applying corrections to each impulse, effectively removing the periodic noise introduced by uneven magnet spacing or sensor positioning.

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

In addition to the implemented standard profiles, ESP Rowing Monitor exposes custom BLE services for additional metrics, settings management, and OTA updates. These are fully supported by the official [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/). The custom services are:

1. Extended Metrics Service, which includes extended metrics, handle forces, and optional delta-time logging
2. Settings Service for runtime-readable settings and runtime-configurable parameters when enabled
3. Over-the-Air Update Service for wireless firmware updates

Please see more details on their specifications and protocols under [Custom BLE Services](./custom-ble-services.md).

## Over-the-Air updates

Version 6 introduced an over-the-air Bluetooth update protocol. The protocol is implemented in the WebGUI so firmware updates can be performed from there. This makes updates much easier because the device does not need to be removed after the initial installation.

More details on the specification of the protocol can be found in the [OTA protocol documentation](./custom-ble-services.md#over-the-air-updater).

## Logging

ESP Rowing Monitor implements a logging mechanism with different log levels (e.g. silent, error, info, trace, verbose, etc.). These logs are sent via serial (UART0) only, so the ESP32 MCU should be connected via USB to view the logs. The log level (0-6) can be set via the BLE Control Point using OpCode 17.

Trace level logging is useful during the initial calibration process as it prints the delta times that can be used for replay. Further details can be found in the [Calibration](settings.md#calibration)

It is possible to log deltaTimes (i.e. time between impulses) to an SD card (if connected and enabled). DeltaTimes are collected in a `vector` and written to SD card on every stroke (after the drive ends) or 4 seconds (which ever happens earlier). This incremental way of making deltaTimes available is to optimize performance.
