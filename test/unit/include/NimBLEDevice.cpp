#include "fakeit.hpp"

#include "NimBLEDevice.h"

fakeit::Mock<NimBLEServer> mockNimBLEServer;
fakeit::Mock<NimBLEAdvertising> mockNimBLEAdvertising;
fakeit::Mock<NimBLEService> mockNimBLEService;
fakeit::Mock<NimBLECharacteristic> mockNimBLECharacteristic;