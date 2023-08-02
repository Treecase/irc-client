/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "args.hpp"
#include "config.hpp"

#include <cstdio>

#include <getopt.h>


void usage(char const *name, bool long_version)
{
    printf("Usage: %s HOSTNAME[:PORT] USERNAME PASSWORD [REALNAME]\n",
        name);
    if (long_version)
    {
        printf((
        "Connect to the IRC server on HOSTNAME:PORT\n"
        "Example: %s irc.example.com:1234 coolguy secret\n"
        "\n"
        "Miscellaneous:\n"
        "  --help     display this help and exit\n"
        "  --version  output version information and exit\n"
        "\n"
        "If unspecified, PORT is 6667 and REALNAME is 'realname'.\n"
        ), name);
    }
    else
    {
        printf("Try '%s --help' for more information.\n", name);
    }
}


void version(void)
{
    puts(IRCC_NAME " " IRCC_VERSION);
}


Config parse_args(int argc, char *argv[])
{
    Config config{
        .hostname="",
        .port="6667",
        .username="",
        .password="",
        .realname="realname",
    };

    char const *const optstring = "";
    struct option const longopts[] = {
        {"help", no_argument, nullptr, 0},
        {"version", no_argument, nullptr, 0},
        {0, 0, 0, 0},
    };
    int longindex;

    // Parse command line options
    int opt;
    while (
        (opt = getopt_long(argc, argv, optstring, longopts, &longindex)) != -1)
    {
        switch (opt)
        {
        // Unknown option
        case '?':
            exit(EXIT_FAILURE);
            break;

        // Long option
        case 0:
            switch (longindex)
            {
            // --help
            case 0:
                usage(argv[0], true);
                exit(EXIT_SUCCESS);
                break;
            // --version
            case 1:
                version();
                exit(EXIT_SUCCESS);
                break;
            }
            break;
        }
    }


    auto const arg_count = argc - optind;
    if (arg_count < 3 || arg_count > 4)
    {
        usage(argv[0], false);
        exit(EXIT_FAILURE);
    }


    config.hostname = argv[optind];
    auto const portsep = config.hostname.find(':');
    if (portsep != std::string::npos)
    {
        config.hostname = config.hostname.substr(portsep);
        config.port = config.hostname.substr(portsep + 1);
    }

    config.username = argv[optind + 1];

    config.password = argv[optind + 2];

    if (arg_count > 3)
        config.realname = argv[optind + 3];

    return config;
}
