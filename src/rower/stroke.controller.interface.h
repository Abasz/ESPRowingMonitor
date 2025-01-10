#include "../utils/configuration.h"
#include "./stroke.model.h"

class IStrokeController
{
protected:
    ~IStrokeController() = default;

public:
    virtual void begin() = 0;
    virtual void update() = 0;

    virtual const RowingDataModels::RowingMetrics &getAllData() const = 0;
    virtual unsigned int getPreviousRevCount() const = 0;
    virtual void setPreviousRevCount() = 0;
    virtual unsigned int getPreviousStrokeCount() const = 0;
    virtual void setPreviousStrokeCount() = 0;

    virtual unsigned long getPreviousRawImpulseCount() const = 0;
    virtual void setPreviousRawImpulseCount() = 0;
    virtual unsigned long getRawImpulseCount() const = 0;
    virtual unsigned long getLastImpulseTime() const = 0;

    virtual unsigned long getDeltaTime() const = 0;
    virtual unsigned long long getLastRevTime() const = 0;
    virtual unsigned int getRevCount() const = 0;
    virtual unsigned long long getLastStrokeTime() const = 0;
    virtual unsigned short getStrokeCount() const = 0;
    virtual Configurations::precision getDistance() const = 0;
    virtual Configurations::precision getRecoveryDuration() const = 0;
    virtual Configurations::precision getDriveDuration() const = 0;
    virtual short getAvgStrokePower() const = 0;
    virtual unsigned char getDragFactor() const = 0;
};