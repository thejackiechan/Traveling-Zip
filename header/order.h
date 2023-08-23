// Copyright 2021 Zipline International Inc. All rights reserved.

#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "hospital.h"
#include "util.h"

namespace zipline
{
class Order
{
   public:
    enum class Priority
    {
        kUnknown = 0,
        kResupply,
        kEmergency,
    };

    static Priority StringToPriority(const std::string &str);
    static std::string PriorityToString(Priority priority);

    Order(Timestamp received_time, const Hospital &hospital, Priority priority)
        : received_time_(received_time), hospital_(hospital), priority_(priority)
    {
    }
    static std::vector<Order> LoadOrders(const std::filesystem::path &filename,
                                         const std::unordered_map<std::string, Hospital> &hospitals);

    Timestamp received_time() const
    {
        return received_time_;
    }

    Priority priority() const
    {
        return priority_;
    }

    const Hospital &hospital() const
    {
        return hospital_;
    }

   private:
    Timestamp received_time_{0};
    Hospital hospital_;
    Priority priority_{Priority::kUnknown};
};

}  // namespace zipline
