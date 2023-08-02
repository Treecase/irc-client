/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef IRCC_ARGS_HPP
#define IRCC_ARGS_HPP

#include <string>


struct Config
{
    std::string hostname, port;
    std::string username, password, realname;
};


/** Print usage information. */
void usage(char const *name, bool long_version);

/** Print version information. */
void version(void);

/** Parse command-line arguments. */
Config parse_args(int argc, char *argv[]);


#endif
