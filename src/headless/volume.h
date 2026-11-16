// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include <algorithm>
#include <cmath>

namespace Volume
{
inline constexpr double minDb = -60.0;

inline double toMpv(double value)
{
    if (value <= 0)
        return 0;
    const double db = minDb * (100.0 - std::clamp(value, 1.0, 100.0)) / 99.0;
    return 100.0 * std::pow(10.0, db / 60.0);
}

inline double fromMpv(double value)
{
    if (value <= 0)
        return 0;
    const double db = 60.0 * std::log10(value / 100.0);
    return std::clamp(100.0 - 99.0 * db / minDb, 1.0, 100.0);
}
}
