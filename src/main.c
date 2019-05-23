/*
 * A Simple IRC Client
 *
 */

/* ==[ INCLUDES ]== */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include <readline/readline.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "data.h"
#include "logging.h"
#include "commands.h"
#include "queue.h"



/* ==[ GLOBALS ]== */
char const *CLIENT_NAME;    /* declared in data.h */


/* ==[ VARIABLES ]== */
static MessageQueue queue;


/* ==[ FUNCTIONS ]== */
void *socket_read_and_write (void *dummy);

/* read_user_input:  */
void *read_user_input (void *dummy)
{
    bool running = true;
    for(; running;)
    {
        char *user_in = readline (" >");

        /* regular message */
        if (user_in != NULL)
        {
            size_t len = strlen (user_in);

            char *cmd = calloc (sizeof(*cmd),
                                len + 3);
            memcpy (cmd, user_in, len);
            cmd[len+0] = '\r';
            cmd[len+1] = '\n';
            cmd[len+2] = '\0';

            queue_push (&queue, cmd);

            free (cmd);
            free (user_in);
        }
        /* user wants to quit */
        else
        {
            running = false;
        }
    }
    dummy = dummy;
    return NULL;
}

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

/* interpret_irc_command: interpret an IRC command message */
void interpret_irc_command (Message cmd)
{
    CommandSpec matched = { 0 };
    bool matched_a_command = false;

    for (size_t i = 0; i < command_count; ++i)
    {
        if (strcmp (cmd.command, commands[i].name) == 0)
        {
            matched = commands[i];
            matched_a_command = true;
            break;
        }
    }

    /* we matched an IRC command, so parse it out and respond */
    if (matched_a_command)
    {
        debug ("matched command '%s'", matched.name);

        char *response = matched.response (cmd);
        if (response != NULL)
        {
            debug ("responding with '%s'", response);
            queue_push (&queue, response);
        }
    }
    /* we did not match an IRC command */
    else
    {
        /* maybe it's a numeric reply? */
        char *end = NULL;
        long numeric = strtol ( cmd.command, &end, 10 );

        /* we successfully parsed a number, so we've received
         * a numeric response */
        if (*end == '\0')
        {
            debug ("numeric response: %ld\n", numeric);
            /* don't print the first parameter of a numeric response,
             * as it's just the username */
            for (size_t i = 1; i < cmd.param_count; ++i)
            {
                printf ("%s", cmd.params[i]);
                if (i < cmd.param_count-1)
                {
                    printf (" ");
                }
            }
            printf ("\n");
        }
        /* we failed to parse a number, so we've received
         * an invalid message from the server */
        else
        {
            fatal ("invalid command '%s'", cmd.command);
        }
    }
}



/* Simple IRC Client */
int main (int argc, char *argv[])
{
    CLIENT_NAME = argv[0];

    char *server_url  = NULL,
         *port_number = NULL;

    if (argc >= 2)
    {
        server_url  = argv[1];
        port_number = (argc >= 3)? argv[2]: "irc";
    }
    else
    {
        printf ("Usage: %s SERVER [PORT]\n", CLIENT_NAME);
        fatal ("no server supplied!");
    }

    /* sockfd is our socket file descriptor */
    int sockfd = 0;
    struct addrinfo hints        = { 0 },
                    *server_info = NULL;

    /* we're using a TCP socket */
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* get address info */
    debug ("getaddrinfo");
    int status = getaddrinfo (server_url,
                              port_number,
                              &hints,
                              &server_info);
    if (status != 0)
    {
        fatal ("addrinfo failed -- %s", gai_strerror (status));
    }

    /* create the socket */
    debug ("socket");
    errno = 0;
    sockfd = socket (server_info->ai_family,
                     server_info->ai_socktype,
                     server_info->ai_protocol);
    if (sockfd == -1)
    {
        fatal ("socket failed -- %s", strerror (errno));
    }

    /* connect to the IRC server */
    debug ("connect");
    errno = 0;
    status = connect (sockfd, server_info->ai_addr,
                      server_info->ai_addrlen);
    if (status != 0)
    {
        fatal ("connect failed -- %s", strerror (errno));
    }


    /* create the message queue */
    queue = queue_new (sockfd);

    /* user registration commands */
    queue_push (&queue, "PASS very_secret_password\r\n");
    queue_push (&queue, "NICK user_name\r\n");
    queue_push (&queue, "USER user_name 0 * :RealName\r\n");


    pthread_t socket_io_thread,
              user_io_thread;

    if (pthread_create (&socket_io_thread, NULL,
                        socket_read_and_write, (void*)&sockfd))
    {
        fatal ("failed to create socket I/O thread");
    }

    if (pthread_create (&user_io_thread, NULL,
                          read_user_input, NULL))
    {
        fatal ("failed to create user I/O thread!");
    }

    pthread_join (user_io_thread, NULL);
    pthread_cancel (socket_io_thread);

    /* free the message queue */
    queue_delete (&queue);
    /* free the server info struct */
    freeaddrinfo (server_info);
    /* close the socket */
    close (sockfd);

    return EXIT_SUCCESS;
}


/* socket_read_and_write: read from and write to the network */
void *socket_read_and_write (void *sockfd_addr)
{
    int sockfd = *((int*)sockfd_addr);

    /* create the read event fd_set */
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    int timeout_counter = 0;
    bool finished = false;
    while (!finished)
    {
        /* copy the fd_sets for input to select */
        fd_set readfds_tmp  = readfds,
               writefds_tmp = queue.write_fds;

        /* set the timeout period */
        struct timeval timeout = { 0 };
        timeout.tv_sec  = 1;
        timeout.tv_usec = 0;

        /* wait for a read/write event */
        errno = 0;
        debug ("waiting for select()...");
        int res = select (sockfd+1,
                          &readfds_tmp,
                          &writefds_tmp,
                          NULL,
                          &timeout);

        /* select returns -1 on error */
        if (res == -1)
        {
            error ("select failed -- %s", strerror (errno));
        }
        /* select returns 0 if it times out */
        else if (res == 0)
        {
            debug ("timed out (%d in a row)", ++timeout_counter);
        }
        /* otherwise, there are some file descriptors which are
         * ready to be read from or written to */
        else
        {
            /* it is possible to receive data */
            if (FD_ISSET(sockfd, &readfds_tmp))
            {
                debug ("read possible");
                char *msg = receive_message (sockfd);

                /* 'msg' is not NULL, so no error occurred */
                if (msg != NULL)
                {
                    debug ("RECEIVED: '%.*s'",
                           (int)(strlen (msg)-2),
                           msg);
                    Message message = str_to_message (msg);
                    interpret_irc_command (message);

                    free (msg);
                }
                /* a NULL message means an error occurred */
                else
                {
                    finished = true;
                    pthread_exit (NULL);
                }
            }
            /* it is possible to send data */
            if (FD_ISSET(sockfd, &writefds_tmp))
            {
                /* there is a message in the queue, so send it */
                if (queue_empty (&queue) == false)
                {
                    debug ("write possible");
                    char *msg = queue_next (&queue);
                    debug ("SENDING '%.*s'",
                           (int)(strlen (msg)-2),
                           msg);
                    send_message (sockfd, msg);
                }
            }
            timeout_counter = 0;
        }

        debug ("end cycle");
    }

    return NULL;
}

