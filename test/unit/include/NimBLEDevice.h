// NOLINTBEGIN
#pragma once

#include <string>
#include <vector>

#include "./fakeit.hpp"

/**
 * @brief Bluetooth TX power level(index), it's just a index corresponding to power(dbm).
 */
typedef enum
{
    ESP_PWR_LVL_N12 = 0,               /*!< Corresponding to -12dbm */
    ESP_PWR_LVL_N9 = 1,                /*!< Corresponding to  -9dbm */
    ESP_PWR_LVL_N6 = 2,                /*!< Corresponding to  -6dbm */
    ESP_PWR_LVL_N3 = 3,                /*!< Corresponding to  -3dbm */
    ESP_PWR_LVL_N0 = 4,                /*!< Corresponding to   0dbm */
    ESP_PWR_LVL_P3 = 5,                /*!< Corresponding to  +3dbm */
    ESP_PWR_LVL_P6 = 6,                /*!< Corresponding to  +6dbm */
    ESP_PWR_LVL_P9 = 7,                /*!< Corresponding to  +9dbm */
    ESP_PWR_LVL_N14 = ESP_PWR_LVL_N12, /*!< Backward compatibility! Setting to -14dbm will actually result to -12dbm */
    ESP_PWR_LVL_N11 = ESP_PWR_LVL_N9,  /*!< Backward compatibility! Setting to -11dbm will actually result to  -9dbm */
    ESP_PWR_LVL_N8 = ESP_PWR_LVL_N6,   /*!< Backward compatibility! Setting to  -8dbm will actually result to  -6dbm */
    ESP_PWR_LVL_N5 = ESP_PWR_LVL_N3,   /*!< Backward compatibility! Setting to  -5dbm will actually result to  -3dbm */
    ESP_PWR_LVL_N2 = ESP_PWR_LVL_N0,   /*!< Backward compatibility! Setting to  -2dbm will actually result to   0dbm */
    ESP_PWR_LVL_P1 = ESP_PWR_LVL_P3,   /*!< Backward compatibility! Setting to  +1dbm will actually result to  +3dbm */
    ESP_PWR_LVL_P4 = ESP_PWR_LVL_P6,   /*!< Backward compatibility! Setting to  +4dbm will actually result to  +6dbm */
    ESP_PWR_LVL_P7 = ESP_PWR_LVL_P9,   /*!< Backward compatibility! Setting to  +7dbm will actually result to  +9dbm */
} esp_power_level_t;

typedef enum
{
    ESP_BLE_PWR_TYPE_CONN_HDL0 = 0, /*!< For connection handle 0 */
    ESP_BLE_PWR_TYPE_CONN_HDL1 = 1, /*!< For connection handle 1 */
    ESP_BLE_PWR_TYPE_CONN_HDL2 = 2, /*!< For connection handle 2 */
    ESP_BLE_PWR_TYPE_CONN_HDL3 = 3, /*!< For connection handle 3 */
    ESP_BLE_PWR_TYPE_CONN_HDL4 = 4, /*!< For connection handle 4 */
    ESP_BLE_PWR_TYPE_CONN_HDL5 = 5, /*!< For connection handle 5 */
    ESP_BLE_PWR_TYPE_CONN_HDL6 = 6, /*!< For connection handle 6 */
    ESP_BLE_PWR_TYPE_CONN_HDL7 = 7, /*!< For connection handle 7 */
    ESP_BLE_PWR_TYPE_CONN_HDL8 = 8, /*!< For connection handle 8 */
    ESP_BLE_PWR_TYPE_ADV = 9,       /*!< For advertising */
    ESP_BLE_PWR_TYPE_SCAN = 10,     /*!< For scan */
    ESP_BLE_PWR_TYPE_DEFAULT = 11,  /*!< For default, if not set other, it will use default value */
    ESP_BLE_PWR_TYPE_NUM = 12,      /*!< TYPE numbers */
} esp_ble_power_type_t;

struct ble_gap_conn_desc
{
    unsigned short conn_handle;
};

typedef enum
{
    READ = 0x0002,
    WRITE = 0x0008,
    NOTIFY = 0x0010,
    INDICATE = 0x0020,
} NIMBLE_PROPERTY;

class NimBLEAttValue
{
private:
    std::vector<unsigned char> data;

public:
    NimBLEAttValue(std::initializer_list<unsigned char> initList)
    {
        data.assign(cbegin(initList), cend(initList));
    }

    size_t size() const
    {
        return data.size();
    }

    const unsigned char *begin() const
    {
        return data.data();
    }

    const unsigned char *end() const
    {
        return &data.back() + 1;
    }

