/* See LICENSE file for copyright and license details.
 * data.c
 *
 *  DESCRIPTION
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "irc_string.h"
#include "logging.h"



/* ==[ UTILITY ]== */
/* read_until_space: return a space separated 'word' from a string,
 *                   with the number of chars read in chars_read */
char *read_until_space (char const *const str, size_t *chars_read)
{
    size_t str_len   = strlen (str),
           space_idx = 0;
    char *out = NULL;

    for (size_t i = 0; i < str_len; ++i)
    {
        space_idx = i;
        if (str[i] == ' ')
        {
            break;
        }
    }

    out = calloc (sizeof(*out), space_idx + 1);
    memcpy (out, str, space_idx);
    out[space_idx] = '\0';

    *chars_read = space_idx;

    return out;
}



/* ==[ PUBLIC FUNCTIONS ]== */
/* message_to_str: convert a message into a string (including the
 *                 CRLF message ending */
char *message_to_str (Message msg)
{
    String str = string_new();

    /* add the prefix to the string */
    if (msg.prefix != NULL)
    {
        string_add (&str, ":");
        string_add (&str, msg.prefix);
    }

    /* add the command to the string */
    string_add (&str, msg.command);

    /* add the params to the string */
    for (size_t i = 0; i < msg.param_count; ++i)
    {
        string_add (&str, " ");
        string_add (&str, msg.params[i]);
    }

    /* add the terminating CRLF */
    string_add (&str, "\r\n");

    if (str.length > 512)
    {
        error ("messages cannot exceed 512 characters!"\
               "(message is %lu chars long)", str.length);
    }
    return str.str;
}

/* str_to_message: convert a string into a message */
Message str_to_message (char const *const str)
{
    Message msg = { 0 };
    size_t str_len = strlen (str),
           str_idx = 0;

    /* prefix */
    if (str[0] == ':')
    {
        str_idx += 1;
        debug ("has prefix");

        size_t chars_read = 0;
        msg.prefix = read_until_space (str + str_idx, &chars_read);
        str_idx += chars_read + 1;

        debug ("prefix is \"%s\"", msg.prefix);
    }

    /* command */
    size_t chars_read = 0;
    msg.command = read_until_space (str + str_idx, &chars_read);
    str_idx += chars_read + 1;

    debug ("command is \"%s\"", msg.command);

    /* parameters */
    size_t allocated = 15;
    msg.params = calloc (sizeof(*msg.params), allocated);
    msg.param_count = 0;

    while (str_idx < str_len)
    {
        if (msg.param_count >= allocated)
        {
            allocated *= 2;
            msg.params = realloc (msg.params,
                sizeof(*msg.params) * allocated);
        }

        size_t chars_read = 0;
        char *param = read_until_space (str + str_idx, &chars_read);

        /* a parameter beginning with a ':' is a trailing
         * parameter; it will always be the final parameter,
         * and may contain spaces */
        if (param[0] == ':')
        {
            free (param);
            /* skip the ':' */
            str_idx++;

            chars_read = str_len - str_idx;

            param = calloc (sizeof(*param), chars_read + 1);

            memcpy (param, str + str_idx, chars_read);
            param[chars_read] = '\0';
        }

        str_idx += chars_read + 1;

        msg.params[msg.param_count] = param;
        debug ("param[%lu] is \"%s\"", msg.param_count, param);

        msg.param_count++;
    }

    msg.params = realloc (msg.params,
        sizeof(*msg.params) * msg.param_count);
    return msg;
}

