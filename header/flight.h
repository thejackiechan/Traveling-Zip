// Copyright 2021 Zipline International Inc. All rights reserved.

#pragma once

#include <vector>

#include "order.h"
#include "util.h"

namespace zipline
{
class Flight
{
   public:
    Flight(Timestamp launch_time, const std::vector<Order> &orders) : launch_time_(launch_time), orders_(orders)
    {
    }

    // for debugging
    const std::vector<Order> &orders() const
    {
        return orders_;
    }

   private:
    Timestamp launch_time_;
    std::vector<Order> orders_;
};
}  // namespace zipline
