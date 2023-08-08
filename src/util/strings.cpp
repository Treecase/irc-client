/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "util/strings.hpp"


std::string lowercase(std::string const &str)
{
    std::string out{};
    for (unsigned char ch : str)
        out.push_back(std::tolower(ch));
    return out;
}
