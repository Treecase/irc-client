/* See LICENSE file for copyright and license details.
 * string.c
 *
 *  DESCRIPTION
 *
 */

#include <stdlib.h>
#include <string.h>

#include "irc_string.h"



/* string_new: create a new String */
String string_new(void)
{
    String out = { 0 };

    out.str       = NULL;
    out.length    = 0;
    out.allocated = 0;

    return out;
}

/* string_delete: free a String */
void string_delete (String *str)
{
    free (str->str);
    str->str = NULL;
    str->length    = 0;
    str->allocated = 0;
}

/* string_add: append to a string */
void string_add (String *str, char const *const add)
{
    size_t add_len = strlen (add);

    /* resize the string if needed */
    while (str->length + add_len >= str->allocated)
    {
        /* make sure we're actually allocating more space */
        if (str->allocated <= 0)
        {
            str->allocated = 16;
        }

        str->str = realloc (str->str,
            sizeof(*str->str) * (str->allocated * 2));

        memset (str->str + str->allocated, 0, str->allocated);

        str->allocated *= 2;
    }

    /* append the string */
    memcpy (str->str + str->length,
        add, add_len);

    str->length += add_len;

    /* ensure the string ends with a NUL */
    str->str[str->length] = '\0';
}

/* string_add_ch: append a character to a string */
void string_add_ch (String *str, char const add)
{
    /* resize the string if needed */
    while (str->length + 1 >= str->allocated)
    {
        /* make sure we're actually allocating more space */
        if (str->allocated <= 0)
        {
            str->allocated = 16;
        }

        str->str = realloc (str->str,
            sizeof(*str->str) * (str->allocated * 2));

        memset (str->str + str->allocated, 0, str->allocated);

        str->allocated *= 2;
    }

    /* append the string */
    str->str[str->length] = add;
    str->length += 1;

    /* ensure the string ends with a NUL */
    str->str[str->length] = '\0';
}

