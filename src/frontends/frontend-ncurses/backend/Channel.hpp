/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTENDNCURSES_CHANNEL_HPP
#define FRONTENDNCURSES_CHANNEL_HPP

#include <irc/Message.hpp>

#include <set>
#include <string>
#include <vector>


/**
 * An IRC channel with scrollback buffer.
 */
class Channel
{
    std::set<std::string> users{};
    std::vector<std::string> scrollback{};
    size_t scrollback_offset{0};

public:
    std::string const name;

    Channel(std::string const &name);

    /** Add a message to the scrollback. */
    void push_message(std::string const &msg);

    /** Scrollback buffer up. */
    void scroll_up(size_t lines);
    /** Scrollback buffer down. */
    void scroll_down(size_t lines);

    /** Add user to user list. */
    void add_user(std::string const &user);
    /** Remove user from user list. */
    void remove_user(std::string const &user);

    auto &get_scrollback() const {return scrollback;}
    auto get_scrollback_offset() const {return scrollback_offset;}
    auto &get_users() const {return users;}
};


#endif
