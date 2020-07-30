/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *
 *  IRC command response callbacks
 */

#include "command_responses.h"
#include "commands.h"
#include "data.h"
#include "logging.h"

#include <string>
#include <vector>


/* don't warn about unused parameters */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"



Response *response_PASS(Message cmd)
{
    return nullptr;
}

Response *response_NICK(Message cmd)
{
    return nullptr;
}

Response *response_USER(Message cmd)
{
    return nullptr;
}

Response *response_OPER(Message cmd)
{
    return nullptr;
}

Response *response_MODE(Message cmd)
{
    return nullptr;
}

Response *response_SERVICE(Message cmd)
{
    return nullptr;
}

Response *response_QUIT(Message cmd)
{
    Response *response = new Response{};
    response->type = Response::Client;
    response->str =\
        "QUIT "
        + cmd.prefix->substr(0, cmd.prefix->find('!'))
        + " :"
        + cmd.params.back();

    return response;
}

Response *response_SQUIT(Message cmd)
{
    return nullptr;
}


Response *response_JOIN(Message cmd)
{
    auto name = *cmd.prefix;
    auto split_idx = name.find('!');

    Response *response = new Response{};
    response->type = Response::Client;
    response->str =\
        "JOIN "
        + name.substr(0, split_idx)
        + " "
        + name.substr(split_idx + 1)
        + " "
        + cmd.params.back();

    return response;
}

Response *response_PART(Message cmd)
{
    Response *response = new Response{};
    response->type = Response::Client;
    response->str =\
        "PART "
        + cmd.prefix->substr(0, cmd.prefix->find('!'))
        + " "
        + cmd.params.back();

    return response;
}

Response *response_MODE_ch(Message cmd)
{
    return nullptr;
}

Response *response_TOPIC(Message cmd)
{
    return nullptr;
}

Response *response_NAMES(Message cmd)
{
    return nullptr;
}

Response *response_LIST(Message cmd)
{
    return nullptr;
}

Response *response_INVITE(Message cmd)
{
    return nullptr;
}

Response *response_KICK(Message cmd)
{
    return nullptr;
}


Response *response_PRIVMSG(Message cmd)
{
    Response *response = new Response{};
    response->type = Response::Client;
    response->str =\
        "PRIVATE "
        + cmd.prefix->substr(0, cmd.prefix->find('!'))
        + " :"
        + cmd.params.back();

    return response;
}

Response *response_NOTICE(Message cmd)
{
    Response *response = nullptr;

    std::string out = cmd.params.back();

    if (!out.empty())
    {
        response = new Response{};
        response->type = Response::Client;
        response->str = "NOTICE :" + out;
    }

    return response;
}


Response *response_MOTD(Message cmd)
{
    return nullptr;
}

Response *response_LUSERS(Message cmd)
{
    return nullptr;
}

Response *response_VERSION(Message cmd)
{
    Response *response = new Response{};
    response->type = Response::IRC;
    response->str = std::string{
        Message{
            nullptr,
            std::to_string(RPL_VERSION),
            {
                std::string(VERSION)
                    + "."
                    + std::to_string(DEBUG_LEVEL),
                CLIENT_NAME}}};
    return response;
}

Response *response_STATS(Message cmd)
{
    return nullptr;
}

Response *response_LINKS(Message cmd)
{
    return nullptr;
}

Response *response_TIME(Message cmd)
{
    return nullptr;
}

Response *response_CONNECT(Message cmd)
{
    return nullptr;
}

Response *response_TRACE(Message cmd)
{
    return nullptr;
}

Response *response_ADMIN(Message cmd)
{
    return nullptr;
}

Response *response_INFO(Message cmd)
{
    return nullptr;
}


Response *response_SERVLIST(Message cmd)
{
    return nullptr;
}

Response *response_SQUERY(Message cmd)
{
    return nullptr;
}


Response *response_WHO(Message cmd)
{
    return nullptr;
}

Response *response_WHOIS(Message cmd)
{
    return nullptr;
}

Response *response_WHOWAS(Message cmd)
{
    return nullptr;
}


Response *response_KILL(Message cmd)
{
    return nullptr;
}

Response *response_PING(Message cmd)
{
    Response *response = new Response{};
    response->type = Response::IRC;
    response->str = std::string{
        Message{nullptr, "PONG", {cmd.params[0]}}};

    return response;
}

Response *response_PONG(Message cmd)
{
    return nullptr;
}

Response *response_ERROR(Message cmd)
{
    Response *response = new Response{};
    response->type = Response::Client;
    response->str = "ERROR :" + cmd.params.back();

    return response;
}


Response *response_AWAY(Message cmd)
{
    return nullptr;
}

Response *response_REHASH(Message cmd)
{
    return nullptr;
}

Response *response_DIE(Message cmd)
{
    return nullptr;
}

Response *response_RESTART(Message cmd)
{
    return nullptr;
}

Response *response_SUMMON(Message cmd)
{
    return nullptr;
}

Response *response_USERS(Message cmd)
{
    return nullptr;
}

Response *response_WALLOPS(Message cmd)
{
    return nullptr;
}

Response *response_USERHOST(Message cmd)
{
    return nullptr;
}

Response *response_ISON(Message cmd)
{
    return nullptr;
}


Response *CAP_response_LS(Message cmd)
{
    size_t idx = 2;

    /* CAP LS messages have a '*' as the first param
     * if they are not the final LS in a chain */
    if (cmd.params[idx] == "*")
    {
        idx += 1;
    }

    Response *response = new Response{};
    response->type = Response::Client;
    response->str = "NOTICE :CAP.LS " + cmd.params.back();

    return response;
}

Response *CAP_response_LIST(Message cmd)
{
    return nullptr;
}

Response *CAP_response_REQ(Message cmd)
{
    return nullptr;
}

Response *CAP_response_ACK(Message cmd)
{
    return nullptr;
}

Response *CAP_response_NAK(Message cmd)
{
    return nullptr;
}

Response *CAP_response_CLEAR(Message cmd)
{
    return nullptr;
}

Response *CAP_response_END(Message cmd)
{
    return nullptr;
}

/* CapSubCommand:
 *  Contains information about a CAP subcommand
 */
struct CapSubCommand
{
    std::string name;
    Response *(*response)(Message cmd);
};

Response *response_CAP(Message cmd)
{
    Response *response = nullptr;

    static std::vector<struct CapSubCommand> const subcommands =\
    {
        { "LS",     CAP_response_LS,    },
        { "LIST",   CAP_response_LIST,  },
        { "REQ",    CAP_response_REQ,   },
        { "ACK",    CAP_response_ACK,   },
        { "NAK",    CAP_response_NAK,   },
        { "CLEAR",  CAP_response_CLEAR, },
        { "END",    CAP_response_END,   },
    };

    for (struct CapSubCommand subcmd : subcommands)
    {
        if (cmd.params[1] == subcmd.name)
        {
            response = subcmd.response(cmd);
            break;
        }
    }

    return response;
}

#pragma GCC diagnostic pop

