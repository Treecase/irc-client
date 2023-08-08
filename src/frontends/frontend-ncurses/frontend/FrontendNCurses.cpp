/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "Frontend.hpp"

#include <util/debug.hpp>
#include <util/strings.hpp>

#include <cctype>
#include <clocale>


Frontend::Frontend()
:   _message_handler{new FrontendMessageHandler{}}
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    mousemask(BUTTON4_PRESSED | BUTTON5_PRESSED, nullptr);

    _backend.signal_response_ready.connect(
        [this](Message const &msg){signal_input_available.emit(msg);});

    int height, width;
    getmaxyx(stdscr, height, width);
    _channelw = newwin(height, 9, 0, 0);
    _main = newwin(height-2, width-9-10, 0, 9);
    _userw = newwin(height, 10, 0, width-10);
    _input = newwin(2, width-9-10, height-2, 9);

    _draw();
}


Frontend::~Frontend()
{
    delwin(_main);
    delwin(_input);
    endwin();
}


bool Frontend::input()
{
    int ch;
    while ((ch = getch()) != ERR)
    {
        switch (ch)
        {
        case KEY_ENTER:
        case '\n':
            _handle_user_input(_buffer);
            _buffer.clear();
            break;

        case KEY_BACKSPACE:
            _backspace();
            break;

        case KEY_MOUSE:{
            MEVENT event;
            getmouse(&event);
            if (wenclose(_main, event.y, event.x))
            {
                if (event.bstate & BUTTON4_PRESSED)
                    _backend.get_active_channel().scroll_up(1);
                if (event.bstate & BUTTON5_PRESSED)
                    _backend.get_active_channel().scroll_down(1);
            }
            if (wenclose(_channelw, event.y, event.x))
            {
                if (event.bstate & BUTTON4_PRESSED)
                {
                    _channels_offset = (
                        _channels_offset == 0? 0 : _channels_offset - 1);
                }
                if (event.bstate & BUTTON5_PRESSED)
                {
                    _channels_offset = std::min(
                        _channels_offset + 1,
                        _backend.get_channels().size());
                }
            }
            if (wenclose(_userw, event.y, event.x))
            {
                if (event.bstate & BUTTON4_PRESSED)
                {
                    _users_offset = (
                        _users_offset == 0? 0 : _users_offset - 1);
                }
                if (event.bstate & BUTTON5_PRESSED)
                {
                    _users_offset = std::min(
                        _users_offset + 1,
                        _backend.get_active_channel().get_users().size());
                }
            }
            break;}

        default:
            if (isascii(ch))
                _add_character(ch);
            break;
        }
    }
    _draw();
    return false;
}


void Frontend::process_message(Message const &msg)
{
    _message_handler->execute(_backend, msg);
    _draw();
}



std::string Frontend::clip(std::string const &string, size_t width)
{
    std::string normalized{};
    for (auto ch : string)
        if (isprint(ch))
            normalized.push_back(ch);

    auto clipped = normalized.substr(0, width);
    if (normalized.size() > width)
        clipped.back() = '-';

    return clipped;
}


void Frontend::_backspace()
{
    if (!_buffer.empty())
        _buffer.pop_back();
}


void Frontend::_add_character(char ch)
{
    _buffer.push_back(ch);
}


void Frontend::_draw_channels()
{
    int height, width;
    getmaxyx(_channelw, height, width);

    werase(_channelw);
    wattrset(_channelw, A_NORMAL);

    // Border
    for (int y = 0; y < height; ++y)
        mvwaddch(_channelw, y, width-1, ACS_VLINE);
    mvwaddch(_channelw, height-2, width-1, ACS_LTEE);

    // Title
    mvwaddstr(_channelw, 0, 0, clip("CHANNELS", width-1).c_str());

    auto const &active = _backend.get_active_channel();
    auto const channels = _backend.get_channels();
    auto it = channels.cbegin();

    for (size_t i = 0; i < _channels_offset && it != channels.cend(); ++i)
        it++;

    for (int y = 0; y < height && it != channels.cend(); ++y, ++it)
    {
        if (it->second.name == active.name)
            wattrset(_channelw, A_REVERSE);
        else
            wattrset(_channelw, A_NORMAL);
        auto const str = clip(it->second.name, width-1);
        mvwaddstr(_channelw, 1+y, 0, str.c_str());
    }
}


void Frontend::_draw_main()
{
    auto const &active = _backend.get_active_channel();
    auto const &scrollback = active.get_scrollback();
    int const height = getmaxy(_main);
    int const width = getmaxx(_main);

    werase(_main);

    int y = 1;
    auto it = scrollback.crbegin();

    for (size_t i = 0; i < active.get_scrollback_offset(); ++i)
        it++;

    while (it != scrollback.crend() && y <= height)
    {
        wmove(_main, height - y, 0);
        for (auto const ch : *it)
        {
            if (getcurx(_main)+1 > width)
                break;
            if (isprint(ch))
                waddch(_main, ch);
        }

        it++;
        y++;
    }
}


void Frontend::_draw_users()
{
    auto &active = _backend.get_active_channel();
    int const width = getmaxx(_userw);
    int const height = getmaxy(_userw);

    // Border
    werase(_userw);
    for (int y = 0; y < height; ++y)
        mvwaddch(_userw, y, 0, ACS_VLINE);
    mvwaddch(_userw, height-2, 0, ACS_RTEE);

    // Title
    mvwaddstr(_userw, 0, 1, clip("USERS", width-1).c_str());

    auto const users = active.get_users();
    auto it = users.cbegin();

    for (size_t i = 0; i < _users_offset && it != users.cend(); ++i)
        it++;

    for (int y = 0; y < height && it != users.cend(); ++y, ++it)
    {
        auto const str = clip(*it, width-1);
        mvwaddstr(_userw, 1+y, 1, str.c_str());
    }
}


void Frontend::_draw_input()
{
    int const width = getmaxx(_input);
    werase(_input);
    mvwhline(_input, 0, 0, ACS_HLINE, width);
    mvwaddstr(_input, 1, 0, _buffer.c_str());
}


void Frontend::_draw()
{
    _draw_channels();
    _draw_main();
    _draw_users();
    _draw_input();
    wrefresh(_channelw);
    wrefresh(_main);
    wrefresh(_userw);
    wrefresh(_input);
}


void Frontend::_handle_user_input(std::string const &line)
{
    if (line.empty())
        return;

    if (line.at(0) != '/')
    {
        auto const channel = _backend.get_active_channel().name;
        signal_input_available.emit("PRIVMSG " + channel + " :" + line);
    }
    else
    {
        auto const cmd = line.substr(1);
        auto const cmdL = lowercase(cmd);
        if (cmdL == "reload")
        {
            _message_handler.reset(new FrontendMessageHandler{});
            debugstream << "=== scripts reloaded" << std::endl;
            for (auto &kv : _backend.get_channels())
            {
                auto channel = kv.second;
                channel.push_message("=== scripts reloaded ===");
            }
        }
        else if (cmdL.find("channel") == 0)
        {
            auto const i = cmd.rfind(' ');
            if (i == std::string::npos)
            {
                _backend.get_active_channel().push_message(
                    "=== /channel: missing channel name");
                return;
            }

            auto const arg = cmd.substr(i+1);
            try {
                _backend.set_active_channel(arg);
            } catch (std::out_of_range const &e) {
                _backend.get_active_channel().push_message(
                    "=== /channel: channel '" + arg + "' does not exist");
            }
        }
        else
        {
            signal_input_available.emit(Message{cmd});
        }
    }
}
