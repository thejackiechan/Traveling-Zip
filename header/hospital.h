// Copyright 2021 Zipline International Inc. All rights reserved.

#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace zipline
{
class Hospital
{
   public:
    Hospital(const std::string &name, int north, int east) : name_(name), north_(north), east_(east)
    {
    }

    static std::unordered_map<std::string, Hospital> LoadHospitals(const std::filesystem::path &filename);

    std::string name() const
    {
        return name_;
    }

    int north() const
    {
        return north_;
    }

    int east() const
    {
        return east_;
    }

   private:
    const std::string name_;
    const int north_;
    const int east_;
};

}  // namespace zipline
