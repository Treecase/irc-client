/* See LICENSE file for copyright and license details.
 * command_responses.c
 *
 *  These functions return response command strings based on the given
 * command message
 *
 */

#include <stdlib.h>
#include <string.h>

#include "command_responses.h"
#include "commands.h"
#include "data.h"
#include "logging.h"


char *response_PASS (Message cmd)
{
    return NULL;
}

char *response_NICK (Message cmd)
{
    return NULL;
}

char *response_USER (Message cmd)
{
    return NULL;
}

char *response_OPER (Message cmd)
{
    return NULL;
}

char *response_MODE (Message cmd)
{
    return NULL;
}

char *response_SERVICE (Message cmd)
{
    return NULL;
}

char *response_QUIT (Message cmd)
{
    return NULL;
}

char *response_SQUIT (Message cmd)
{
    return NULL;
}


char *response_JOIN (Message cmd)
{
    return NULL;
}

char *response_PART (Message cmd)
{
    return NULL;
}

char *response_MODE_ch (Message cmd)
{
    return NULL;
}

char *response_TOPIC (Message cmd)
{
    return NULL;
}

char *response_NAMES (Message cmd)
{
    return NULL;
}

char *response_LIST (Message cmd)
{
    return NULL;
}

char *response_INVITE (Message cmd)
{
    return NULL;
}

char *response_KICK (Message cmd)
{
    return NULL;
}


char *response_PRIVMSG (Message cmd)
{
    return NULL;
}

char *response_NOTICE (Message cmd)
{
    return NULL;
}


char *response_MOTD (Message cmd)
{
    return NULL;
}

char *response_LUSERS (Message cmd)
{
    return NULL;
}

char *response_VERSION (Message cmd)
{
    Message response = { 0 };

    response.command = strdup ("XXX");
    snprintf (response.command, 4, "%3d", RPL_VERSION);

    response.param_count = 2;
    response.params      = calloc (sizeof(*response.params),
                                   response.param_count);

    /* param 0 is in the form <version>.<debug> */
    int version_length = snprintf (NULL, 0, "%s", VERSION),
        debug_length   = snprintf (NULL, 0, "%d", DEBUG_LEVEL),
        total_length   = version_length + 1 + debug_length + 1;

    response.params[0] = malloc (total_length + 1);
    snprintf (response.params[0], total_length, "%s.%d", VERSION, DEBUG_LEVEL);

    /* param 1 is <server> */
    response.params[1] = strdup (CLIENT_NAME);

    return message_to_str (response);
}

char *response_STATS (Message cmd)
{
    return NULL;
}

char *response_LINKS (Message cmd)
{
    return NULL;
}

char *response_TIME (Message cmd)
{
    return NULL;
}

char *response_CONNECT (Message cmd)
{
    return NULL;
}

char *response_TRACE (Message cmd)
{
    return NULL;
}

char *response_ADMIN (Message cmd)
{
    return NULL;
}

char *response_INFO (Message cmd)
{
    return NULL;
}


char *response_SERVLIST (Message cmd)
{
    return NULL;
}

char *response_SQUERY (Message cmd)
{
    return NULL;
}


char *response_WHO (Message cmd)
{
    return NULL;
}

char *response_WHOIS (Message cmd)
{
    return NULL;
}

char *response_WHOWAS (Message cmd)
{
    return NULL;
}


char *response_KILL (Message cmd)
{
    return NULL;
}

char *response_PING (Message cmd)
{
    Message response = { 0 };

    response.command     = strdup ("PONG");
    response.param_count = 1;
    response.params      = calloc (sizeof(*response.params),
                                   response.param_count);
    response.params[0]   = cmd.params[0];

    return message_to_str (response);
}

char *response_PONG (Message cmd)
{
    return NULL;
}

char *response_ERROR (Message cmd)
{
    return NULL;
}


char *response_AWAY (Message cmd)
{
    return NULL;
}

char *response_REHASH (Message cmd)
{
    return NULL;
}

char *response_DIE (Message cmd)
{
    return NULL;
}

char *response_RESTART (Message cmd)
{
    return NULL;
}

char *response_SUMMON (Message cmd)
{
    return NULL;
}

char *response_USERS (Message cmd)
{
    return NULL;
}

char *response_WALLOPS (Message cmd)
{
    return NULL;
}

char *response_USERHOST (Message cmd)
{
    return NULL;
}

char *response_ISON (Message cmd)
{
    return NULL;
}

