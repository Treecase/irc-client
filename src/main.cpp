/* Copyright (C) 2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *  IRC
 */

#define LOGFILE G_logfile
#include "data.h"
#include "logging.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <csignal>

#include <memory>


/* print usage info */
static void usage(char const *name, bool long_version);

/* the IRC client process */
void client(
    int fd,
    std::string hostname,
    std::string port);

/* the GUI process */
void gui(
    int fd,
    std::string username,
    std::string password,
    std::string realname);


/* declared in data.h */
char const *CLIENT_NAME = nullptr;

/* declared in logging.h */
FILE *G_logfile = nullptr;


static pid_t pid = 0;


/* atexit callback */
void cleanup_func(void)
{
    if (G_logfile != nullptr)
    {
        fclose(G_logfile);
        G_logfile = nullptr;
    }
    if (pid != 0)
    {
        kill(pid, SIGTERM);
    }
}

/* SIGCHLD handler */
void parent_sigchldaction(int sig, siginfo_t *info, void *ucontext)
{
    exit(info->si_status);
}



int main(int argc, char *argv[])
{
    atexit(cleanup_func);

    CLIENT_NAME = argv[0];
    G_logfile = fopen((std::string(CLIENT_NAME) + ".log").c_str(), "w");

    if (argc == 2)
    {
        if (   std::string(argv[1]) == "--help"
            || std::string(argv[1]) == "-h")
        {
            usage(argv[0], true);
        }
    }
    if (argc != 4 && argc != 5)
    {
        usage(argv[0], false);
        exit(EXIT_FAILURE);
    }

    std::string hostname = argv[1],
                port     = "6667";

    std::string username = argv[2],
                password = argv[3],
                realname = (argc == 5? argv[4] : "realname");

    /* get the hostname and port number */
    std::string hostname_and_port = argv[1];
    size_t colon = hostname_and_port.rfind(':');
    if (colon != std::string::npos)
    {
        port = hostname_and_port.substr(colon + 1);
    }
    hostname = hostname_and_port.substr(0, colon);


    /* set up sockets for the IRC client and GUI to communicate */
    int sv[2] = { -1, -1 };
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1)
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    /* fork off the IRC client and GUI processes */
    pid = fork();
    /* child */
    if (pid == 0)
    {
        try
        {
            client(sv[1], hostname, port);
        } catch (std::exception &e)
        {
            error("client: %s", e.what());
        }
        close(sv[0]);
        close(sv[1]);
    }
    /* parent */
    else if (pid != -1)
    {
        struct sigaction newaction{};
        newaction.sa_sigaction = parent_sigchldaction;
        newaction.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
        sigaction(SIGCHLD, &newaction, nullptr);

        gui(sv[0], username, password, realname);
    }
    else
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

void usage(char const *name, bool long_version)
{
    printf("Usage: %s HOSTNAME[:PORT] USERNAME PASSWORD [REALNAME]\n",
        name);
    if (long_version)
    {
        printf((
        "Connect to the IRC server on HOSTNAME:PORT\n"
        "Example: %s irc.example.com:1234 coolguy secret\n"
        "If unspecified, PORT is 6667 and REALNAME is 'realname'.\n"
        ), name);
    }
    else
    {
        printf("Try '%s --help' for more information.\n", name);
    }
}

