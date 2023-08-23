// Copyright 2021 Zipline International Inc. All rights reserved.

#include "order.h"

#include <cassert>
#include <fstream>

#include "hospital.h"
#include "util.h"

namespace zipline
{
std::vector<Order> Order::LoadOrders(const std::filesystem::path &filename,
                                     const std::unordered_map<std::string, Hospital> &hospitals)
{
    std::ifstream stream{filename};

    std::vector<Order> orders{};

    Timestamp last_timestamp{0};
    std::string line{};
    while (std::getline(stream, line, '\n'))
    {
        auto tokens = ParseInputLine(line);
        assert(tokens.size() == 3 && "Got wrong number of order elements");

        Timestamp timestamp = atoi(tokens[0].c_str());
        assert(timestamp >= last_timestamp && "Found order timestamps in decreasing order");

        orders.emplace_back(Order{timestamp, hospitals.at(tokens[1]), StringToPriority(tokens[2])});

        last_timestamp = timestamp;
    }

    return orders;
}

Order::Priority Order::StringToPriority(const std::string &str)
{
    if (str == "Emergency")
    {
        return Order::Priority::kEmergency;
    }
    if (str == "Resupply")
    {
        return Order::Priority::kResupply;
    }

    return Order::Priority::kUnknown;
}

std::string Order::PriorityToString(Order::Priority priority)
{
    switch (priority)
    {
        case Order::Priority::kUnknown:
        {
            return "Unknown";
        }
        case Order::Priority::kResupply:
        {
            return "Resupply";
        }
        case Order::Priority::kEmergency:
        {
            return "Emergency";
        }
    }
    assert(0 && "Invalid string passed to PriorityToString");
    return {};
}
}  // namespace zipline
