/*
 * DESCRIPTION
 *
 */

#include <string.h>
#include <stdlib.h>

#include "logging.h"
#include "data.h"



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
    memcpy (out, str, space_idx + 1);
    out[space_idx] = '\0';

    *chars_read = space_idx;

    return out;
}



/* ==[ PUBLIC FUNCTIONS ]== */
/* message_to_str: convert a message into a string (including the
 *                 CRLF message ending */
char *message_to_str (Message msg)
{
    char *str = NULL;
    size_t str_len = 0;

    /* add the prefix to the string */
    if (msg.prefix != NULL)
    {
        size_t prefix_len = strlen (msg.prefix) + 1;

        str = realloc (str, str_len + prefix_len);
        str[0] = ':';
        memcpy (str + 1, msg.prefix, prefix_len - 1);

        str_len += prefix_len;
    }

    /* add the command to the string */
    size_t msg_len = strlen (msg.command);
    str = realloc (str, str_len + msg_len);
    memcpy (str + str_len, msg.command, msg_len);
    str_len += msg_len;

    /* add the params to the string */
    for (size_t i = 0; i < msg.param_count; ++i)
    {
        size_t param_len = strlen (msg.params[i]);

        str = realloc (str, str_len + param_len + 1);
        str[str_len] = ' ';
        memcpy (str + str_len + 1, msg.params[i], param_len);

        str_len += param_len + 1;
    }

    /* add the terminating CRLF */
    str = realloc (str, str_len + 3);
    str_len += 3;
    str[str_len-3] = '\r';
    str[str_len-2] = '\n';
    str[str_len-1] = '\0';

    if (str_len > 512)
    {
        error ("messages cannot exceed 512 characters!"\
               "(message is %lu chars long)", str_len);
    }
    return str;
}

/* str_to_message: convert a string into a message */
Message str_to_message (char const *const str)
{
    Message msg = { 0 };
    size_t str_len = strlen (str),
           str_idx = 0;

    if (str[0] == ':')
    {
        str_idx += 1;
        debug ("has prefix");

        size_t chars_read = 0;
        msg.prefix = read_until_space (str + str_idx, &chars_read);
        str_idx += chars_read + 1;

        debug ("prefix is '%s'", msg.prefix);
    }

    size_t chars_read = 0;
    msg.command = read_until_space (str + str_idx, &chars_read);
    str_idx += chars_read + 1;

    debug ("command is '%s'", msg.command);

    msg.params = calloc (sizeof(*msg.params), 15);

    size_t param_count = 0;
    while (str_idx < str_len)
    {
        if (param_count >= 15)
        {
            error ("too many parameters! (%lu/15)", param_count);
            break;
        }

        size_t chars_read = 0;
        char *param = read_until_space (str + str_idx, &chars_read);

        /* a parameter beginning with a ':' is a trailing
         * parameter; it will always be the final parameter,
         * and may contain spaces */
        if (param[0] == ':')
        {
            free (param);
            /* skip the leading ':' */
            str_idx++;

            chars_read = str_len - str_idx - 2;
            param = calloc (sizeof(*param), chars_read + 1);

            /* we only copy `str_len - str_idx - 2` characters
             * so that the terminating CRLF is not included */
            memcpy (param, str + str_idx, chars_read);
            param[chars_read] = '\0';

            chars_read += 2;
        }

        str_idx += chars_read + 1;

        msg.params[param_count] = param;
        debug ("param[%lu] is '%s'", param_count, param);

        param_count++;
    }

    msg.params = realloc (msg.params,
                          sizeof(*msg.params) * param_count);
    msg.param_count = param_count;
    return msg;
}

