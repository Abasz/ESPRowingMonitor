class Settings
{
public:
    // Hardware settings
    static gpio_num_t const SENSOR_PIN_NUMBER = GPIO_NUM_26;
    static byte const IMPULSES_PER_REVOLUTION = 1;
    static double constexpr FLYWHEEL_INERTIA = 0.073;
    static unsigned short const LED_BLINK_FREQUENCY = 1000;

    // Sensor signal filter settings
    static byte const ROTATION_DEBOUNCE_TIME_MIN = 15;
    static unsigned short const ROWING_STOPPED_THRESHOLD_PERIOD = 7000;

    // Drag factor filter settings
    static byte const DRAG_FACTOR_ROTATION_DELTA_UPPER_THRESHOLD = 0;
    static double constexpr GOODNESS_OF_FIT_THRESHOLD = 0.9996;
    static unsigned short const MAX_DRAG_FACTOR_RECOVERY_PERIOD = 6000;
    static double constexpr LOWER_DRAG_FACTOR_THRESHOLD = 75 / 1e6;
    static double constexpr UPPER_DRAG_FACTOR_THRESHOLD = 250 / 1e6;
    static byte const DRAG_COEFFICIENTS_ARRAY_LENGTH = 1;

    // Stroke phase detection filter settings
    static unsigned short const MINIMUM_DECELERATION_DELTA_TO_UNPOWERED = 0;
    static unsigned short const MINIMUM_ACCELERATION_DELTA_TO_POWERED = 0;
    static byte const STROKE_DEBOUNCE_TIME = 200;
    static byte const FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD = 1;
    static byte const DELTA_TIME_ARRAY_LENGTH = 2;

    // Device power management settings
    static byte const VOLTAGE_DIVIDER_RATIO = 2;
    static double constexpr BATTERY_VOLTAGE_MIN = 3.3 / Settings::VOLTAGE_DIVIDER_RATIO;
    static double constexpr BATTERY_VOLTAGE_MAX = 4.00 / Settings::VOLTAGE_DIVIDER_RATIO;
    static byte const BATTERY_LEVEL_ARRAY_LENGTH = 5;
    static byte const INITIAL_BATTERY_LEVEL_MEASUREMENT_COUNT = 10;
    static unsigned int const BATTERY_MEASUREMENT_FREQUENCY = 10 * 60 * 1000;
    static unsigned long const DEEP_SLEEP_TIMEOUT = 4 * 60 * 1000;
};