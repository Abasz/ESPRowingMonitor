#pragma once

/// RGB color channel orderings, used when configuring LED strips to determine what order the controller should send data out in. The default ordering is RGB. Values are octal encoded positions.
enum class EOrder : unsigned char
{
    /// Red,   Green, Blue
    RGB = 0012,
    /// Red,   Blue,  Green
    RBG = 0021,
    /// Green, Red,   Blue
    GRB = 0102,
    /// Green, Blue,  Red
    GBR = 0120,
    /// Blue,  Red,   Green
    BRG = 0201,
    /// Blue,  Green, Red
    BGR = 0210
};

/// LED color codes using HTML/Web color values
enum class LedColor : unsigned int
{
    Black = 0x000000,
    Blue = 0x0000FF,
    Green = 0x008000,
    Red = 0xFF0000
};

enum class StrokeDetectionType : unsigned char
{
    Torque,
    Slope,
    Both
};

enum class CyclePhase : unsigned char
{
    Stopped,
    Recovery,
    Drive
};

enum class BleServiceFlag : unsigned char
{
    CpsService,
    CscService,
    FtmsService
};

enum class BaudRates : unsigned int
{
    Baud9600 = 9'600U,
    Baud14400 = 14'400U,
    Baud19200 = 19'200U,
    Baud33600 = 33'600U,
    Baud38400 = 38'400U,
    Baud56000 = 56'000U,
    Baud57600 = 57'600U,
    Baud76800 = 76'800U,
    Baud115200 = 115'200U,
    Baud128000 = 128'000U,
    Baud153600 = 153'600U,
    Baud230400 = 230'400U,
    Baud460800 = 460'800U,
    Baud921600 = 921'600U,
    Baud1500000 = 1'500'000U,
};
