/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "Frontend.hpp"

#include <util/debug.hpp>

#include <clocale>
#include <cctype>


Frontend::Frontend()
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
            if (event.bstate & BUTTON4_PRESSED)
                _backend.get_active_channel().scroll_up(1);
            if (event.bstate & BUTTON5_PRESSED)
                _backend.get_active_channel().scroll_down(1);
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
    auto const interaction = _message_handler.execute(this, msg);
    if (interaction)
        interaction->execute(_backend);
    _draw();
}



std::string Frontend::clip(std::string const &string, size_t width)
{
    std::string normalized{};
    for (auto ch : string)
        if (isprint(ch))
            normalized.push_back(ch);

    auto clipped = normalized.substr(0, width);
    if (normalized.size() >= width)
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

    // Border
    for (int y = 0; y < height; ++y)
        mvwaddch(_channelw, y, width-1, ACS_VLINE);
    mvwaddch(_channelw, height-2, width-1, ACS_LTEE);

    mvwaddstr(_channelw, 0, 0, "CHANNELS");
    auto const channels = _backend.get_channels();
    auto it = channels.cbegin();
    for (int i = 0; it != channels.cend() && i < height; ++i, ++it)
    {
        auto const str = clip(it->first, width);
        mvwaddstr(_channelw, 1+i, 0, str.c_str());
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
        auto msg = *it;
        msg.prefix.reset();
        auto const str = clip(msg, width);
        mvwaddstr(_main, height - y, 0, str.c_str());

        it++;
        y++;
    }
}


void Frontend::_draw_users()
{
    auto &active = _backend.get_active_channel();
    int const width = getmaxx(_userw);
    int const height = getmaxy(_userw);

    werase(_userw);
    for (int y = 0; y < height; ++y)
        mvwaddch(_userw, y, 0, ACS_VLINE);
    mvwaddch(_userw, height-2, 0, ACS_RTEE);

    mvwaddstr(_userw, 0, 1, "USERS");
    auto it = active.get_users().cbegin();
    for (int i = 0; it != active.get_users().cend() && i < height; ++i, ++it)
    {
        auto const str = clip(*it, width-1);
        mvwaddstr(_userw, 1+i, 1, str.c_str());
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
        signal_input_available.emit(Message{"PRIVMSG target :" + line});
    else
        signal_input_available.emit(Message{line.substr(1)});
}
