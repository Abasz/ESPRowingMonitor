// NOLINTBEGIN(bugprone-throwing-static-initialization)
#include "fakeit.hpp"

#include "./Arduino.h"

fakeit::Mock<MockArduino> mockArduino;
fakeit::Mock<HardwareSerial> mockSerial;

HardwareSerial &Serial = mockSerial.get();
// NOLINTEND(bugprone-throwing-static-initialization)