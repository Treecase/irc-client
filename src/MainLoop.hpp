/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef IRCC_MAINLOOP_HPP
#define IRCC_MAINLOOP_HPP

#include <util/Signal.hpp>

#include <functional>
#include <unordered_map>


using FDStateFlags = unsigned short;

/** File descriptor state flags. */
enum FDState : FDStateFlags
{
    NONE  = 0b000,
    ERROR = 0b001,
    READ  = 0b010,
    WRITE = 0b100,
};


/**
 * IRC client main loop.
 *
 * First give the loop some file descriptors to monitor. In the loop, these
 * will be polled on until they either encounter an error or are ready to be
 * written to/read from, depending on flags returned by the respective function
 * passed to 'set_get_monitor_fn'. When a file descriptor is ready, it will
 * emit an 'on_polled' signal. This signal can be accessed through the
 * 'signal_on_polled' method. If any of the connected callbacks return TRUE,
 * that file descriptor will be closed. The loop runs until all monitored file
 * descriptors are closed.
 */
class MainLoop
{
    using GetMonitor = std::function<FDStateFlags()>;

    struct FDMonitor
    {
        GetMonitor get_monitor_fn{};
        Signal<bool(FDStateFlags)> signal_on_polled{};
        Signal<void()> signal_on_closed{};
    };

    std::unordered_map<int, FDMonitor> _fd_monitors{};

    static short _fdstate_to_pollevent(FDStateFlags state);
    static FDStateFlags _pollevent_to_fdstate(short events);

protected:
    static FDStateFlags _get_monitor_default();

public:
    /** Add a file descriptor to be monitored. */
    void add_fd(int fd);
    /** Stop monitoring the file descriptor. */
    void remove_fd(int fd);
    /** Run one iteration of the mainloop. */
    bool step();
    /** Run the mainloop. */
    void run();

    /** Set a file descriptor's GetMonitor function. */
    void set_get_monitor_fn(int fd, GetMonitor fn);
    /** Get a file descriptor's "on_polled" Signal. */
    auto &signal_on_polled(int fd)
    {return _fd_monitors.at(fd).signal_on_polled;};
    /** Get a file descriptor's "on_closed" Signal. */
    auto &signal_on_closed(int fd)
    {return _fd_monitors.at(fd).signal_on_closed;};
};


#endif
