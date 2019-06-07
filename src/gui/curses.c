/* See LICENSE file for copyright and license details.
 * gui.c
 *
 *  DESCRIPTION
 *
 */
/* TODO:
 *  - output window scrolling
 *   > requires character-by-character user input
 *  - use proper queue depending upon which win is passed into `write_to_window'
 */

/* ==[ INCLUDES ]== */
#include <ctype.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include "../gui.h"
#include "../logging.h"
#include "../queue.h"


/* ==[ CONSTANTS ]== */
/* technically, the window variables aren't constant, since you can't
 * tell the window size at compile time, but they shouldn't ever be
 * assigned to by any function other than gui_init() */
int user_in_win_x      = 0,
    user_in_win_y      = 0;
int user_in_win_width  = 80,
    user_in_win_height = 3;

int user_out_win_x      = 0,
    user_out_win_y      = 3;
int user_out_win_width  = 40,
    user_out_win_height = 25;

int debug_win_x      = 0,
    debug_win_y      = 3;
int debug_win_width  = 40,
    debug_win_height = 25;

int const colour_table[] =\
{
    COLOR_WHITE,    /* white */
    COLOR_BLACK,    /* black */
    COLOR_BLUE,     /* blue */
    COLOR_GREEN,    /* green */
    COLOR_RED,      /* light red */
    COLOR_RED,      /* brown */
    COLOR_MAGENTA,  /* purple */
    COLOR_YELLOW,   /* orange */
    COLOR_YELLOW,   /* yellow */
    COLOR_GREEN,    /* light green */
    COLOR_CYAN,     /* cyan */
    COLOR_CYAN,     /* light cyan */
    COLOR_BLUE,     /* light blue */
    COLOR_MAGENTA,  /* pink */
    COLOR_WHITE,    /* grey */
    COLOR_WHITE,    /* light grey */
};


/* ==[ VARIABLES ]== */
static pthread_mutex_t gui_mutex = PTHREAD_MUTEX_INITIALIZER;

static WINDOW *user_in_win  = NULL,
              *user_out_win = NULL,
              *debug_win    = NULL;

Queue user_out_buffer = { 0 };
Queue debug_buffer    = { 0 };

static short pair_number = 1;


/* ==[ FUNCTIONS ]== */
void write_to_window (WINDOW *win, char const *const str);
void print_to_window (WINDOW *win, char const *const fmt, va_list ap);



/* gui_init: init GUI */
void gui_init(void)
{
    pthread_mutex_lock (&gui_mutex);

    /* start up ncurses */
    initscr();

    /* use colours */
    if (has_colors() == TRUE)
    {
        start_color();

        /* set custom colours if possible */
        if (can_change_color() == TRUE)
        {
            /* TODO: */
        }
    }
    /* TODO: handle non-colourful terminals instead of just exiting */
    else
    {
        endwin();
        fatal ("Your terminal does not support colours!");
    }

    /* set the window dimensions and locations */
    /* width */
    user_in_win_width  = COLS;
    user_out_win_width = COLS / 2;
    debug_win_width    = COLS / 2;

    /* height */
    user_in_win_height  = 3;
    user_out_win_height = LINES - user_in_win_height;
    debug_win_height    = LINES - user_in_win_height;

    /* x */
    user_in_win_x  = 0;
    user_out_win_x = 0;
    debug_win_x    = COLS / 2;

    /* y */
    user_in_win_y  = 0;
    user_out_win_y = user_in_win_height;
    debug_win_y    = user_in_win_height;
    /* END window dimensions */


    /* create the windows */
    user_in_win  = newwin (user_in_win_height, user_in_win_width,
        user_in_win_y, user_in_win_x);
    user_out_win = newwin (user_out_win_height, user_out_win_width,
        user_out_win_y, user_out_win_x);
    debug_win    = newwin (debug_win_height, debug_win_width,
        debug_win_y, debug_win_x);

    /* make the windows look pretty */
    box (user_in_win , 0, 0);
    box (user_out_win, 0, 0);
    box (debug_win   , 0, 0);

    /* move the cursor to the appropriate location for each window */
    wmove (user_in_win , 1, 1);
    wmove (user_out_win, 1, 1);
    wmove (debug_win   , 1, 1);

    /* draw to the screen */
    wrefresh (user_in_win);
    wrefresh (user_out_win);
    wrefresh (debug_win);

    /* history */
    user_out_buffer = queue_new();
    debug_buffer    = queue_new();

    pthread_mutex_unlock (&gui_mutex);
}

