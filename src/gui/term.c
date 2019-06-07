/* See LICENSE file for copyright and license details.
 * gui.c
 *
 *  DESCRIPTION
 *
 */

/* ==[ INCLUDES ]== */
#include <stdarg.h>

#include <readline/readline.h>



/* ==[ PRIVATE FUNCTIONS ]== */
/* irc_command_generator: return possible IRC commands that match
 *  'text' (used by GNU Readline) */
static char *irc_command_generator (char const *text, int state)
{
    static size_t len;
    static size_t idx;

    if (state == 0)
    {
        len = strlen (text);
        idx = 0;
    }

    while (idx < command_count)
    {
        char *cmd = commands[idx++].name;
        if (strncasecmp (cmd, text, len) == 0)
        {
            return strdup (cmd);
        }
    }
    return NULL;
}

/* irc_complete_command: return a list of possible IRC commands
 *  that match 'text' (used by GNU Readline) */
static char **irc_complete_command (
    char const *text, int start, int end)
{
    start = start;  /* shuts gcc up about unused variables */
    end   = end;    /* shuts gcc up about unused variables */
    rl_attempted_completion_over = 1;
    return rl_completion_matches (text, irc_command_generator);
}



/* gui_init: init GUI */
void gui_init(void)
{
    /* initialize GNU Readline */
    rl_readline_name = "IRC";
    rl_attempted_completion_function = irc_complete_command;
    using_history();
}

/* gui_shutdown: shutdown GUI */
void gui_shutdown(void)
{
}

/* gui_user_print: print a message to the user screen */
void gui_user_print  (char const *const fmt, ...)
{
    va_list args;
    va_start (args, fmt);

    vprintf (fmt, args);

    va_end (args);
}

/* gui_debug_print: print a message to the debug screen */
void gui_debug_print (char const *const fmt, ...)
{
    va_list args;
    va_start (args, fmt);

    vprintf (fmt, args);

    va_end (args);
}

/* gui_user_read: read a message from the user */
char *gui_user_read(void)
{
    char *out = readline (" >");

    int old_pos = where_history(),
        idx     = 0;

    /* if the user's input is already in the
     * history, remove the old entry for it */
    if ((idx = history_search (out, 0)) != -1)
    {
        free_history_entry (remove_history (idx));
    }

    /* add user's input to the readline history */
    add_history (out);
    history_set_pos (old_pos);

    return out;
}

