/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *  Data structures/constants dealing with IRC commands
 */

#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <string>
#include <vector>


/* Responses:
 *  Numeric responses to IRC messages
 */
typedef enum Responses
{
    /* Command Responses */
    RPL_WELCOME         =   1,
    RPL_YOURHOST        =   2,
    RPL_CREATED         =   3,
    RPL_MYINFO          =   4,
    RPL_BOUNCE          =   5,
    RPL_USERHOST        = 302,
    RPL_ISON            = 303,
    RPL_AWAY            = 301,
    RPL_UNAWAY          = 305,
    RPL_NOWAWAY         = 306,
    RPL_WHOISUSER       = 311,
    RPL_WHOISSERVER     = 312,
    RPL_WHOISOPERATOR   = 313,
    RPL_WHOISIDLE       = 317,
    RPL_ENDOFWHOIS      = 318,
    RPL_WHOISCHANNELS   = 319,
    RPL_WHOWASUSER      = 314,
    RPL_ENDOFWHOWAS     = 369,
    RPL_LISTSTART       = 321,
    RPL_LIST            = 322,
    RPL_LISTEND         = 323,
    RPL_UNIQOPIS        = 325,
    RPL_CHANNELMODEIS   = 324,
    RPL_NOTOPIC         = 331,
    RPL_TOPIC           = 332,
    RPL_INVITING        = 341,
    RPL_SUMMONING       = 342,
    RPL_INVITELIST      = 346,
    RPL_ENDOFINVITELIST = 347,
    RPL_EXCEPTLIST      = 348,
    RPL_ENDOFEXCEPTLIST = 349,
    RPL_VERSION         = 351,
    RPL_WHOREPLY        = 352,
    RPL_ENDOFWHO        = 315,
    RPL_NAMREPLY        = 353,
    RPL_ENDOFNAMES      = 366,
    RPL_LINKS           = 364,
    RPL_ENDOFLINKS      = 365,
    RPL_BANLIST         = 367,
    RPL_ENDOFBANLIST    = 368,
    RPL_INFO            = 371,
    RPL_ENDOFINFO       = 374,
    RPL_MOTDSTART       = 375,
    RPL_MOTD            = 372,
    RPL_ENDOFMOTD       = 376,
    RPL_YOUREOPER       = 381,
    RPL_REHASHING       = 382,
    RPL_YOURESERVICE    = 383,
    RPL_TIME            = 391,
    RPL_USERSSTART      = 392,
    RPL_USERS           = 393,
    RPL_ENDOFUSERS      = 394,
    RPL_NOUSERS         = 395,
    RPL_TRACELINK       = 200,
    RPL_TRACECONNECTING = 201,
    RPL_TRACEHANDSHAKE  = 202,
    RPL_TRACEUNKNOWN    = 203,
    RPL_TRACEOPERATOR   = 204,
    RPL_TRACEUSER       = 205,
    RPL_TRACESERVER     = 206,
    RPL_TRACESERVICE    = 207,
    RPL_TRACENEWTYPE    = 208,
    RPL_TRACECLASS      = 209,
    RPL_TRACERECONNECT  = 210,
    RPL_TRACELOG        = 261,
    RPL_TRACEEND        = 262,
    RPL_STATSLINKINFO   = 211,
    RPL_STATSCOMMANDS   = 212,
    RPL_ENDOFSTATS      = 219,
    RPL_STATSUPTIME     = 242,
    RPL_STATSOLINE      = 243,
    RPL_UMODEIS         = 221,
    RPL_SERVLIST        = 234,
    RPL_SERVLISTEND     = 235,
    RPL_LUSERCLIENT     = 251,
    RPL_LUSEROP         = 252,
    RPL_LUSERUNKNOWN    = 253,
    RPL_LUSERCHANNELS   = 254,
    RPL_LUSERME         = 255,
    RPL_ADMINME         = 256,
    RPL_ADMINLOC1       = 257,
    RPL_ADMINLOC2       = 258,
    RPL_ADMINEMAIL      = 259,
    RPL_TRYAGAIN        = 263,
    RPL_SERVICEINFO     = 231,
    RPL_SERVICE         = 233,
    RPL_NONE            = 300,
    RPL_KILLDONE        = 361,
    RPL_CLOSEEND        = 363,
    RPL_MYPORTIS        = 384,
    RPL_STATSCLINE      = 213,
    RPL_STATSILINE      = 215,
    RPL_STATSQLINE      = 217,
    RPL_STATSVLINE      = 240,
    RPL_STATSHLINE      = 244,
    RPL_STATSPING       = 246,
    RPL_STATSDLINE      = 250,
    /* END Command Responses */

    /* Error Replies */
    ERR_NOSUCHNICK          = 401,
    ERR_NOSUCHSERVER        = 402,
    ERR_NOSUCHCHANNEL       = 403,
    ERR_CANNOTSENDTOCHAN    = 404,
    ERR_TOOMANYCHANNELS     = 405,
    ERR_WASNOSUCHNICK       = 406,
    ERR_TOOMANYTARGETS      = 407,
    ERR_NOSUCHSERVICE       = 408,
    ERR_NOORIGIN            = 409,
    ERR_NORECIPIENT         = 411,
    ERR_NOTEXTTOSEND        = 412,
    ERR_NOTOPLEVEL          = 413,
    ERR_WILDTOPLEVEL        = 414,
    ERR_BADMASK             = 415,
    ERR_UNKNOWNCOMMAND      = 421,
    ERR_NOMOTD              = 422,
    ERR_NOADMININFO         = 423,
    ERR_FILEERROR           = 424,
    ERR_NONICKNAMEGIVEN     = 431,
    ERR_ERRONEUSNICKNAME    = 432,
    ERR_NICKNAMEINUSE       = 433,
    ERR_NICKCOLLISION       = 436,
    ERR_UNAVAILRESOURCE     = 437,
    ERR_USERNOTINCHANNEL    = 441,
    ERR_NOTONCHANNEL        = 442,
    ERR_USERONCHANNEL       = 443,
    ERR_NOLOGIN             = 444,
    ERR_SUMMONDISABLED      = 445,
    ERR_USERSDISABLED       = 446,
    ERR_NOTREGISTERED       = 451,
    ERR_NEEDMOREPARAMS      = 461,
    ERR_ALREADYREGISTRED    = 462,
    ERR_NOPERMFORHOST       = 463,
    ERR_PASSWDMISMATCH      = 464,
    ERR_YOUREBANNEDCREEP    = 465,
    ERR_YOUWILLBEBANNED     = 466,
    ERR_KEYSET              = 467,
    ERR_CHANNELISFULL       = 471,
    ERR_UNKNOWNMODE         = 472,
    ERR_INVITEONLYCHAN      = 473,
    ERR_BANNEDFROMCHAN      = 474,
    ERR_BADCHANNELKEY       = 475,
    ERR_BADCHANMASK         = 476,
    ERR_NOCHANMODES         = 477,
    ERR_BANLISTFULL         = 478,
    ERR_NOPRIVILEGES        = 481,
    ERR_CHANOPRIVSNEEDED    = 482,
    ERR_CANTKILLSERVER      = 483,
    ERR_RESTRICTED          = 484,
    ERR_UNIQOPPRIVSNEEDED   = 485,
    ERR_NOOPERHOST          = 491,
    ERR_UMODEUNKNOWNFLAG    = 501,
    ERR_USERSDONTMATCH      = 502,
    ERR_NOSERVICEHOST       = 492,
    /* END Error Replies */
} Responses;


