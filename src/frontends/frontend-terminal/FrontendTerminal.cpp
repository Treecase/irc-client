/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "frontend/Frontend.hpp"

#include <iostream>


bool Frontend::input()
{
    char *line = nullptr;
    size_t n = 0;
    auto const i = getline(&line, &n, stdin);
    if (i != -1)
    {
        std::string input{line, static_cast<size_t>(i)};
        if (input.back() == '\n')
            input.pop_back();
        output(Message{input});
    }
    else
        return true;
    return false;
}


void Frontend::process_message(Message const &msg)
{
    std::cout << "irc <- " << msg << '\n';

    if (msg.command == "PING")
        output(Message{"PONG", msg.params});
}



void Frontend::output(Message const &message)
{
    std::cout << "irc -> " << message << '\n';
    signal_input_available.emit(message);
}
