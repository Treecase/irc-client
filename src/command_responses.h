/* See LICENSE file for copyright and license details.
 * command_responses.h
 *
 *  DESCRIPTION
 *
 */

#ifndef _COMMAND_RESPONSES_H
#define _COMMAND_RESPONSES_H


#include "data.h"


/* Connection Registration */
char *response_PASS (Message msg);
char *response_NICK (Message msg);
char *response_USER (Message msg);
char *response_OPER (Message msg);
char *response_MODE (Message msg);
char *response_SERVICE (Message msg);
char *response_QUIT (Message msg);
char *response_SQUIT (Message msg);

/* Channel Operations */
char *response_JOIN (Message msg);
char *response_PART (Message msg);
char *response_MODE_ch (Message msg);
char *response_TOPIC (Message msg);
char *response_NAMES (Message msg);
char *response_LIST (Message msg);
char *response_INVITE (Message msg);
char *response_KICK (Message msg);

/* Message Sending */
char *response_PRIVMSG (Message msg);
char *response_NOTICE (Message msg);

/* Server Queries */
char *response_MOTD (Message msg);
char *response_LUSERS (Message msg);
char *response_VERSION (Message msg);
char *response_STATS (Message msg);
char *response_LINKS (Message msg);
char *response_TIME (Message msg);
char *response_CONNECT (Message msg);
char *response_TRACE (Message msg);
char *response_ADMIN (Message msg);
char *response_INFO (Message msg);

/* Service Queries */
char *response_SERVLIST (Message msg);
char *response_SQUERY (Message msg);

/* User Queries */
char *response_WHO (Message msg);
char *response_WHOIS (Message msg);
char *response_WHOWAS (Message msg);

/* Misc. */
char *response_KILL (Message msg);
char *response_PING (Message msg);
char *response_PONG (Message msg);
char *response_ERROR (Message msg);

/* Optional Features */
char *response_AWAY (Message msg);
char *response_REHASH (Message msg);
char *response_DIE (Message msg);
char *response_RESTART (Message msg);
char *response_SUMMON (Message msg);
char *response_USERS (Message msg);
char *response_WALLOPS (Message msg);
char *response_USERHOST (Message msg);
char *response_ISON (Message msg);

/* CAP */
char *response_CAP (Message msg);


#endif