/* CommandSpec:
 *  Contains information about a command
 */
typedef struct CommandSpec
{
    std::string name;
    size_t param_count,
           optional_param_count;
} CommandSpec;


/* List of IRC commands */
static std::vector<CommandSpec> const irc_commands =
{
    /* Connection Registration */
    { "PASS",       1,  0, },
    { "NICK",       1,  0, },
    { "USER",       4,  0, },
    { "OPER",       2,  0, },
    { "MODE",       2,  0, },
    { "SERVICE",    6,  0, },
    { "QUIT",       0,  1, },
    { "SQUIT",      2,  0, },
    /* Channel Operations */
    { "JOIN",       1,  1, },
    { "PART",       1,  1, },
    { "MODE",       2,  0, },
    { "TOPIC",      1,  1, },
    { "NAMES",      0,  2, },
    { "LIST",       0,  2, },
    { "INVITE",     2,  0, },
    { "KICK",       4,  1, },
    /* Message Sending */
    { "PRIVMSG",    2,  0, },
    { "NOTICE",     2,  0, },
    /* Server Queries */
    { "MOTD",       0,  1, },
    { "LUSERS",     0,  2, },
    { "VERSION",    0,  1, },
    { "STATS",      0,  2, },
    { "LINKS",      0,  2, },
    { "TIME",       0,  1, },
    { "CONNECT",    2,  1, },
    { "TRACE",      0,  1, },
    { "ADMIN",      0,  1, },
    { "INFO",       0,  1, },
    /* Service Queries */
    { "SERVLIST",   0,  2, },
    { "SQUERY",     2,  0, },
    /* User Queries */
    { "WHO",        0,  2, },
    { "WHOIS",      1,  1, },
    { "WHOWAS",     1,  2, },
    /* Misc. */
    { "KILL",       2,  0, },
    { "PING",       1,  1, },
    { "PONG",       1,  1, },
    { "ERROR",      1,  0, },
    /* Optional Features */
    { "AWAY",       0,  1, },
    { "REHASH",     0,  0, },
    { "DIE",        0,  0, },
    { "RESTART",    0,  0, },
    { "SUMMON",     1,  2, },
    { "USERS",      0,  1, },
    { "WALLOPS",    1,  0, },
    { "USERHOST",   1,  4, },
    { "ISON",       1, 14, },

    /* CAP */
    { "CAP",        0,  0, },
};


#endif

