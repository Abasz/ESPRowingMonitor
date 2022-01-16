#ESP32 - BLE Cycling and Cadence Sensor for Rowing Machines

This project is for ESP32 based rotation sensor that may be used for a Rowing Machine (or anything that has a rotating flywheel, the speed of which can be measured). The sensor code implements BLE Cycling and Cadence Sensor Profile for connecting smart watches.

It uses special filter to measure stroke rate (by detecting the acceleration, when powered by the paddler, and slight deceleration of the flywheel following the stroke).

