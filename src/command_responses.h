/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 * command_responses.h
 *
 *  IRC command response callbacks
 *
 */

#ifndef _COMMAND_RESPONSES_H
#define _COMMAND_RESPONSES_H


#include "data.h"

#include <string>
#include <unordered_map>


/* Response:
 *  IRC message response
 */
typedef struct Response
{
    enum
    {
        Error,
        Client,
        IRC,
    } type;
    std::string str;
} Response;

typedef Response *(*CmdResponse)(Message);


/* Connection Registration */
Response *response_PASS(Message msg);
Response *response_NICK(Message msg);
Response *response_USER(Message msg);
Response *response_OPER(Message msg);
Response *response_MODE(Message msg);
Response *response_SERVICE(Message msg);
Response *response_QUIT(Message msg);
Response *response_SQUIT(Message msg);

/* Channel Operations */
Response *response_JOIN(Message msg);
Response *response_PART(Message msg);
Response *response_MODE_ch(Message msg);
Response *response_TOPIC(Message msg);
Response *response_NAMES(Message msg);
Response *response_LIST(Message msg);
Response *response_INVITE(Message msg);
Response *response_KICK(Message msg);

/* Message Sending */
Response *response_PRIVMSG(Message msg);
Response *response_NOTICE(Message msg);

/* Server Queries */
Response *response_MOTD(Message msg);
Response *response_LUSERS(Message msg);
Response *response_VERSION(Message msg);
Response *response_STATS(Message msg);
Response *response_LINKS(Message msg);
Response *response_TIME(Message msg);
Response *response_CONNECT(Message msg);
Response *response_TRACE(Message msg);
Response *response_ADMIN(Message msg);
Response *response_INFO(Message msg);

/* Service Queries */
Response *response_SERVLIST(Message msg);
Response *response_SQUERY(Message msg);

/* User Queries */
Response *response_WHO(Message msg);
Response *response_WHOIS(Message msg);
Response *response_WHOWAS(Message msg);

/* Misc. */
Response *response_KILL(Message msg);
Response *response_PING(Message msg);
Response *response_PONG(Message msg);
Response *response_ERROR(Message msg);

/* Optional Features */
Response *response_AWAY(Message msg);
Response *response_REHASH(Message msg);
Response *response_DIE(Message msg);
Response *response_RESTART(Message msg);
Response *response_SUMMON(Message msg);
Response *response_USERS(Message msg);
Response *response_WALLOPS(Message msg);
Response *response_USERHOST(Message msg);
Response *response_ISON(Message msg);

/* CAP */
Response *response_CAP(Message msg);



/* IRC command responses mapped to their respective commands */
static std::unordered_map<std::string, CmdResponse>
    irc_command_responses =
{
    /* Connection Registration */
    { "PASS",     response_PASS,     },
    { "NICK",     response_NICK,     },
    { "USER",     response_USER,     },
    { "OPER",     response_OPER,     },
    { "MODE",     response_MODE,     },
    { "SERVICE",  response_SERVICE,  },
    { "QUIT",     response_QUIT,     },
    { "SQUIT",    response_SQUIT,    },
    /* Channel Operations */
    { "JOIN",     response_JOIN,     },
    { "PART",     response_PART,     },
    { "MODE",     response_MODE_ch,  },
    { "TOPIC",    response_TOPIC,    },
    { "NAMES",    response_NAMES,    },
    { "LIST",     response_LIST,     },
    { "INVITE",   response_INVITE,   },
    { "KICK",     response_KICK,     },
    /* Message Sending */
    { "PRIVMSG",  response_PRIVMSG,  },
    { "NOTICE",   response_NOTICE,   },
    /* Server Queries */
    { "MOTD",     response_MOTD,     },
    { "LUSERS",   response_LUSERS,   },
    { "VERSION",  response_VERSION,  },
    { "STATS",    response_STATS,    },
    { "LINKS",    response_LINKS,    },
    { "TIME",     response_TIME,     },
    { "CONNECT",  response_CONNECT,  },
    { "TRACE",    response_TRACE,    },
    { "ADMIN",    response_ADMIN,    },
    { "INFO",     response_INFO,     },
    /* Service Queries */
    { "SERVLIST", response_SERVLIST, },
    { "SQUERY",   response_SQUERY,   },
    /* User Queries */
    { "WHO",      response_WHO,      },
    { "WHOIS",    response_WHOIS,    },
    { "WHOWAS",   response_WHOWAS,   },
    /* Misc. */
    { "KILL",     response_KILL,     },
    { "PING",     response_PING,     },
    { "PONG",     response_PONG,     },
    { "ERROR",    response_ERROR,    },
    /* Optional Features */
    { "AWAY",     response_AWAY,     },
    { "REHASH",   response_REHASH,   },
    { "DIE",      response_DIE,      },
    { "RESTART",  response_RESTART,  },
    { "SUMMON",   response_SUMMON,   },
    { "USERS",    response_USERS,    },
    { "WALLOPS",  response_WALLOPS,  },
    { "USERHOST", response_USERHOST, },
    { "ISON",     response_ISON,     },

    /* CAP */
    { "CAP",        response_CAP,      },
};
#endif