/* gui_shutdown: shutdown GUI */
void gui_shutdown(void)
{
    pthread_mutex_lock (&gui_mutex);

    /* free the output and debug history buffers */
    queue_delete (&user_out_buffer);
    queue_delete (&debug_buffer);


    /* delete the windows */
    delwin (user_in_win);
    delwin (user_out_win);
    delwin (debug_win);
    user_in_win  = NULL;
    user_out_win = NULL;
    debug_win    = NULL;

    /* shutdown ncurses */
    endwin();

    pthread_mutex_unlock (&gui_mutex);
}

/* gui_user_print: print a message to the user screen */
void gui_user_print  (char const *const fmt, ...)
{
    pthread_mutex_lock (&gui_mutex);

    va_list ap;
    va_start (ap, fmt);

    print_to_window (user_out_win, fmt, ap);

    va_end (ap);

    pthread_mutex_unlock (&gui_mutex);
}

/* gui_debug_print: print a message to the debug screen */
void gui_debug_print (char const *const fmt, ...)
{
    pthread_mutex_lock (&gui_mutex);

    va_list ap;
    va_start (ap, fmt);

    print_to_window (debug_win, fmt, ap);

    va_end (ap);

    pthread_mutex_unlock (&gui_mutex);
}

/* gui_user_read: read a message from the user */
char *gui_user_read(void)
{
    pthread_mutex_lock (&gui_mutex);

    char *str = calloc (sizeof(*str), COLS+1);
    mvwaddstr (user_in_win, 0, 0, "INPUT");
    wmove (user_in_win, 1, 1);

    pthread_mutex_unlock (&gui_mutex);


    wgetnstr (user_in_win, str, COLS);


    pthread_mutex_lock (&gui_mutex);

    wclear (user_in_win);
    box (user_in_win, 0, 0);
    wrefresh (user_in_win);

    pthread_mutex_unlock (&gui_mutex);
    if (str[0] == '')
    {
        return NULL;
    }
    else
    {
        return str;
    }
}


