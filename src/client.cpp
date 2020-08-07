/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *  A Simple IRC Client
 */

#include "commands.h"
#include "command_responses.h"
#include "data.h"
#include "logging.h"
#include "socket.h"

#include <netdb.h>
#include <poll.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstdbool>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <memory>
#include <stdexcept>
#include <system_error>



/* Get a TCP socket, connected to the given server */
int get_tcp_socket(std::string hostname, std::string port_number)
{
    /* set up hints for a TCP socket */
    struct addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* get information about the given hostname */
    struct addrinfo *server_info = NULL;
    int status = getaddrinfo(
        hostname.c_str(),
        port_number.c_str(),
        &hints,
        &server_info);
    if (status != 0)
    {
        throw std::runtime_error(
            "addrinfo failed -- "
            + std::string(gai_strerror(status)));
    }

    /* create the socket */
    errno = 0;
    int fd = socket(
        server_info->ai_family,
        server_info->ai_socktype,
        server_info->ai_protocol);
    if (fd == -1)
    {
        throw std::system_error(
            errno,
            std::generic_category(),
            "socket() failed");
    }

    /* connect to the server */
    errno = 0;
    status = connect(
        fd,
        server_info->ai_addr,
        server_info->ai_addrlen);
    if (status != 0)
    {
        throw std::system_error(
            errno,
            std::generic_category(),
            "connect() failed");
    }

    freeaddrinfo(server_info);
    return fd;
}



void client(
    int fd,
    std::string hostname,
    std::string port)
{
    /* open a socket to the IRC server */
    QueuedSocket tcp_socket{get_tcp_socket(hostname, port)};
    std::string tcp_unfinished_msg = "";

    /* open a socket to the GUI client */
    QueuedSocket client_socket{fd};

    std::deque<std::string> irc_read{};
    std::deque<std::string> irc_write{};


    while (!tcp_socket.is_closed && !client_socket.is_closed)
    {
        if (client_socket.is_closed)
        {
            tcp_socket.write("QUIT :\r\n");
        }
        if (tcp_socket.is_closed)
        {
            client_socket.write("EXIT\r\n");
        }

        /* pull messages out of the irc_read queue, then write
         * the proper response to the irc_write queue */
        while (!irc_read.empty())
        {
            std::string msg = irc_read.front();
            irc_read.pop_front();
            debug("TCP rcv: '%s'", msg.c_str());

            Message message(msg);

            bool is_command = false;
            Response *response = nullptr;
            for (CommandSpec cmd : irc_commands)
            {
                if (cmd.name == message.command)
                {
                    response =\
                        irc_command_responses[cmd.name](message);
                    is_command = true;
                    break;
                }
            }
            if (is_command)
            {
                if (response != nullptr)
                {
                    switch (response->type)
                    {
                    case Response::IRC:
                        irc_write.push_back(response->str);
                        break;

                    case Response::Client:
                        client_socket.write(
                            response->str + "\r\n");
                        debug("GUI snd '%s'",
                            response->str.c_str());
                        break;

                    case Response::Error:
                        error("%s", response->str.c_str());
                        break;
                    }
                    delete response;
                }
            }
            else
            {
                bool is_numeric_reply = true;
                for (char c : message.command)
                {
                    if (!isdigit(c))
                    {
                        is_numeric_reply = false;
                        break;
                    }
                }

                if (is_numeric_reply)
                {
                    debug("numeric reply: %s",
                        message.command.c_str());

                    auto out = "NUMERIC " + message.command + " ";
                    for (
                        size_t i = 1;
                        i < message.params.size();
                        ++i)
                    {
                        out += message.params[i];
                        if (i + 1 < message.params.size())
                        {
                            out += " ";
                        }
                    }

                    client_socket.write(out + "\r\n");
                }
                else
                {
                    error("bad message: '%s'", msg.c_str());
                }
            }
        }

        /* IMPORTANT: this MUST be done AFTER reading the input queue,
         * since we want our responses to be sent too! */
        /* pull messages out of the irc_write queue
         * and send them down the TCP socket */
        while (!irc_write.empty())
        {
            /* since we want to write to the TCP socket, we'll
             * monitor it for writing to be possible */
            std::string msg = irc_write.front();
            irc_write.pop_front();
            debug("TCP snd: '%s'", msg.c_str());
            tcp_socket.write(msg + "\r\n");
        }

        struct pollfd poll_fds[2] = {};
        poll_fds[0] = tcp_socket.get_pollfd();
        poll_fds[1] = client_socket.get_pollfd();

        int ret = poll(poll_fds, 2, -1);

        if (ret == -1)
        {
            throw std::system_error(
                errno,
                std::generic_category(),
                "poll() failed");
        }
        else if (ret > 0)
        {
            tcp_socket.update();
            client_socket.update();

            /* TCP fd has some data for us */
            if (tcp_socket.can_read())
            {
                std::string data = tcp_socket.read();

                /* pull all the sent messages out of the buffer
                 * and send them to the message queue */
                size_t msg_start = 0;
                while (msg_start < data.size())
                {
                    size_t msg_end = data.find("\r\n", msg_start);
                    if (msg_end != std::string::npos)
                    {
                        tcp_unfinished_msg +=\
                            data.substr(
                                msg_start,
                                msg_end - msg_start);

                        irc_read.push_back(tcp_unfinished_msg);
                        tcp_unfinished_msg = "";
                        msg_start = msg_end + 2;
                    }
                    else
                    {
                        tcp_unfinished_msg += data.substr(msg_start);
                        break;
                    }
                }
            }
            /* client fd has some data for us */
            if (client_socket.can_read())
            {
                std::string data = client_socket.read();

                /* split the messages on \r\n */
                size_t msg_start = 0;
                while (msg_start < data.size())
                {
                    size_t msg_end = data.find("\r\n", msg_start);
                    if (msg_end != std::string::npos)
                    {
                        std::string msgstr = data.substr(
                            msg_start,
                            msg_end - msg_start);

                        debug("GUI recv '%s'", msgstr.c_str());
                        msg_start = msg_end + 2;

                        /* send the received message */
                        irc_write.push_back(msgstr);
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    if (!client_socket.is_closed)
    {
        client_socket.write("EXIT\r\n");
        while (client_socket.data_to_send())
        {
            client_socket.update();
        }
    }

    /* cleanup */
    close(tcp_socket.fd);
    debug("finished!");
}

