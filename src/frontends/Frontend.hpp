/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 *
 * This file should never actually be included in the program.
 * It only serves as interface documentation for what methods/members must be
 * defined and their purpose.
 */

#error "This Frontend.hpp should not be included in your project! It is meant"\
       "to serve only as a reference for actual implementations."


#include <irc/Message.hpp>
#include <util/Signal.hpp>


class Frontend
{
public:
    /** Emitted when user input has an IRC message ready to be sent. */
    Signal<void(Message)> signal_input_available{};

    /** Process input. Returns true on error/EOF. */
    virtual bool input()=0;

    /** Process available IRC messages from CLIENT. */
    virtual void process_message(Message const &message)=0;
};
