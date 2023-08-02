/* Copyright (C) 2020, 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "args.hpp"
#include "MainLoop.hpp"

#include <irc/IRCClient.hpp>
#include <util/sockets.hpp>
#include <util/debug.hpp>
#include <Frontend.hpp>

#include <unistd.h>     // STDIN_FILENO

#include <iostream>


/** Called by MainLoop when STDIN has some input for us. */
bool stdin_cb(FDStateFlags events, Frontend &frontend)
{
    if (events & FDState::ERROR)
        return true;
    if (events & FDState::READ)
        return frontend.input();
    return false;
}


/** Called by IRCClient when it has messages ready to be recieved. */
void frontend_recieve_messages(Frontend &frontend, IRCClient &client)
{
    while (!client.is_recieve_queue_empty())
    {
        auto const msg = client.pop();
        frontend.process_message(msg);
    }
}


/** Called by MainLoop to get poll() event argument for the IRC socket. */
FDStateFlags irc_getmonitor(IRCClient &client)
{
    if (!client.is_send_queue_empty())
        return FDState::READ | FDState::WRITE;
    else
        return FDState::READ;
}


/** Called by MainLoop when the IRC socket has input/can be written to. */
bool irc_cb(FDStateFlags events, int fd, IRCClient &client)
{
    if (events & FDState::ERROR)
    {
        return true;
    }
    if (events & FDState::READ)
    {
        auto const data = read_socket(fd);

        auto d2 = data;
        while (d2.find("\r\n") != std::string::npos)
        {
            auto const msg = Message::parse(d2);
            debugstream << "RECV: " << msg << std::endl;
        }

        if (data.size() == 0)
            return true;
        else
            client.recieve(data);
    }
    if (events & FDState::WRITE)
    {
        auto const data = client.send();

        auto d2 = data;
        while (d2.find("\r\n") != std::string::npos)
        {
            auto const msg = Message::parse(d2);
            debugstream << "SEND: " << msg << std::endl;
        }

        write_socket(fd, data);
    }
    return false;
}


/** Called when the frontend has a message to be sent over IRC. */
void on_frontend_input_available(Message const &message, IRCClient &client)
{
    client.push(message);
}


int main(int argc, char *argv[])
{
    auto const config = parse_args(argc, argv);

    auto const irc_socket = get_tcp_socket(config.hostname, config.port);

    IRCClient irc_client{};
    Frontend frontend{};

    // Notify the frontend when the client recieves an IRC message.
    irc_client.signal_message_recieved.connect(
        std::bind(
            frontend_recieve_messages,
            std::ref(frontend),
            std::ref(irc_client)));

    // Send frontend input to the client.
    frontend.signal_input_available.connect(
        std::bind(
            on_frontend_input_available,
            std::placeholders::_1,
            std::ref(irc_client)));

    // Preload the IRC login process.
    irc_client.push("PASS " + config.password);
    irc_client.push("NICK " + config.username);
    irc_client.push("USER " + config.username + " 0 * :" + config.realname);

    MainLoop mainloop{};

    // stdin monitor.
    mainloop.add_fd(STDIN_FILENO);
    mainloop.set_get_monitor_fn(
        STDIN_FILENO,
        [](){return FDState::READ;});
    mainloop.signal_on_polled(STDIN_FILENO).connect(
        [&frontend](auto events){return stdin_cb(events, frontend);});
    mainloop.signal_on_closed(STDIN_FILENO).connect(
        [irc_socket](){close(irc_socket);});

    // IRC socket monitor.
    mainloop.add_fd(irc_socket);
    mainloop.set_get_monitor_fn(
        irc_socket,
        std::bind(irc_getmonitor, std::ref(irc_client)));
    mainloop.signal_on_polled(irc_socket).connect(
        std::bind(
            irc_cb,
            std::placeholders::_1,
            irc_socket,
            std::ref(irc_client)));
    mainloop.signal_on_closed(irc_socket).connect(
        [&mainloop](){mainloop.remove_fd(STDIN_FILENO);});

    mainloop.run();

    return EXIT_SUCCESS;
}
