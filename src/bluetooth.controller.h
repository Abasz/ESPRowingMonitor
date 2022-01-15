#include "bluetooth.service.h"

class BluetoothController
{
    BluetoothService const bluetoothService;

public:
    BluetoothController();
    void begin() const;
    void setBattery(unsigned char batteryLevel) const;
    void notifyCsc(unsigned long lastRevTime, unsigned int revCount, unsigned long lastStrokeTime, unsigned short strokeCount) const;
    void notifyDragFactor(unsigned char dragFactor) const;
    bool isDeviceConnected() const;
};
