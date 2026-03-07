#pragma once

#include <cstddef>
#include <vector>

#include "../configuration.h"

using std::size_t;

class Series
{
    unsigned char maxSeriesLength;
    unsigned short maxAllocationCapacity;
    Configurations::precision seriesSum = 0;
    std::vector<Configurations::precision> seriesArray;

public:
    constexpr explicit Series(
        const unsigned char _maxSeriesLength = 0,
        const unsigned short initialCapacity = Configurations::defaultAllocationCapacity,
        const unsigned short _maxAllocationCapacity = 1'000)
        : maxSeriesLength(_maxSeriesLength),
          maxAllocationCapacity(_maxAllocationCapacity)
    {
        seriesArray.reserve(_maxSeriesLength > 0 ? _maxSeriesLength : initialCapacity);
    }

    const Configurations::precision &operator[](size_t index) const;

    [[nodiscard]] Configurations::precision front() const;
    [[nodiscard]] Configurations::precision back() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t capacity() const;
    [[nodiscard]] Configurations::precision average() const;
    [[nodiscard]] Configurations::precision median() const;
    [[nodiscard]] Configurations::precision sum() const;

    void push(Configurations::precision value);
    void reset();
};
