/* See LICENSE file for copyright and license details.
 * command_responses.c
 *
 *  These functions return response command strings based on the given
 * command message
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "command_responses.h"
#include "commands.h"
#include "data.h"
#include "gui.h"
#include "irc_string.h"
#include "logging.h"


char *response_PASS (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_NICK (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_USER (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_OPER (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_MODE (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_SERVICE (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_QUIT (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_SQUIT (Message cmd)
{
    cmd = cmd;
    return NULL;
}


char *response_JOIN (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_PART (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_MODE_ch (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_TOPIC (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_NAMES (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_LIST (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_INVITE (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_KICK (Message cmd)
{
    cmd = cmd;
    return NULL;
}


char *response_PRIVMSG (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_NOTICE (Message cmd)
{
    String out = string_new();

    /* i starts at 1 because the 1st param of a NOTICE
     * command is always just your username */
    for (size_t i = 1; i < cmd.param_count; ++i)
    {
        string_add (&out, cmd.params[i]);
        if (i != cmd.param_count - 1)
        {
            string_add (&out, " ");
        }
    }

    if (out.str != NULL)
    {
        gui_user_print ("%s\n", out.str);
    }
    return NULL;
}


char *response_MOTD (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_LUSERS (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_VERSION (Message cmd)
{
    cmd = cmd;

    Message response = { 0 };

    response.command = strdup ("XXX");
    snprintf (response.command, 4, "%3d", RPL_VERSION);

    response.param_count = 2;
    response.params      = calloc (sizeof(*response.params),
                                   response.param_count);

    /* param 0 is in the form <version>.<debug> */
    int version_length = snprintf (NULL, 0, "%s", VERSION),
        debug_length   = snprintf (NULL, 0, "%d", DEBUG_LEVEL),
        total_length   = version_length + 1 + debug_length;

    response.params[0] = calloc (sizeof(*response.params[0]),
        total_length + 1);
    snprintf (response.params[0], total_length, "%s.%d",
        VERSION, DEBUG_LEVEL);

    /* param 1 is <server> */
    response.params[1] = strdup (CLIENT_NAME);

    return message_to_str (response);
}

char *response_STATS (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_LINKS (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_TIME (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_CONNECT (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_TRACE (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_ADMIN (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_INFO (Message cmd)
{
    cmd = cmd;
    return NULL;
}


char *response_SERVLIST (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_SQUERY (Message cmd)
{
    cmd = cmd;
    return NULL;
}


char *response_WHO (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_WHOIS (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_WHOWAS (Message cmd)
{
    cmd = cmd;
    return NULL;
}


char *response_KILL (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_PING (Message cmd)
{
    Message response = { 0 };

    response.command     = strdup ("PONG");
    response.param_count = 1;
    response.params      = calloc (sizeof(*response.params),
                                   response.param_count);
    response.params[0]   = strdup (cmd.params[0]);

    return message_to_str (response);
}

char *response_PONG (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_ERROR (Message cmd)
{
    cmd = cmd;
    return NULL;
}


char *response_AWAY (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_REHASH (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_DIE (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_RESTART (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_SUMMON (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_USERS (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_WALLOPS (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_USERHOST (Message cmd)
{
    cmd = cmd;
    return NULL;
}

char *response_ISON (Message cmd)
{
    cmd = cmd;
    return NULL;
}


void CAP_response_LS (Message msg)
{
    size_t idx = 2;

    /* CAP LS messages have a '*' as the first param
     * if they are not the final LS in a chain */
    if (strcmp (msg.params[idx], "*") == 0)
    {
        idx += 1;
    }

    /* print the list to userout */
    gui_user_print ("CAP LS -- %s\n", msg.params[idx]);
}

void CAP_response_LIST (Message msg)
{
    msg = msg;
}

void CAP_response_REQ (Message msg)
{
    msg = msg;
}

void CAP_response_ACK (Message msg)
{
    msg = msg;
}

void CAP_response_NAK (Message msg)
{
    msg = msg;
}

void CAP_response_CLEAR (Message msg)
{
    msg = msg;
}

void CAP_response_END (Message msg)
{
    msg = msg;
}

/* CapSubCommand:
 *  DESCRIPTION
 */
struct CapSubCommand
{
    char *name;
    void (*response) (Message msg);
};

char *response_CAP (Message cmd)
{
    static struct CapSubCommand const subcommands[] =\
    {
        { "LS",     CAP_response_LS,    },
        { "LIST",   CAP_response_LIST,  },
        { "REQ",    CAP_response_REQ,   },
        { "ACK",    CAP_response_ACK,   },
        { "NAK",    CAP_response_NAK,   },
        { "CLEAR",  CAP_response_CLEAR, },
        { "END",    CAP_response_END,   },
    };

    for (size_t i = 0;
        i < sizeof(subcommands) / sizeof(*subcommands); ++i)
    {
        if (strcasecmp (cmd.params[1], subcommands[i].name) == 0)
        {
            subcommands[i].response (cmd);
            break;
        }
    }

    return NULL;
}

