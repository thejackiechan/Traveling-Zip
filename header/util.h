// Copyright 2021 Zipline International Inc. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cmath>

namespace zipline
{
using Timestamp = int32_t;

std::vector<std::string> ParseInputLine(const std::string &line);

// Helper utility function
inline float GetDistanceBetweenPoints(const int p1_x, const int p1_y, const int p2_x, const int p2_y)
{
    return std::hypot(p2_x - p1_x, p2_y - p1_y);
}

}  // namespace zipline
