#ESP32 - BLE Cycling Speed and Cadence/Power Meter Sensor for Rowing Machines

This project is for ESP32 based rotation sensor that may be used for a Rowing Machine (or anything that has a rotating flywheel, the speed of which can be measured). The sensor code implements BLE Cycling Speed and Cadence Sensor Profile and Cycling Power Meter Profile for connecting smart watches or any other device.

It measures stroke rate from the acceleration and deceleration of the flywheel as well as calculates power and other data necessary for a client to calculate speed and cadence.

For the sensor to properly work, some flywheel specific settings needs to be changed in the settings.h file (e.g. number of impulses per rotation, inertia of the flywheel, etc.)

This project may be considered as a simplified port to a ESP32 micro controller of the [Open Rowing Monitor Project](https://github.com/JaapvanEkris/openrowingmonitor). There are a lot of information available on the physics and math behind the code. Its important to note that Open Rowing Monitor has a much more robust rotation impulse filtering (i.e. it is more capable to process less accurate rotation impulses).
