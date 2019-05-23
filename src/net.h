/* See LICENSE file for copyright and license details.
 * net.h
 *
 *  DESCRIPTION
 *
 */

#ifndef _NET_H
#define _NET_H


/* ==[ FUNCTIONS ]== */
void send_message (int fd, char const *const msg);
int get_byte_from_inet (int fd);
char *receive_message (int fd);


#endif

