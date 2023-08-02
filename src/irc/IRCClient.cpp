/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "irc/IRCClient.hpp"

#include <sstream>


void IRCClient::recieve(std::string const &data)
{
    auto dat = leftover + data;
    while (dat.find("\r\n") != std::string::npos)
        _recieve_queue.push(Message::parse(dat));
    leftover = dat;
    signal_message_recieved.emit();
}


std::string IRCClient::send()
{
    std::stringstream out{};
    while (!_send_queue.empty())
    {
        auto const msg = _send_queue.front();
        _send_queue.pop();
        out << msg.to_irc();
    }
    return out.str();
}


Message IRCClient::pop()
{
    auto const msg = _recieve_queue.front();
    _recieve_queue.pop();
    return msg;
}


void IRCClient::push(Message const &msg)
{
    _send_queue.push(msg);
}


bool IRCClient::is_recieve_queue_empty() const
{
    return _recieve_queue.empty();
}


bool IRCClient::is_send_queue_empty() const
{
    return _send_queue.empty();
}
