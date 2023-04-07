# ESP32 Rowing Monitor

## Aim of the project

The purpose of this project is to provide a capabilities similar to the [Open Rowing Monitor Project](https://github.com/laberning/openrowingmonitor)(ORM) but with the ESP32 microcontroller. The math and algorithm used in ESP Rowing Monitor is based on the approach developed by JaapvanEkris that improves the stroke detection algorithm of the original ORM.

The choice to the ESP32 was made instead of the Rpi due to its smaller size, its operability completely from battery, its very low cost and easy accessibility.

There are a lot of information available on the physics and math behind the code in the repos related to ORM. And before using this project I recommend reading the documentation of ORM (specifically the docs provided currently under the [v1beta branch](https://github.com/laberning/openrowingmonitor/tree/v1beta)) as this would greatly help with setting up and tuning in the settings of ESP Rowing Monitor.

## Technical details

The way the monitor works is by detecting the speed of the rotating flywheel by measuring the time between impulses via a reed or hall sensor on the rowing machine (more on the physics [here](https://github.com/laberning/openrowingmonitor/blob/v1beta/docs/physics_openrowingmonitor.md)).

Please note that for this monitor to work, one really need to measure the rotation speed of the flywheel rather than the speed of the handle being pulled. There are [several discussions](https://github.com/laberning/openrowingmonitor/discussions/95) on this topic under the ORM repos. It is possible that one can make it work but I have not tested such setup. I recommend reading those discussions to better understand the consequences of not measuring the flywheel speed.

### Impulse detection

All the metrics calculated are based on measuring the time between two consecutive impulses. Time is registered via interrupt that is triggered by the reed/hall sensor connected to the ESP MCU. Basically the ISR gets the current time stamp in micro seconds and calculates the delta since the previous interrupt (as well as counts the number of times the ISR is triggered). This information is then fed into the stroke detection algorithm.

One advantage of the ESP32 ISR is that it is real time (compared to ORM's polling strategy), which in theory would make this solution more accurate. However testing showed that any deviation of the data produced by ORM and ESP Rowing Monitor is within the margin of error. So there is no real evidence that this added accuracy can be translated into apparent improvement of the data quality. Actually due to some noise filtering that ORM has may make ORM for certain setups a better choice (mostly machines that produce quite some noise).

This project uses the same [Theil Sen Quadratic Regression](https://github.com/laberning/openrowingmonitor/blob/v1beta/docs/physics_openrowingmonitor.md#a-mathematical-perspective-on-key-metrics) model to determine torque as ORM, which is used as the main stroke detection algorithm. This is supplement with a secondary fall back algorithm (which uses a  different approach compared to the way ORM tackles this). This secondary algorithm fits a linear regression curve to the calculated slopes of the recovery regression lines for every "flank" (i.e. it looks for the slope of slopes on every impulse). The slope of slopes calculated from the data points recorded within a "flank" (once there is no power provided to the flywheel by the rower), becomes flat (within a margin) as the deceleration of the flywheel becomes fairly constant.

![Recovery slopes chart](docs/imgs/recovery-slopes-chart.jpg)

The secondary algorithm looks for the moment when the slope of slopes flat lines (again, within a margin set by the user).

Please note that due to the difference between the Rpi and the ESP32 (including but not limited to the CPU power, flash size etc.) there are certain limitations and constrains apply to this project. Please see limitations section for further details.

## Features

ESP Rowing Monitor provides several ways to get the data and metrics it measures

### Bluetooth

ESP Rowing Monitor implements two BLE profiles:

1. Cycling Speed and Cadence Sensor profile
2. Power Meter Sensor profile

It is possible to switch via the WebGUI as well as via BLE Control point using specific OpCode (18). The implementation formally complies with BLE standards (control point characteristic) but code used are not in the standard.

Both BLE profiles are fully complying with BLE standards hence they are accessibly by compatible devices e.g. smart watches. Both profiles have been tested with Garmin smart watches (FR235, 645, 255).

Please note that for the Garmin watches to work one need to select a cycling activity, otherwise (due to the limitation of Garmin) the watch will connect but will not use the sensor data for metrics on the watch.

In order to get accurate speed and distance data the wheel circumference should be set to 10mm when pairing the device to ESP Rowing Monitor.

### Web interface

ESP Rowing Monitor has a websocket server that can send the calculated metrics to the connected clients (maximum of 2 at the time). Data is sent in JSON and follows the following structure:

```typescript
{
    driveDuration: number;
    recoveryDuration: number;
    batteryLevel: number;
    bleServiceFlag: BleServiceFlag;
    logLevel: LogLevel;
    revTime: number;
    distance: number;
    strokeTime: number;
    strokeCount: number;
    avgStrokePower: number;
    dragFactor: number;
    handleForces: Array<number>;
}
```

Due to resource constrains of the ESP32 MCU data is only sent on every stroke or in every 4 seconds (which ever occurs the earlier). This is not a significant issue perse as most of the metrics are only available at certain known states of the rowing cycle (e.g. end of drive, end of recovery etc.)

Currently a simple WebGUI is being developed using Angular. The related repo is located [here](https://github.com/Abasz/ESPRowingMonitor-WebGUI). Details how to use/install is under the readme of that repo.

### Logging

ESP Rowing Monitor implements a logging mechanism with different log levels (e.g. silent, error, info, trace, verbose, etc.). These are logged via serial (UART0) only so the ESP32 MCU should be connected via USB. The log level (0-6) can be set via the web socket or BLE control point with using OpCode 17.

Trace level logging is useful during the initial calibration as it prints the delta times that can be used to do replay. Please see for further details in [Calibration](docs/settings.md#calibration)

### Metrics

Currently ESP Rowing Monitor can calculate (similarly to ORM) the following metrics:

- _driveDuration_ and _recoveryDuration_
- _drive length (under development)_
- _average cycle power_
- _distance_
- _stroke rate_
- _handle forces for the drive phase_
- _peak force for the drive phase_
- _speed and pace_

Due to limitation some metrics will not be available directly from ESP Rowing Monitor (e.g. cal, heart rate vo2max etc.). Please see [Limitations](#limitations) section for further details.

## Settings

Please see dedicated [settings page](docs/settings.md)

## Installation

Please see dedicated [installation page](docs/installation.md)

## Limitations

There a certain limitations for this rowing monitor that should be noted. Please find these below

### CPU power and resource limitation of ESP32 chip

The ESP32 chips have 240Mhz, either single or a dual core. I recommend dual core (as the tests were done on dual core ESP32). Nevertheless, currently only the webserver runs on the secondary core.

The algorithm used for calculating the necessary data for stroke detection cab become rather CPU hungary. In a nutshell the issue is that the Theil-Sen Quadratic Regression is O(N&#178;) which means that as the size of the `IMPULSE_DATA_ARRAY_LENGTH` increases the time required to complete the calculations increase exponentially (for more information please see [this explanation](https://github.com/laberning/openrowingmonitor/blob/v1beta/docs/physics_openrowingmonitor.md#use-of-quadratic-theil-senn-regression-for-determining-%CE%B1-and-%CF%89-based-on-time-and-%CE%B8)).

I run some high-level tests and measured the execution times that is shown in the below table:

|IMPULSE_DATA_ARRAY_LENGTH|Execution time (us)|
|:-----------------------:|:----------------:|
|18                       |10319.02324       |
|15                       |7238.056122       |
|12                       |5032.933107       |
|9                        |3252.230726       |
|8                        |2656.080499       |
|7                        |2248.905896       |
|6                        |1903.043651       |
|5                        |1556.085034       |
|3                        |987.0181406       |

The above table shows that with an `IMPULSE_DATA_ARRAY_LENGTH` size of 18 the total execution time of the calculations on every impulse is more than 10ms. This could cause issues for instance when a new impulse comes in before the pervious calculation is finished. Due to the above currently the compiler does not allow `IMPULSE_DATA_ARRAY_LENGTH` size higher than 15 and gives a warning at 12.

As an example on my setup I use 3 impulses per rotation. Based on my experience the delta times cannot dip below 10ms. So with an `IMPULSE_DATA_ARRAY_LENGTH` size of 7 (execution time with double is approx. 2.2ms) this should be pretty much fine.

If for some reason testing shows that a higher value for the `IMPULSE_DATA_ARRAY_LENGTH` size is necessary the execution times can be reduced to some extent if instead of doubles floats are used:

|IMPULSE_DATA_ARRAY_LENGTH|Execution time (us)|
|:-----------------------:|:-----------------:|
|18                       |6943.947846        |
|15                       |5101.781746        |
|12                       |3706.609977        |
|9                        |2543.750567        |
|8                        |2152.922336        |
|7                        |1848.677438        |
|6                        |1602.282313        |
|5                        |1350.632653        |
|3                        |899.649093         |

Using float precision instead of double precision of course reduces the precision but shaves off on the execution times. I have not run extensive testing on this but for the limited simulations I run this did not make a significant difference.

The below picture shows that the blue chart cuts some corners but generally follows the same curve (which does not mean that in certain edge cases the reduced precision does not create errors).

![Float vs. Double](docs/imgs/float-vs-double.jpg)

Another limitation that relates to CPU speed is the peripherals (BLE and the webserver). The refresh rate of both are limited to save resources (WS only updates on new stroke or after 4 second if no new stroke is made within that period).

I will need to run more tests if this frequency can be increased (e.g. on every full rotation), but this would probably only be feasible on dual core versions.

### Noise filtering

Unlike ORM ESP Rowing Monitor has only one impulse noise filter and that is a minimum required time between impulses. This means that the hardware used should produce a clean impulse that implies that read switches (due to their debounce) may not produce good results. Exception may be if the impulses per revolution is low (e.g. 1) and hence the minimum time between impulses can be sufficiently high.

Please see [Sensor signal filter settings](docs/settings.md#sensor-signal-filter-settings) for more details.

### BLE and connectivity

ESP Rowing Monitor will not be able to support heart rate monitors. It would bo probably possible to add BLE heart rate monitor capabilities on the Web GUI side (e.g. via the browser BLE API) but not directly to the monitor it self. While technically it may be possible it does not really makes much sense.

ESP Rowing Monitor is unlikely to get ANT+ support. While an appropriate ANT+ library for the Arduino framework exists I had no success finding and setting up the hardware (the ANT+ radio). I tested several chips I was not able to load the ANT network processor firmware so I gave up on it.

ESP Rowing Monitor exposes BLE Cycling Power Profile and Cycling Speed and Cadence Profile which is a slight hack. As mentioned before the wheel circumference should be set to 10mm in order for clients to show correct metrics (please see [this](docs/settings.md#) for more information). Also for instance in case of Garmin watches the data from these profiles are only show up if the activity is cycling (so with Garmin rowing activity profile this does not work). What I currently do with my Garmin FR255 is that I change the activity type manually.

## Backlog

- Make WebGUI served up by ESP Rowing Monitor - In progress
- Need to improve and extend validation of settings on compiling
- Enable disabling peripherals with compiler flags

## Attribution

[Lars Berning](https://github.com/laberning/) - Original ORM implementation
[Jaap van Ekris](https://github.com/JaapvanEkris) - Lots of help and explanation on the background and inner workings of the upgraded stroke detection algorithm
