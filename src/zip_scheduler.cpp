// Copyright 2021 Zipline International Inc. All rights reserved.

#include "zip_scheduler.h"

namespace zipline
{

void ZipScheduler::InitializeZips(const int num_zips)
{
    for (int i = 0; i < num_zips; ++i) zip_return_times_.push_back(0);
}

void ZipScheduler::QueueOrder(const Order &order)
{
    std::cout << "Queuing order:\n\t" << order.received_time() << "\n\t" << order.hospital().name() << "\n\t"
              << Order::PriorityToString(order.priority()) << std::endl;

    std::shared_ptr<Order> order_ptr = std::make_shared<Order>(order);
    if (order.priority() == Order::Priority::kEmergency)
    {
        emergency_orders_.push_back(order_ptr);
    }
    else
    {
        resupply_orders_.push_back(order_ptr);
    }
}

std::vector<Flight> ZipScheduler::LaunchFlights(Timestamp current_time)
{
    std::cout << "Asking for flights at " << current_time << std::endl;
    std::vector<Flight> flights;
    // compute the number of available zips
    std::vector<int> free_zip_indexes = GetFreeZips(current_time);

    if (!free_zip_indexes.empty())
    {
        size_t zip_idx_itr = 0;
        // prioritize emergency orders
        while (!emergency_orders_.empty() && zip_idx_itr < free_zip_indexes.size())
        {
            ScheduleFlights(free_zip_indexes[zip_idx_itr], flights, kMaxRange, current_time);
            zip_idx_itr++;
        }
        if (zip_idx_itr < free_zip_indexes.size())
        {
            // deploy only one zip at a time for just resupply orders
            // wait until 5 resupply orders in queue or 15 min, whichever comes first
            if (resupply_orders_.size() >= kMinResupplyOrders ||
                (!resupply_orders_.empty() &&
                 resupply_orders_.front()->received_time() + kMaxResupplyWaitTime <= current_time))
            {
                std::unordered_set<size_t> zip_idx_reduced_range;  // indices 8, 9
                for (size_t i = zip_return_times_.size() - 1; i >= zip_return_times_.size() - kNumZipsReducedRange; --i)
                {
                    zip_idx_reduced_range.insert(i);
                }

                size_t zip_idx = free_zip_indexes[zip_idx_itr];
                // if zip 8 or 9, keep flight to 45 min round trip to have them back sooner for any incoming emergencies
                if (zip_idx_reduced_range.find(zip_idx) != zip_idx_reduced_range.end())
                {
                    ScheduleFlights(zip_idx, flights, kRangeReducedRange, current_time);
                }
                else  // other zips can use the max range
                {
                    ScheduleFlights(zip_idx, flights, kMaxRange, current_time);
                }
            }
        }
    }

    // print flights
    for (size_t i = 0; i < flights.size(); ++i)
    {
        std::cout << "Flight " << i + 1 << std::endl;
        for (Order order : flights[i].orders())
        {
            std::cout << "Executing order:\n\t" << order.received_time() << "\n\t" << order.hospital().name() << "\n\t"
                      << Order::PriorityToString(order.priority()) << std::endl
                      << std::endl;
        }
    }

    std::cout << "Number of available zips: " << free_zip_indexes.size() << std::endl;
    std::cout << "Number of zips taking off: " << flights.size() << std::endl;
    assert(free_zip_indexes.size() >= flights.size());
    std::cout << "Number of emergency orders remaining: " << emergency_orders_.size() << std::endl;
    std::cout << "Number of resupply orders remaining: " << resupply_orders_.size() << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    return flights;
}

// Helper functions
// Returns the indices (0-9) of zips available to deliver orders by checking
// the zips' return time against the current time.
std::vector<int> ZipScheduler::GetFreeZips(const Timestamp curr_time)
{
    std::vector<int> zip_indexes;

    for (size_t i = 0; i < zip_return_times_.size(); ++i)
    {
        if (zip_return_times_[i] <= curr_time) zip_indexes.push_back(i);
    }
    return zip_indexes;
}

/*
    Description: Performs order scheduling for a zip by maximizing the amount of packages it can deliver
                 within its maximum range. After adding the first emergency or resupply order (based on
                 received time), it searches through other orders (prioritizing emergency ones) and adding
                 those closest to the current order location.
    Arguments:
        - zip_idx: index of the zip for which the flight is being scheduled for
        - flights: vector of flights that the computed flight will be appended to
        - max_range: max distance for the zip (either reduced or max)
        - curr_time: the launch time for the flight
*/
void ZipScheduler::ScheduleFlights(const int zip_idx, std::vector<Flight> &flights, const int max_range,
                                   const int curr_time)
{
    std::vector<Order> orders;
    int curr_dist(0);
    int return_dist(0);
    int curr_x(0);
    int curr_y(0);

    // get first order in emergency (priority) or resupply queue
    if (!emergency_orders_.empty())
    {
        orders.push_back(*GetFirstOrder(curr_x, curr_y, curr_dist, return_dist, emergency_orders_));
    }
    else if (!resupply_orders_.empty())
    {
        orders.push_back(*GetFirstOrder(curr_x, curr_y, curr_dist, return_dist, resupply_orders_));
    }
    else  // no emergency and resupply orders
    {
        return;
    }

    // add any nearby emergency/resupply orders nearby up to range and package capacity
    while (curr_dist < max_range && orders.size() < kMaxPackages)
    {
        std::shared_ptr<Order> order_ptr = GetNextOrderByDist(curr_x, curr_y, curr_dist, return_dist, max_range);
        if (order_ptr)
        {
            orders.push_back(*order_ptr);
        }
        else
        {
            break;
        }
    }
    curr_dist += return_dist;  // compute total distance of orders
    zip_return_times_[zip_idx] = curr_time + ceil((float)curr_dist / kZipSpeed);
    flights.push_back(Flight(curr_time, orders));
}

// Returns the first order in orders and updates the necessary parameters.
std::shared_ptr<Order> ZipScheduler::GetFirstOrder(int &curr_x, int &curr_y, int &curr_dist, int &return_dist,
                                                   std::vector<std::shared_ptr<Order>> &orders)
{
    std::shared_ptr<Order> order = orders.front();
    curr_dist = GetDistanceBetweenPoints(curr_x, curr_y, order->hospital().east(), order->hospital().north());
    return_dist = curr_dist;
    curr_x = order->hospital().east();
    curr_y = order->hospital().north();
    orders.erase(orders.begin());
    return order;
}

/*
    Description: Gets the order location (hospital) that is closest to the current one, ensuring zip can both reach and
                 return to nest from there. Prioritizes emergency orders before resupply ones.
    Arguments:
        - curr_x: east coordinate of the hospital associated with the current order
        - curr_y: north coordinate of the hospital associated with the current order
        - curr_dist: running total of the distance to deliver all of the orders
        - return_dist: distance from the last order location to nest
        - max_range: max distance for zip
    Returns: Pointer to the nearest order or nullptr if none is within range.
*/
std::shared_ptr<Order> ZipScheduler::GetNextOrderByDist(int &curr_x, int &curr_y, int &curr_dist, int &return_dist,
                                                        const int max_range)
{
    std::shared_ptr<Order> min_order_ptr =
        GetNextOrderInQueue(curr_x, curr_y, curr_dist, return_dist, max_range, emergency_orders_);
    if (!min_order_ptr)
        min_order_ptr = GetNextOrderInQueue(curr_x, curr_y, curr_dist, return_dist, max_range, resupply_orders_);
    return min_order_ptr;
}

/*
    Description: Gets the order location (hospital) that is closest to the current one, ensuring zip can both reach and
                 return to nest from there.
    Arguments:
        - curr_x: east coordinate of the hospital associated with the current order
        - curr_y: north coordinate of the hospital associated with the current order
        - curr_dist: running total of the distance to deliver all of the orders
        - return_dist: distance from the last order location to nest
        - max_range: max distance for zip
        - orders: vector of emergency or resupply orders to search through
    Returns: Pointer to the nearest order or nullptr if none is within range.
*/
std::shared_ptr<Order> ZipScheduler::GetNextOrderInQueue(int &curr_x, int &curr_y, int &curr_dist, int &return_dist,
                                                         const int max_range,
                                                         std::vector<std::shared_ptr<Order>> &orders)
{
    if (orders.empty()) return nullptr;  // return null when there are no emergency orders
    int min_dist(INT_MAX);
    int min_idx(-1);

    // find idx of next order with the closest hospital to (curr_x, curr_y)
    for (size_t i = 0; i < orders.size(); ++i)
    {
        Hospital hospital = orders[i]->hospital();
        int dist_to_next = GetDistanceBetweenPoints(curr_x, curr_y, hospital.east(), hospital.north());
        if (dist_to_next < min_dist)
        {
            min_dist = dist_to_next;  // distance from curr hospital to next
            min_idx = i;
        }
    }

    std::shared_ptr<Order> min_order = orders[min_idx];
    int next_x(min_order->hospital().east());
    int next_y(min_order->hospital().north());
    // compute distance from hospital of next order to nest
    int dist_next_to_nest = GetDistanceBetweenPoints(next_x, next_y, 0, 0);

    // check if zip can make it back home
    if (curr_dist + min_dist + dist_next_to_nest < max_range)
    {
        curr_x = next_x;
        curr_y = next_y;
        return_dist = dist_next_to_nest;
        curr_dist += min_dist;
        orders.erase(orders.begin() + min_idx);
        return min_order;
    }
    return nullptr;
}
}  // namespace zipline