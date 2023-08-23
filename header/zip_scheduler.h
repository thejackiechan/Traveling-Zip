// Copyright 2021 Zipline International Inc. All rights reserved.
#pragma once

#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

#include <functional>
#include "flight.h"
#include "order.h"
#include "util.h"

namespace
{
constexpr auto kMinResupplyOrders = 5;
constexpr auto kMaxResupplyWaitTime = 15 * 60;  // 15 min in seconds
constexpr auto kMaxRange = 160 * 1000;          // (meters) 160 km
constexpr auto kMaxPackages = 3;
constexpr auto kZipSpeed = 30;                     // (meters per second)
constexpr auto kRangeReducedRange = 45 * 60 * 30;  // 45 min * 60 s/min * 30 m/s
constexpr auto kNumZipsReducedRange = 2;           // 2 drones guaranteed to return within 45 min in resupply time
}  // namespace

namespace zipline
{

class ZipScheduler
{
   public:
    void InitializeZips(const int num_zips);

    // Add an order to the queue to potentially launch at the next time LaunchFlights is called.
    void QueueOrder(const Order &order);

    // Returns an ordered list of flights to launch.
    std::vector<Flight> LaunchFlights(Timestamp current_time);

   private:
    std::vector<Timestamp> zip_return_times_;
    std::vector<std::shared_ptr<Order>> emergency_orders_;
    std::vector<std::shared_ptr<Order>> resupply_orders_;

    std::vector<int> GetFreeZips(const Timestamp curr_time);
    std::shared_ptr<Order> GetNextOrderByDist(int &curr_x, int &curr_y, int &curr_dist, int &return_dist,
                                              const int max_range);
    std::shared_ptr<Order> GetNextOrderInQueue(int &curr_x, int &curr_y, int &curr_dist, int &return_dist,
                                               const int max_range, std::vector<std::shared_ptr<Order>> &orders);
    void ScheduleFlights(const int zip_idx, std::vector<Flight> &flights, const int max_range, const int curr_time);
    std::shared_ptr<Order> GetFirstOrder(int &curr_x, int &curr_y, int &curr_dist, int &return_dist,
                        std::vector<std::shared_ptr<Order>> &orders);
};

}  // namespace zipline
