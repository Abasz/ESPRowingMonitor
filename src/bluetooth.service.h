#include <array>

class BluetoothService
{
    std::array<uint8_t, 2> const FEATURES_FLAG = {{0b11, 0b0}};

    unsigned short const CYCLING_SPEED_CADENCE_SVC_UUID = 0x1816;
    unsigned short const CSC_MEASUREMENT_UUID = 0x2A5B;
    unsigned short const SC_CONTROL_POINT_UUID = 0x2A55;
    unsigned short const CSC_FEATURE_UUID = 0x2A5C;
    unsigned short const SENSOR_LOCATION_UUID = 0x2A5D;

    unsigned short const BATTERY_SVC_UUID = 0x180F;
    unsigned short const BATTERY_LEVEL_UUID = 0x2A19;

    unsigned short const DEVICE_INFO_SVC_UUID = 0x180A;
    unsigned short const MODEL_NUMBER_SVC_UUID = 0x2A24;
    unsigned short const SERIAL_NUMBER_SVC_UUID = 0x2A25;
    unsigned short const SOFTWARE_NUMBER_SVC_UUID = 0x2A28;
    unsigned short const MANUFACTURER_NAME_SVC_UUID = 0x2A29;

    unsigned short const NOTIFICATION_DESCRIPTOR_UUID = 0x2902;

    unsigned short const BLE_APPEARANCE_CYCLING_SPEED_CADENCE = 1157;

    // TODO: Descide how dragfactor and distance setting should be reported? Using PM5 UUID or implementing custom one
    // PM5 General Status Characteristic UUID `ce06003143e511e4916c0800200c9a66` that includes dragFactor

    void setupServices() const;
    void setupAdvertisment() const;

public:
    BluetoothService();

    void setup() const;
    void startBLEServer() const;
    void stopServer() const;
    void setBattery(unsigned char batteryLevel) const;
    void notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const;
    void notifyDragFactor(unsigned char distance, unsigned char dragFactor) const;
};
