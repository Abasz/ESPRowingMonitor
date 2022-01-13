#include <array>

#include "stroke.model.h"

enum class CyclePhase
{
    Stopped,
    Recovery,
    Drive
};

class StrokeService
{
    static unsigned char const ROTATION_DEBOUNCE_TIME_MIN = 10;
    static unsigned short const ROTATION_DEBOUNCE_TIME_MAX = 350;
    static unsigned char const STROKE_DEBOUNCE_TIME = 200;
    static unsigned short const MAX_DRAG_FACTOR_RECOVERY_PERIOD = 6000;
    static unsigned char const FLYWHEEL_POWER_CHANGE_DETECTION_THRESHOLD = 1;
    static unsigned char const DELTA_TIME_ARRAY_LENGTH = 2;

    volatile unsigned long lastRevTime = 0;
    volatile unsigned long lastStrokeTime = 0;
    volatile unsigned int revCount = 0;
    volatile unsigned short strokeCount = 0;
    volatile double dragFactor = 0.0;
    volatile double recoveryStartAngularVelocity = 0.0;

    volatile unsigned long drivePhaseStartTime = 0;
    volatile unsigned long recoveryPhaseStartTime = 0;
    volatile unsigned int drivePhaseDuration = 0;
    volatile unsigned int recoveryPhaseDuration = 0;
    volatile unsigned long previousDeltaTime = 0;
    volatile unsigned long previousRawRevTime = 0;

    volatile CyclePhase cyclePhase = CyclePhase::Stopped;
    std::array<volatile unsigned long, DELTA_TIME_ARRAY_LENGTH> cleanDeltaTimes{{0UL}};

    bool isFlywheelUnpowered();
    bool isFlywheelPowered();

public:
    StrokeService();

    StrokeModel::CscData getCscData() const;
    unsigned long getLastRevReadTime() const;
    void setLastRevReadTime();
    unsigned long getLastRevTime() const;
    void processRotation(unsigned long now);
};