/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef IRC_IRCCLIENT_HPP
#define IRC_IRCCLIENT_HPP

#include "Message.hpp"

#include <util/Signal.hpp>

#include <queue>


class IRCClient
{
public:
    Signal<void()> signal_message_recieved{};

    /** Push raw IRC data into the recieve queue. */
    void recieve(std::string const &data);
    /** Pull raw IRC data from the send queue. */
    std::string send();

    /**
     * Pop the next message from the recieve queue.
     * Throws if the recieve queue is empty.
     */
    Message pop();
    /** Push a message on to the send queue. */
    void push(Message const &msg);

    /** true if the recieve queue is empty. */
    bool is_recieve_queue_empty() const;
    /** true if the send queue is empty. */
    bool is_send_queue_empty() const;

private:
    std::queue<Message> _recieve_queue{};
    std::queue<Message> _send_queue{};
    std::string leftover{};
};


#endif
