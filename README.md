
# ESP32 Rowing Monitor

The purpose of this project is to provide professional-grade rowing (and other similar ergometer) analytics to indoor rowing machines, similar to the [Open Rowing Monitor Project](https://github.com/laberning/openrowingmonitor) (ORM), but with a much more affordable ESP32 microcontroller.

> 💡 **New to ESP Rowing Monitor?** Start with the [DeepWiki documentation](https://deepwiki.com/Abasz/ESPRowingMonitor) for an interactive guide to setup, calibration, and troubleshooting. Or explore the [FAQ](docs/faq.md) for quick answers to common questions.

## ⚠️ Breaking Changes (7.0.0)

### Stroke Detection Settings BLE Characteristic (Dec 2025)

The `minimumRecoverySlopeMargin` setting has been **removed** from the firmware. This is a **breaking change** affecting the BLE Settings Service:

- **Stroke Detection Settings Characteristic** payload size changed from **15 bytes to 11 bytes**
- Old GUI clients sending the 15-byte payload will receive an `InvalidParameter` BLE error
- Firmware will **not** accept or parse old-format payloads
- **Required action**: Use the latest version of the [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/)

See [Custom BLE Services](docs/custom-ble-services.md#settings-service) for the updated byte layout.

### Extended Metrics BLE Characteristic (Dec 2025)

The `dragFactor` field in the Extended Metrics characteristic has been changed from 8-bit (1 byte) to 16-bit (2 bytes, unsigned short, Little Endian):

- **Extended Metrics (UUID: 808a0d51-efae-4f0c-b2e0-48bc180d65c3)** now reports `dragFactor` as a 16-bit unsigned value rather than 8-bit.
- Old clients that read a single byte will misinterpret values or parse the payload incorrectly if the actual value is above 255. If the value stays below 256, compatibility is effectively preserved.

## 📌 Table of Contents

1. [Breaking Changes](#%EF%B8%8F-breaking-changes-700)
2. [Aim of the Project](#-aim-of-the-project)
3. [Key Features](#-key-features)
4. [Installation](#-installation)
5. [Settings](#%EF%B8%8F-settings)
6. [FAQ](#-faq)
7. [Technical Details](#%EF%B8%8F-technical-details)
8. [Backlog](#-backlog)
9. [Attribution](#-attribution)

## 🎯 Aim of the project

This project aims to provide the same accuracy of rowing analytics to indoor ergometers (e.g. rowing, kayak, canoe, SUP etc machines) machines as ORM. However, with a much more affordable, compact, widely available and potentially battery operated ESP32 microcontroller.

The math and algorithm used in ESP Rowing Monitor are based on the approach developed by @JaapvanEkris that improves the stroke detection algorithm of the original ORM.

The choice of the ESP32 over the Raspberry Pi (Rpi) due to its smaller size, its operability completely from battery, its very low cost, and easy accessibility.

There is a lot of information available on the physics and math behind the code in the repos related to ORM. Before using this project I recommend reading the documentation of ORM (specifically the docs provided currently under the [v1beta_updates branch](https://github.com/JaapvanEkris/openrowingmonitor/tree/v1beta_updates)) as this would greatly help with setting up and tuning in the settings of ESP Rowing Monitor.

## 🚀 Key features

_Highlights:_

- Very accurate stroke detection and a wide range of rowing metrics including force curve.
- Advanced cyclic error filtering for cleaner force curves and improved accuracy.
- BLE connectivity supporting multiple devices simultaneously.
- WebGUI for intuitive setup and data visualization.
- OTA firmware updates for seamless usage and detailed logging for replay.
- Calibration helper GUI for visual analysis of sensor data and stroke detection.

### Web interface

An intuitive [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/) is available that simplifies monitoring, configuration, and firmware updates, accessible directly through a browser. It's an installable Progressive Web App (PWA) that works offline after installation. It connects to ESP Rowing Monitor via bluetooth taking advantage the WebBluetooth stack. For further details please read the [documentation](https://github.com/Abasz/ESPRowingMonitor-WebGUI/).

### Metrics

Currently, ESP Rowing Monitor is capable of calculating the following metrics, similar to ORM:

- _driveDuration_ and _recoveryDuration_
- _average cycle power_
- _distance_
- _stroke rate_
- _handle forces for the drive phase_
- _peak force for the drive phase_
- _drag factor of the flywheel (i.e. resistance level)_
- _speed and pace_

ESP Rowing Monitor may not directly provide certain metrics such as caloric burn, heart rate, VO2max, etc. due to limitations of the device. These metrics require additional sensors or calculations that may not be supported by ESP Rowing Monitor's hardware or software capabilities. Users should refer to the [Limitations](docs/limitation.md#limitations) section for more detailed information on which metrics may not be available directly from ESP Rowing Monitor.

### Connectivity

ESP Rowing Monitor provides a BLE interface to get the data and metrics it measures.

It supports dual BLE connections, allowing simultaneous use of two clients, e.g. a smartwatch and the [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/) or other compatible apps (like Kinomap, EXR, etc.).

ESP Rowing Monitor supports three standard BLE profiles that allows it to be connected to smart watches and 3rd party apps and smart watches (fully compliant with BLE standards):

1. Cycling Speed and Cadence Sensor profile
2. Power Meter Sensor profile
3. Fitness Machine Rowing profile (currently experimental)

For further details how to set these up devices connected under Cycling Speed and Cadence profile or Power Meter profile please read the [features section](docs/features.md#bluetooth).

In addition to the implemented standard profiles, ESP Rowing Monitor exposes custom BLE services for additional metrics and tooling, fully supported by the official [WebGUI](https://abasz.github.io/ESPRowingMonitor-WebGUI/). These are:

1. Extended Metrics (metrics not included in the base profiles)
2. Handle Forces recorded during the last drive
3. Impulse time logging (logging of delta times for settings calibration)

Please see more details on their specifications and protocols under [Custom BLE Services](docs/custom-ble-services.md).

### Over-the-Air updates

As of version 6, after the initial installation, an over-the-air Bluetooth update protocol is available. The protocol is implemented in the WebGUI so firmware updates can be performed from there.

More details on the specification can be found in the [OTA protocol documentation](docs/custom-ble-services.md#over-the-air-updater)

### SD-Card impulse logging

It is possible to log deltaTimes (i.e. time between impulses) to an SD card (if connected and enabled). DeltaTimes are collected in a `vector` and written to SD card on every stroke (after the drive ends) or 4 seconds (which ever happens earlier). This incremental way of making deltaTimes available is to optimize performance.

## 📥 Installation

Please see the dedicated [installation page](docs/installation.md). Release assets include precompiled firmware, the ESPTool desktop flasher, and the calibration helper desktop GUI.

## ⚙️ Settings

Please see dedicated [settings page](docs/settings.md)

## ❓ FAQ

For common questions, troubleshooting guides, and community-proven solutions, see the [FAQ](docs/faq.md). The FAQ covers:

- Hardware compatibility and sensor selection
- Calibration workflow and flywheel inertia measurement
- Stroke detection tuning
- BLE connectivity and app compatibility
- Troubleshooting common issues

## 🛠️ Technical details

The monitor works by detecting the speed of the rotating flywheel via measuring the time between impulses through a reed or hall sensor on the rowing machine (more on the physics in the [ORM physics documentation](https://github.com/laberning/openrowingmonitor/tree/v1beta/docs/physics_openrowingmonitor.md)).

Please note that, for this monitor to function correctly, you need to measure the rotation speed of the flywheel rather than the handle speed. There are [several discussions](https://github.com/laberning/openrowingmonitor/discussions/95) on this topic under the ORM repos. It is possible that one can make it work but I have not tested such setup. I recommend reading those discussions to better understand the consequences of not measuring the flywheel speed.

### Impulse detection

All the metrics calculated are based on measuring the time between two consecutive impulses. Time is registered via an interrupt that is triggered by the reed/hall sensor connected to the ESP32 MCU. Basically, the ISR gets the current timestamp in microseconds and calculates the delta since the previous interrupt (as well as counts the number of times the ISR is triggered). This information is then fed into the stroke detection algorithm.

One advantage of the ESP32 ISR is that it is real-time (compared to ORM's polling strategy), which in theory would make this solution more accurate. However, testing showed that any deviation of the data produced by ORM and ESP Rowing Monitor is within the margin of error. So there is no real evidence that this added accuracy can be translated into an apparent improvement of the data quality. Actually, due to some noise filtering that ORM has, ORM may be a better choice for certain setups (mostly machines that produce quite some noise).

This project by default uses the same [Theil Sen Quadratic Regression](https://github.com/JaapvanEkris/openrowingmonitor/tree/main/docs/physics_openrowingmonitor.md#a-mathematical-perspective-on-key-metrics) model to determine torque as ORM, which is used as the main stroke detection algorithm.

Nevertheless, for certain machines (based on experience where only 1 or 2 impulses per rotation is present), the user can select the traditional stroke detection algorithm. There are three options in this respect:

1) the more advanced torque based (recommended for machines capable of producing several impulses per rotation),
2) the slope based (that is basically the traditional acceleration and deceleration base method), or
3) use both at the same time

Please note that due to the difference between the Rpi and the ESP32 (including but not limited to the CPU power, flash size, etc.), certain limitations and constraints apply to this project. Please see the [Limitations](docs/limitation.md#limitations) section for further details.

## 📋 Backlog

- Allow changing BLE device and model name dynamically

## 🙏 Attribution

- [Lars Berning](https://github.com/laberning/) - Original ORM implementation
- [Jaap van Ekris](https://github.com/JaapvanEkris) - Lots of help and explanation on the background and inner workings of the upgraded stroke detection algorithm
- [Double-A-92](https://github.com/Double-A-92) - For the Kettler Stroker rower profile
