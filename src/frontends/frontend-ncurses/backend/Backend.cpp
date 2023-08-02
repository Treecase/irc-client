/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "Backend.hpp"


Backend::Backend()
{
    _channels.emplace("", "<base>");
}


void Backend::set_active_channel(std::string const &channel)
{
    _active_channel = channel;
}


Channel &Backend::get_active_channel()
{
    return _channels.at(_active_channel);
}


void Backend::send_response(Message const &msg)
{
    signal_response_ready.emit(msg);
}
