/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *  Socket with attached queue.
 *  IMPORTANT: The file descriptor is *shared* between copies!
 */

#ifndef _SOCKET_H
#define _SOCKET_H

#include <string>
#include <deque>


class QueuedSocket
{
private:
    std::deque<std::string> _read_queue,
                            _write_queue;

    std::string _read_data_real(void);
    void _write_data_real(std::string data);

public:
    int fd;
    bool is_closed;

    /* pull data out of the read queue */
    std::string read(void);

    /* push data into the write queue */
    void write(std::string data);

    /* check if the fd can be read or written to,
     * and take any available action */
    void update(void);

    /* return true if the fd can be read from */
    bool can_read(void);

    /* return true if the fd has data in its write buffer */
    bool data_to_send(void);

    /* get a pollfd struct for the fd */
    struct pollfd get_pollfd(void);

    QueuedSocket(int fd_);
    ~QueuedSocket();
};


#endif

