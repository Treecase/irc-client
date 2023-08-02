/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTEND_FRONTENDNCURSES_HPP
#define FRONTEND_FRONTENDNCURSES_HPP

#include <Backend.hpp>
#include <Channel.hpp>
#include <MessageHandler.hpp>

#include <irc/Message.hpp>
#include <util/Signal.hpp>

#include <ncurses.h>

#include <string>
#include <unordered_map>


/**
 * FrontendNCurses.
 *
 * More advanced frontend using NCurses functionality.
 */
class Frontend
{
    friend FrontendMessageHandler;
public:
    /** Emitted when user input has an IRC message ready to be sent. */
    Signal<void(Message)> signal_input_available{};

    Frontend();
    ~Frontend();

    /** Process input. Returns true on error/EOF. */
    bool input();

    /** Process available IRC messages from CLIENT. */
    void process_message(Message const &message);

private:
    std::string _buffer{};

    Backend _backend{};
    FrontendMessageHandler _message_handler{};

    WINDOW *_channelw{nullptr};
    WINDOW *_main{nullptr};
    WINDOW *_userw{nullptr};
    WINDOW *_input{nullptr};

    static std::string clip(std::string const &string, size_t width);

    // input controls
    void _backspace();
    void _add_character(char ch);

    void _handle_user_input(std::string const &line);

    void _draw_channels();
    void _draw_main();
    void _draw_users();
    void _draw_input();
    void _draw();
};


#endif
