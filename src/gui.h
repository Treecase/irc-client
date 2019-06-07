/* See LICENSE file for copyright and license details.
 * gui.h
 *
 *  DESCRIPTION
 *
 */

#ifndef _GUI_H
#define _GUI_H


void gui_init(void);
void gui_shutdown(void);

void gui_user_print  (char const *const fmt, ...);
void gui_debug_print (char const *const fmt, ...);

char *gui_user_read(void);


#endif

