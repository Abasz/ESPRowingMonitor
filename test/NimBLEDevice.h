
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
