// Copyright 2021 Zipline International Inc. All rights reserved.

#include <cassert>
#include <iostream>

#include "hospital.h"
#include "order.h"
#include "zip_scheduler.h"

namespace
{
// Moved kMaxRange, kMaxPackages, kZipSpeed to zip_scheduler.h
// constexpr auto kMaxRange = 160 * 1000;  // (meters) 160 km
// constexpr auto kMaxPackages = 3;
// constexpr auto kZipSpeed = 30;         // (meters per second)
constexpr auto kTimeBetweenLaunches = 60;  // (seconds)
constexpr auto kSecondsPerDay = 24 * 60 * 60;
constexpr auto kNumZips = 10;
}  // namespace

using zipline::Hospital;
using zipline::Order;
using zipline::Timestamp;

int main()
{
    auto hospitals = Hospital::LoadHospitals("../inputs/hospitals.csv");
    for (const auto &[hospital_name, hospital] : hospitals)
    {
        assert(hospital_name == hospital.name());
        std::cout << hospital.name() << " " << hospital.north() << " " << hospital.east() << std::endl;
    }

    auto orders = Order::LoadOrders("../inputs/orders.csv", hospitals);

    zipline::ZipScheduler scheduler{};
    scheduler.InitializeZips(kNumZips);

    size_t order_idx = 0;
    const auto num_orders = orders.size();

    auto first_timestamp = orders[order_idx].received_time();

    for (Timestamp cur_time{first_timestamp}; cur_time < kSecondsPerDay; cur_time++)
    {
        while (order_idx < num_orders && orders[order_idx].received_time() <= cur_time)
        {
            scheduler.QueueOrder(orders[order_idx]);
            order_idx++;
        }
        if (cur_time % kTimeBetweenLaunches == 0)
        {
            auto flights = scheduler.LaunchFlights(cur_time);
            assert(flights.size() <= kNumZips);
        }
    }

    return 0;
}
