/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTENDNCURSES_BACKEND_HPP
#define FRONTENDNCURSES_BACKEND_HPP

#include "Channel.hpp"

#include <util/Signal.hpp>

#include <unordered_map>
#include <string>


/**
 * IRC client backend.
 */
class Backend
{
    std::unordered_map<std::string, Channel> _channels{};
    std::string _active_channel{};

public:
    Signal<void(Message)> signal_response_ready{};

    Backend();

    auto &get_channels() const {return _channels;}
    void set_active_channel(std::string const &channel);
    Channel &get_active_channel();
    void send_response(Message const &msg);
};


#endif
