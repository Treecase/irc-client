/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTEND_FRONTENDTERMINAL_HPP
#define FRONTEND_FRONTENDTERMINAL_HPP

#include <irc/Message.hpp>
#include <util/Signal.hpp>


/**
 * FrontendTerminal.
 *
 * Super simple frontend that just uses stdin/stdout.
 */
class Frontend
{
public:
    /** Emitted when user input has an IRC message ready to be sent. */
    Signal<void(Message)> signal_input_available{};

    /** Process input. Returns true on error/EOF. */
    bool input();

    /** Process available IRC messages from CLIENT. */
    void process_message(Message const &message);

private:
    void output(Message const &message);
};


#endif
