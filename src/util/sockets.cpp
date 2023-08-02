/* Copyright (C) 2020, 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include <sys/types.h>  // getaddrinfo
#include <sys/socket.h> // connect, getaddrinfo, recv, socket
#include <netdb.h>      // getaddrinfo

#include <cerrno>

#include <sstream>
#include <system_error>


int get_tcp_socket(std::string const &hostname, std::string const &port_number)
{
    struct addrinfo hints{
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *server_info = nullptr;
    int status = getaddrinfo(
        hostname.c_str(),
        port_number.c_str(),
        &hints,
        &server_info);
    if (status != 0)
    {
        throw std::runtime_error{
            "addrinfo failed -- "
            + std::string{gai_strerror(status)}};
    }

    errno = 0;
    int fd = socket(
        server_info->ai_family,
        server_info->ai_socktype,
        server_info->ai_protocol);
    if (fd == -1)
    {
        throw std::system_error{
            errno,
            std::generic_category(),
            "socket()"};
    }

    errno = 0;
    status = connect(
        fd,
        server_info->ai_addr,
        server_info->ai_addrlen);
    if (status != 0)
    {
        throw std::system_error{
            errno,
            std::generic_category(),
            "connect()"};
    }

    freeaddrinfo(server_info);
    return fd;
}


std::string read_socket(int socket)
{
    char buf[256];
    std::stringstream out{};
    for(;;)
    {
        errno = 0;
        ssize_t const len = recv(socket, buf, sizeof(buf), MSG_DONTWAIT);
        if (len == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
            {
                throw std::system_error{
                    errno,
                    std::generic_category(),
                    "recv()"};
            }
        }
        else if (len == 0)
            break;
        else
            out.write(buf, len);
    }
    return out.str();
}


void write_socket(int socket, std::string const &data)
{
    for (size_t total_sent = 0; total_sent < data.size(); )
    {
        errno = 0;
        ssize_t const bytes_sent = send(
            socket,
            data.data() + total_sent,
            data.size() - total_sent,
            0);
        if (bytes_sent == -1)
        {
            throw std::system_error{
                errno,
                std::generic_category(),
                "send()"};
        }
        total_sent += bytes_sent;
    }
}
