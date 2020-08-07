/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *  IRC Structs/Classes
 */

#ifndef _DATA_H
#define _DATA_H

#include <cstddef>

#include <string>
#include <vector>


/* 'VERSION' must be defined at build time */
#ifndef VERSION
#error A version number is required
#endif

/* 'DEBUG_LEVEL' can be defined at build time */
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL    0
#endif


extern char const *CLIENT_NAME; /* defined in main.c */


/* Message
 *  An IRC message
 */
typedef struct Message
{
    /* NOTE:
     *  ``Clients SHOULD NOT use a prefix when sending a
     *    message; if they use one, the only valid prefix
     *    is the registered nickname associated with the
     *    client.''
     *  --RFC 2812, 2.3 (pg 5)
     */
    std::string *prefix;
    std::string command;
    std::vector<std::string> params;


    /* NOTE:
     *  ``IRC messages are always lines of characters terminated
     *    with a CR-LF (Carriage Return - Line Feed) pair, and
     *    these messages SHALL NOT exceed 512 characters in length,
     *    counting all characters including the trailing CR-LF.
     *    Thus, there are 510 characters maximum allowed for the
     *    command and its parameters.''
     *   --RFC 2812, 2.3 (pg 5)
     */
    operator std::string() const;

    Message(
        std::string *prefix_,
        std::string command_,
        std::vector<std::string> params_);
    Message(std::string &str);

} Message;


#endif

