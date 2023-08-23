// Copyright 2021 Zipline International Inc. All rights reserved.

#include "hospital.h"

#include <cassert>
#include <fstream>
#include <vector>
#include <iostream>

#include "util.h"

namespace zipline
{
std::unordered_map<std::string, Hospital> Hospital::LoadHospitals(const std::filesystem::path &filename)
{
    std::ifstream stream{filename};

    std::unordered_map<std::string, Hospital> hospitals;

    std::string line{};
    while (std::getline(stream, line, '\n'))
    {
        auto tokens = ParseInputLine(line);

        assert(tokens.size() == 3 && "Got wrong number of hospital elements");
        hospitals.insert({tokens[0], Hospital{tokens[0], atoi(tokens[1].c_str()), atoi(tokens[2].c_str())}});
    }

    return hospitals;
}

}  // namespace zipline
