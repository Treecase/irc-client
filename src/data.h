/* See LICENSE file for copyright and license details.
 * data.h
 *
 *  IRC Structs/Classes
 *
 */

#ifndef _DATA_H
#define _DATA_H

#include <stddef.h>


/* ==[ DEFINES ]== */
#ifndef VERSION
#error A version number is required
#endif

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL    0
#endif


/* ==[ GLOBALS ]== */
extern char const *CLIENT_NAME; /* defined in main.c */


/* ==[ STRUCTS ]== */
/* Message
 *  DESCRIPTION
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
    char  *prefix;
    char  *command;
    char **params;
    size_t param_count;
} Message;


/* ==[ FUNCTIONS ]== */
/* NOTE:
 *  ``IRC messages are always lines of characters terminated
 *    with a CR-LF (Carriage Return - Line Feed) pair, and
 *    these messages SHALL NOT exceed 512 characters in length,
 *    counting all characters including the trailing CR-LF.
 *    Thus, there are 510 characters maximum allowed for the
 *    command and its parameters.''
 *   --RFC 2812, 2.3 (pg 5)
 */
char *message_to_str (Message msg);
Message str_to_message (char const *const str);


#endif

