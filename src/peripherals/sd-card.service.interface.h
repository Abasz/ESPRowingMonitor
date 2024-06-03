#pragma once

#include <vector>

class ISdCardService
{
protected:
    ~ISdCardService() = default;

public:
    ISdCardService() = default;

    virtual void setup() = 0;
    virtual void saveDeltaTime(const std::vector<unsigned long> &deltaTime) = 0;
    virtual bool isLogFileOpen() const = 0;
};