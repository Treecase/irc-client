/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 * socket.cpp
 *
 *  Class for queued Unix socket
 *
 */

#include "socket.h"
#include "logging.h"

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#include <stdexcept>
#include <system_error>



std::string QueuedSocket::read(void)
{
    std::string data = _read_queue.front();
    _read_queue.pop_front();
    return data;
}

void QueuedSocket::write(std::string data)
{
    _write_queue.push_back(data);
}

void QueuedSocket::update(void)
{
    if (is_closed)
    {
        return;
    }

    struct pollfd monitor = get_pollfd();
    int count = poll(&monitor, 1, 0);

    /* the fd is ready for some operation */
    if (count > 0)
    {
        /* data can be read */
        if (monitor.revents & POLLIN)
        {
            std::string data = _read_data_real();
            _read_queue.push_back(data);
        }
        /* data can be written */
        if (monitor.revents & POLLOUT)
        {
            while (!_write_queue.empty())
            {
                _write_data_real(_write_queue.front());
                _write_queue.pop_front();
            }
        }
        if (monitor.revents & (POLLHUP | POLLNVAL | POLLERR))
        {
            is_closed = true;
        }
    }
    /* an error occurred */
    else if (count < 0)
    {
        throw std::system_error(
            errno,
            std::generic_category(),
            "poll");
    }
}

bool QueuedSocket::can_read(void)
{
    return !_read_queue.empty();
}

bool QueuedSocket::data_to_send(void)
{
    return !_write_queue.empty();
}

struct pollfd QueuedSocket::get_pollfd(void)
{
    struct pollfd monitor{};
    monitor.fd = fd;
    monitor.events = POLLIN | (_write_queue.empty()? 0 : POLLOUT);
    monitor.revents = 0;
    return monitor;
}

QueuedSocket::QueuedSocket(int fd_)
:   fd(fd_),
    is_closed(false)
{
}

QueuedSocket::~QueuedSocket()
{
}


std::string QueuedSocket::_read_data_real(void)
{
    std::string out = "";

    size_t size = 100;
    char *buf = new char[size];

    for(;;)
    {
        errno = 0;
        ssize_t len = recv(fd, buf, size, MSG_DONTWAIT);

        if (len == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                throw std::system_error(
                    errno,
                    std::generic_category(),
                    "recv");
            }
        }
        else if (len == 0 && size != 0)
        {
            is_closed = true;
            break;
        }
        else
        {
            char *tmpbuf = new char[len+1];
            memcpy(tmpbuf, buf, len);
            tmpbuf[len] = '\0';
            out += std::string(tmpbuf);
            delete[] tmpbuf;
        }
    }
    delete[] buf;
    return out;
}

void QueuedSocket::_write_data_real(std::string data)
{
    size_t total_sent = 0;
    do
    {
        errno = 0;
        ssize_t bytes_sent = send(
            fd,
            data.data() + total_sent,
            data.size() - total_sent,
            0);

        if (bytes_sent == -1)
        {
            throw std::system_error(
                errno,
                std::generic_category(),
                "send");
        }
        total_sent += bytes_sent;
    } while (total_sent < data.size());
}

