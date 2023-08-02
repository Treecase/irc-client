/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "MainLoop.hpp"

#include <poll.h>

#include <system_error>


/* ==[ Public ]== */
void MainLoop::add_fd(int fd)
{
    _fd_monitors.emplace(fd, FDMonitor{MainLoop::_get_monitor_default, {}, {}});
}


void MainLoop::remove_fd(int fd)
{
    _fd_monitors.erase(fd);
}


bool MainLoop::step()
{
    std::vector<struct pollfd> monitors{};
    for (auto const &monitor : _fd_monitors)
    {
        monitors.emplace_back(
            pollfd{
                monitor.first,
                _fdstate_to_pollevent(monitor.second.get_monitor_fn()),
                0});
    }

    auto const err = poll(monitors.data(), monitors.size(), -1);
    if (err == -1)
    {
        throw std::system_error{errno, std::generic_category(), "poll()"};
    }
    else if (err > 0)
    {
        std::vector<int> closed{};
        for (auto const &monitor : monitors)
        {
            auto const flags = _pollevent_to_fdstate(monitor.revents);
            if (flags != FDState::NONE)
                if (_fd_monitors.at(monitor.fd).signal_on_polled.emit(flags))
                    closed.push_back(monitor.fd);
        }
        for (auto const fd : closed)
        {
            // A signal handler could remove an fd between iterations, so we
            // need to check for out_of_range.
            try
            {
                _fd_monitors.at(fd).signal_on_closed.emit();
                remove_fd(fd);
            }
            catch (std::out_of_range const &)
            {
            }
        }
    }
    return !_fd_monitors.empty();
}


void MainLoop::run()
{
    while (step())
        ;
}


void MainLoop::set_get_monitor_fn(int fd, GetMonitor fn)
{
    _fd_monitors.at(fd).get_monitor_fn = fn;
}



/* ===[ Protected ]=== */
FDStateFlags MainLoop::_get_monitor_default()
{
    return FDState::NONE;
}



/* ==[ Private ]== */
short MainLoop::_fdstate_to_pollevent(FDStateFlags state)
{
    short events = 0;
    if (state & READ)
        events |= POLLIN;
    if (state & WRITE)
        events |= POLLOUT;
    // poll() always reports errors, so we don't need to worry about that.
    return events;
}

FDStateFlags MainLoop::_pollevent_to_fdstate(short event)
{
    FDStateFlags state = FDState::NONE;
    if (event & (POLLERR | POLLHUP | POLLNVAL))
        state |= FDState::ERROR;
    if (event & POLLIN)
        state |= FDState::READ;
    if (event & POLLOUT)
        state |= FDState::WRITE;
    return state;
}