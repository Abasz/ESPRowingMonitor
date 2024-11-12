// NOLINTBEGIN
#include "./fakeit.hpp"

static constexpr unsigned char labelLength = 17;
typedef struct
{
    unsigned char flash_chip;            /*!< SPI flash chip on which the partition resides */
    unsigned char type;                  /*!< partition type (app/data) */
    unsigned char subtype;               /*!< partition subtype */
    uint32_t address;                    /*!< starting address of the partition in flash */
    uint32_t size;                       /*!< size of the partition, in bytes */
    std::array<char, labelLength> label; /*!< partition label, zero-terminated ASCII string */
    bool encrypted;                      /*!< flag is set to true if partition is encrypted */
} esp_partition_t;

class MockOtaOps
{
public:
    virtual esp_partition_t *esp_ota_get_next_update_partition(const esp_partition_t *start_from) = 0;
    virtual esp_partition_t *esp_ota_get_running_partition() = 0;
};

extern fakeit::Mock<MockOtaOps> mockOtaOps;

inline esp_partition_t *esp_ota_get_next_update_partition(const esp_partition_t *start_from)
{
    return mockOtaOps.get().esp_ota_get_next_update_partition(start_from);
};

inline esp_partition_t *esp_ota_get_running_partition()
{
    return mockOtaOps.get().esp_ota_get_running_partition();
};
// NOLINTEND