/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "Channel.hpp"

#include <algorithm>


Channel::Channel(std::string const &name)
:   name{name}
{
}


void Channel::push_message(Message const &msg)
{
    if (scrollback_offset != 0)
        scrollback_offset += 1;
    scrollback.push_back(msg);
}


void Channel::scroll_up(size_t lines)
{
    scrollback_offset = std::min({
        scrollback.size(),
        scrollback_offset + lines});
}


void Channel::scroll_down(size_t lines)
{
    if (lines <= scrollback_offset)
        scrollback_offset -= lines;
}


void Channel::add_user(std::string const &user)
{
    users.insert(user);
}


void Channel::remove_user(std::string const &user)
{
    users.erase(user);
}
