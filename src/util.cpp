// Copyright 2021 Zipline International Inc. All rights reserved.

#include "util.h"

namespace zipline
{
namespace
{
std::string trim(const std::string &str)
{
    const std::string whitespace = " \t\n";
    const auto start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
    {
        return {};
    }

    const auto end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}
}  // namespace

std::vector<std::string> ParseInputLine(const std::string &line)
{
    size_t pos{0};
    size_t last_pos{0};
    std::vector<std::string> tokens{};
    std::string delim = ", ";
    while ((pos = line.substr(last_pos, line.size()).find(delim)) != std::string::npos)
    {
        auto token = trim(line.substr(last_pos, pos));
        tokens.emplace_back(token);
        last_pos += pos + delim.size();
    }

    tokens.emplace_back(trim(line.substr(line.find_last_of(delim), line.size())));
    return tokens;
}

}  // namespace zipline
