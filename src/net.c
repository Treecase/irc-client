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
#include "irc_string.h"
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

    /* an error occurred */
    if (status == -1)
    {
        error ("recv failed -- %s", strerror (errno));
    }
    /* the connection was closed */
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
    String out = string_new();

    int received = get_byte_from_inet (fd);
    bool hit_message_end = false;
    while (received != -1 && hit_message_end == false)
    {
        string_add_ch (&out, received);

        if (out.length >= 2)
        {
            /* commands end with CRLF */
            if (out.str[out.length - 2] == '\r'
             && out.str[out.length - 1] == '\n')
            {
                hit_message_end = true;

                /* remove the CRLF from our return value */
                out.length -= 2;
                out.str[out.length] = '\0';
                break;
            }
        }

        received = get_byte_from_inet (fd);
    }
    /* some error occurred */
    if (received == -1)
    {
        string_delete (&out);
        return NULL;
    }


    return out.str;
}