/* write_to_window: generic window writing function */
void write_to_window (WINDOW *win, char const *const str)
{
    bool bolded    = false,
         underline = false,
         italics   = false,
         reverse   = false;

    int y = 0,
        x = 0;
    int height = 0,
        width  = 0;

    getyx (win, y, x);
    getmaxyx (win, height, width);

    for (size_t i = 0; i < strlen (str); ++i)
    {
        getyx (win, y, x);

        switch (str[i])
        {
        /* newlines move the cursor down 1 row */
        case '\n':
          { /* don't print past the bottom of the screen */
            if (y + 1 < height)
            {
                wmove (win, y+1, 1);
            }
            /* scroll down */
            else
            {
                /* reset the window */
                wclear (win);
                box (win, 0, 0);
                wmove (win, 1, 1);

                /* calculate the offset into the Queue */
                size_t end = 0;
                size_t lines_to_print = height - 2 - 2;
                if (user_out_buffer.end < lines_to_print)
                {
                    end = user_out_buffer.size
                        - (lines_to_print - user_out_buffer.end);
                }
                else
                {
                    end = user_out_buffer.end - lines_to_print;
                }

                /* scroll down one line at a time
                 * once the screen is full */
                for (size_t i = end; i != user_out_buffer.end;)
                {
                    write_to_window (win, user_out_buffer.queue[i]);

                    if (i >= user_out_buffer.size - 1)
                    {
                        i = 0;
                    }
                    else
                    {
                        i++;
                    }
                }
            }

            /* new line, so turn off formatting */
            wattrset (win, A_NORMAL);
            bolded    = false;
            underline = false;
            italics   = false;
            reverse   = false;
          } break;

        /* bold */
        case 0x02:  /* aka ^B */
          {
            bolded = !bolded;
            if (bolded)
            {
                wattron (win, A_BOLD);
            }
            else
            {
                wattroff (win, A_BOLD);
            }
          } break;

        /* colours */
        case 0x03:  /* aka ^C */
          {
            char foreground[2] = { 0,0 },
                 background[2] = { 0,0 };
            bool fg = false,
                 bg = false;

            /* get foreground colours */
            if (isdigit (str[i+1]))
            {
                foreground[0] = str[++i];
                if (isdigit (str[i+1]))
                {
                    foreground[1] = str[++i];
                }
                fg = true;

                /* get background colours (optional, marked by ',') */
                if (str[i+1] == ',')
                {
                    i++;
                    if (isdigit (str[i+1]))
                    {
                        background[0] = str[++i];
                        if (isdigit (str[i+1]))
                        {
                            background[1] = str[++i];
                        }
                        bg = true;
                    }
                    /* next char was not a digit, so the comma wasn't
                     * marking a background colour; unparse the ',' */
                    else
                    {
                        i--;
                    }
                }
            }

            if (fg)
            {
                short fg_colour = strtol (foreground, NULL, 10);
                short bg_colour = 1;

                if (bg)
                {
                    bg_colour = strtol (background, NULL, 10);
                }

                init_pair (pair_number,
                    colour_table[fg_colour],
                    colour_table[bg_colour]);

                wattron (win, COLOR_PAIR(pair_number));
                debug ("color");

                pair_number = (pair_number + 1) % COLOR_PAIRS;
            }
            /* a ^C with no colour afterwards turns off colours */
            else
            {
                wattroff (win, COLOR_PAIR(pair_number-1));
            }
          } break;

        /* clear formatting */
        case 0x0F:  /* aka ^O */
          {
            wattrset (win, A_NORMAL);
            bolded    = false;
            underline = false;
            italics   = false;
            reverse   = false;
          } break;

        /* reverse */
        case 0x16:  /* aka ^R */
          {
            reverse = !reverse;
            if (reverse)
            {
                wattron (win, A_REVERSE);
            }
            else
            {
                wattroff (win, A_REVERSE);
            }
          } break;

        /* italics */
        case 0x1D:  /* aka ^I */
          {
            italics = !italics;
            if (italics)
            {
                wattron (win, A_DIM);
            }
            else
            {
                wattroff (win, A_DIM);
            }
          } break;

        /* underline */
        case 0x1F:  /* aka ^U */
          {
            underline = !underline;
            if (underline)
            {
                wattron (win, A_UNDERLINE);
            }
            else
            {
                wattroff (win, A_UNDERLINE);
            }
          } break;


        /* regular character, just print */
        default:
          { /* don't print past the right side of the screen */
            if (x + 1 < width)
            {
                waddch (win, str[i]);
            }
          } break;
        }
    }
}

/* print_to_window: print to a window */
void print_to_window (WINDOW *win, char const *const fmt, va_list ap)
{
    va_list ap_tmp;

    int   length = 0;
    char *str    = NULL;

    va_copy (ap_tmp, ap);

    length = vsnprintf (NULL, 0, fmt, ap_tmp);
    str = calloc (sizeof(*str), length + 1);
    vsprintf (str, fmt, ap);

    debug ("str: %s", str);

    queue_push (&user_out_buffer, str);
    write_to_window (win, str);
    wrefresh (win);
    free (str);

    va_end (ap_tmp);
}