    const unsigned char &operator[](size_t index) const
    {
        return data[index];
    }

    void push_back(unsigned char newElement)
    {
        data.push_back(newElement);
    }

    void insert(std::vector<unsigned char> &newRange)
    {
        data.insert(std::cend(data), cbegin(newRange), cend(newRange));
    }
};

class NimBLEUUID
{
public:
    std::string uuid;

    NimBLEUUID(const std::string &_uuid)
    {
        uuid = _uuid;
    };
    NimBLEUUID(unsigned short _uuid)
    {
        uuid = std::to_string(_uuid);
    };
    NimBLEUUID(unsigned int _uuid)
    {
        uuid = std::to_string(_uuid);
    };

    std::string toString() const
    {
        return uuid;
    };
};

class NimBLEService;
class NimBLEServer;
class NimBLECharacteristic;

class NimBLEServerCallbacks
{
public:
    virtual void onConnect(NimBLEServer *pServer) {};
    virtual void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) {};
};

class NimBLECharacteristicCallbacks
{
public:
    virtual void onWrite(NimBLECharacteristic *pCharacteristic) {};
    virtual void onWrite(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc) {};
    virtual void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, unsigned short subValue) {};
};

class NimBLEServer
{
public:
    NimBLEServerCallbacks *callbacks;

    virtual void createServer() = 0;

    virtual void start() = 0;
    virtual NimBLEService *createService(const std::string uuid) = 0;
    virtual NimBLEService *createService(const unsigned short uuid) = 0;
    virtual size_t getConnectedCount() = 0;
    virtual void init(const std::string deviceName) = 0;
    virtual void setPower(esp_power_level_t powerLevel, esp_ble_power_type_t powerType) = 0;
    void setCallbacks(NimBLEServerCallbacks *pCallbacks)
    {
        callbacks = pCallbacks;
    };
    virtual unsigned short getPeerMTU(unsigned short conn_id) = 0;
};
extern fakeit::Mock<NimBLEServer> mockNimBLEServer;

class NimBLECharacteristic
{
public:
    NimBLECharacteristicCallbacks *callbacks;

    virtual NimBLEAttValue getValue() = 0;

    virtual void setValue(const unsigned short s) = 0;
    virtual void setValue(const unsigned char *data, size_t length) = 0;
    virtual void setValue(const std::string s) = 0;
    virtual void setValue(const std::array<unsigned char, 1U> s) = 0;
    virtual void setValue(const std::array<unsigned char, 3U> s) = 0;
    virtual void setValue(const std::array<unsigned char, 7U> s) = 0;
    virtual void setValue(const std::array<unsigned char, 11U> s) = 0;
    virtual void setValue(const std::array<unsigned char, 14U> s) = 0;
    virtual void setCallbacks(NimBLECharacteristicCallbacks *pCallbacks)
    {
        callbacks = pCallbacks;
    };
    virtual void notify() = 0;
    virtual void indicate() = 0;

    virtual size_t getSubscribedCount() = 0;
    virtual NimBLEService *getService() = 0;
    virtual NimBLEUUID getUUID() = 0;
};
extern fakeit::Mock<NimBLECharacteristic> mockNimBLECharacteristic;

class NimBLEService
{
public:
    virtual NimBLECharacteristic *createCharacteristic(const unsigned short uuid, unsigned int properties) = 0;
    virtual NimBLECharacteristic *createCharacteristic(const std::string uuid, unsigned int properties) = 0;
    virtual NimBLEServer *getServer() = 0;
    virtual bool start() = 0;
};
extern fakeit::Mock<NimBLEService> mockNimBLEService;

class NimBLEAdvertising
{
public:
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual void setAppearance(unsigned short appearance) = 0;
    virtual void addServiceUUID(unsigned short uuid) = 0;
};
extern fakeit::Mock<NimBLEAdvertising> mockNimBLEAdvertising;

class NimBLEDevice
{
public:
    static NimBLEServer *createServer()
    {
        mockNimBLEServer.get().createServer();

        return &mockNimBLEServer.get();
    }
    static NimBLEServer *getServer()
    {
        return &mockNimBLEServer.get();
    }

    static NimBLEAdvertising *getAdvertising()
    {
        return &mockNimBLEAdvertising.get();
    }

    static void init(const std::string deviceName)
    {
        mockNimBLEServer.get().init(deviceName);
    }

    static void setPower(esp_power_level_t powerLevel, esp_ble_power_type_t powerType)
    {
        mockNimBLEServer.get().setPower(powerLevel, powerType);
    }
};
// NOLINTEND
