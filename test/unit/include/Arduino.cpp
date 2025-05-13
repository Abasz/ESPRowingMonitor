#include "fakeit.hpp"

#include "./Arduino.h"

fakeit::Mock<MockArduino> mockArduino;
fakeit::Mock<HardwareSerial> mockSerial;

HardwareSerial &Serial = mockSerial.get();
