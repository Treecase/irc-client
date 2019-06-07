/* See LICENSE file for copyright and license details.
 * main.c
 *
 *  A Simple IRC Client
 *
 */

/* ==[ INCLUDES ]== */
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "data.h"
#include "commands.h"
#include "gui.h"
#include "irc_string.h"
#include "logging.h"
#include "net.h"
#include "queue.h"



/* ==[ GLOBALS ]== */
char const *CLIENT_NAME;    /* declared in data.h */


/* ==[ VARIABLES ]== */
static MessageQueue queue;


/* ==[ FUNCTIONS ]== */
/* interpret_irc_command: interpret an IRC command message */
void interpret_irc_command (Message cmd)
{
    CommandSpec matched = { 0 };
    bool matched_a_command = false;

    /* search for an exact match of 'cmd.command' in the list
     * of all IRC commands */
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
            msg_queue_push (&queue, response);
        }
    }
    /* we did not match an IRC command */
    else
    {
        /* maybe it's a numeric reply? */
        char *end = NULL;
        long numeric = strtol (cmd.command, &end, 10);

        /* we successfully parsed a number, so we've received
         * a numeric response */
        if (*end == '\0')
        {
            debug ("numeric response: %ld", numeric);

            String out = string_new();

            /* i starts at 1 because the 1st param of a numeric
             * response is always just your username */
            for (size_t i = 1; i < cmd.param_count; ++i)
            {
                int add = snprintf (NULL, 0, "%s ", cmd.params[i]);
                char *new = calloc (sizeof(*new), add + 1);
                sprintf (new, "%s ", cmd.params[i]);

                string_add (&out, new);
            }

            if (out.length > 0)
            {
                gui_user_print ("%s\n", out.str);
            }
        }
        /* we failed to parse a number, so we've received
         * an invalid message from the server */
        else
        {
            error ("invalid command '%s'", cmd.command);
        }
    }
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
        /* copy the fd_sets for select */
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
                    debug ("RECEIVED: \"%s\"", msg);
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
                if (msg_queue_empty (&queue) == false)
                {
                    debug ("write possible");
                    char *msg = msg_queue_next (&queue);
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


/* read_user_input: get input from the user and add it to
 *  the message queue */
void *read_user_input (void *dummy)
{
    bool running = true;
    for(; running;)
    {
        char *user_in = gui_user_read();

        /* gui_user_read returns NULL on ^D, so if the
         * result is not NULL, it's a command */
        if (user_in != NULL)
        {

            /* add the CRLF IRC command trailer */
            size_t len = strlen (user_in);
            char *cmd = calloc (sizeof(*cmd),
                                len + 3);
            memcpy (cmd, user_in, len);
            cmd[len+0] = '\r';
            cmd[len+1] = '\n';
            cmd[len+2] = '\0';

            /* add the command to the queue */
            msg_queue_push (&queue, cmd);

            free (cmd);
        }
        /* user entered ^D, so we'll exit */
        else
        {
            running = false;
        }
    }
    dummy = dummy;  /* shuts gcc up about unused variables */
    return NULL;
}



/* Simple IRC Client */
int main (int argc, char *argv[])
{
    /* the program's name */
    CLIENT_NAME = argv[0];

    char *server_url  = NULL,
         *port_number = NULL;

    /* handle argv */
    /* TODO: real argv handling */
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


    /* start the GUI */
    gui_init();

    /* create the message queue */
    queue = msg_queue_new (sockfd);

    /* user registration commands */
    msg_queue_push (&queue, "CAP LS\r\n");
    msg_queue_push (&queue, "CAP END\r\n");
    msg_queue_push (&queue, "PASS very_secret_password\r\n");
    msg_queue_push (&queue, "NICK user_name\r\n");
    msg_queue_push (&queue, "USER user_name 0 * :RealName\r\n");


    /* we use two threads, one for user input, and one for
     * sending/receiving from the server */
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

    /* TODO: kill user_io_tread if socket_io_thread exits first
     * (eg. if it encounters an error) */
    /* TODO: kill threads on signal exits (eg. SIGINT, etc.) */
    pthread_join (user_io_thread, NULL);

    msg_queue_push (&queue, "QUIT :Goodbye.\r\n");
    /* idle until the QUIT message is sent */
    /* TODO: don't use a while loop! */
    while (!msg_queue_empty (&queue))
    {
    }
    pthread_cancel (socket_io_thread);

    /* free the message queue */
    msg_queue_delete (&queue);
    /* free the server info struct */
    freeaddrinfo (server_info);
    /* close the socket */
    close (sockfd);

    gui_shutdown();

    return EXIT_SUCCESS;
}

