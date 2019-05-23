/* See LICENSE file for copyright and license details.
 * net.c
 *
 *  DESCRIPTION
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "net.h"
#include "logging.h"



/* send_message: send a message over the Internet */
void send_message (int fd, char const *const msg)
{
    size_t msg_length = strlen (msg),
           bytes_sent = 0;
    do
    {
        errno = 0;
        ssize_t this_round = send (fd,
                                   msg + bytes_sent,
                                   msg_length,
                                   0);
        if (this_round == -1)
        {
            error ("send failed -- %s", strerror (errno));
        }

        bytes_sent += this_round;
    } while (bytes_sent < msg_length);
}

/* get_byte_from_inet: receive one byte from the Internet */
int get_byte_from_inet (int fd)
{
    unsigned char out = 0;

    int status = recv (fd, &out, sizeof(out), 0);

    /* error */
    if (status == -1)
    {
        error ("recv failed -- %s", strerror (errno));
    }
    /* connection closed */
    else if (status == 0)
    {
        error ("connection closed");
    }
    /* success */
    else
    {
        return (int)out;
    }
    return -1;
}

/* receive_message: receive an IRC message from the Internet */
char *receive_message (int fd)
{
    size_t out_len  = 100,
           out_idx  = 0;
    char  *out      = calloc (sizeof(*out), out_len);


    int received = get_byte_from_inet (fd);
    bool hit_message_end = false;
    while (received != -1 && hit_message_end == false)
    {
        if (out_idx >= out_len)
        {
            out = realloc (out, sizeof(*out) * (out_len * 2));
            out_len *= 2;
        }

        out[out_idx++] = (unsigned char)received;
        if (out_idx > 0)
        {
            if (out[out_idx-2] == '\r' && out[out_idx-1] == '\n')
            {
                hit_message_end = true;
                out_idx -= 2;
                break;
            }
        }

        received = get_byte_from_inet (fd);
    }
    /* some error occurred */
    if (received == -1)
    {
        free (out);
        return NULL;
    }


    out = realloc (out, sizeof(*out) * out_idx);
    return out;
}

