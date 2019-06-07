/* See LICENSE file for copyright and license details.
 * string.h
 *
 *  DESCRIPTION
 *
 */

#ifndef _IRC_STRING_H
#define _IRC_STRING_H


/* String:
 *  DESCRIPTION
 */
typedef struct String
{
    char *str;
    size_t length,
           allocated;
} String;


String string_new(void);
void string_delete (String *str);

void string_add (String *str, char const *const add);
void string_add_ch (String *str, char const add);


#endif

