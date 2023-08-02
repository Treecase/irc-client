/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "irc/Message.hpp"

#include <sstream>
#include <stdexcept>


Message Message::parse(std::string &src)
{
    auto const crlf = src.find("\r\n");
    if (crlf == std::string::npos)
        throw std::runtime_error{"incomplete IRC message"};

    Message msg{src.substr(0, crlf)};

    src = src.substr(crlf + 2);
    return msg;
}


std::string Message::to_irc() const
{
    auto const str = this->operator std::string() + "\r\n";
    if (str.size() > 512)
    {
        throw std::runtime_error{
            "Messages cannot exceed 512 characters!"
            " (Message is " + std::to_string(str.size()) + " chars long)"};
    }
    return str;
}


Message::operator std::string() const
{
    std::stringstream str{};

    if (prefix.has_value())
        str << ':' << prefix.value() << ' ';

    str << command;

    for (size_t i = 1; i < params.size(); ++i)
        str << ' ' << params.at(i-1);
    if (!params.empty())
        str << " :" << params.back();

    return str.str();
}


Message::Message(
    std::string const &command,
    std::vector<std::string> const &params)
:   command{command}
,   params{params}
{
}


Message::Message(
    std::string const &prefix,
    std::string const &command,
    std::vector<std::string> const &params)
:   prefix{prefix}
,   command{command}
,   params{params}
{
}


Message::Message(std::string message)
{
    // TODO: Handle case of more than 1 space character.
    if (message.at(0) == ':')
    {
        auto const prefix_end = message.find(' ');
        if (prefix_end == std::string::npos)
            throw std::runtime_error{"message is missing command"};
        prefix = message.substr(1, prefix_end - 1);
        message = message.substr(prefix_end + 1);
    }

    auto const command_end = message.find(' ');
    command = message.substr(0, command_end);

    auto end = command_end;
    while (end != std::string::npos)
    {
        message = message.substr(end + 1);
        size_t i = 0;
        if (message.at(0) == ':')
        {
            i = 1;
            end = std::string::npos;
        }
        else
            end = message.find(' ');
        params.push_back(message.substr(i, end));
    }
}



std::ostream &operator<<(std::ostream &os, Message const &msg)
{
    return os << std::string{msg};
}
