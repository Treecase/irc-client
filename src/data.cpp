/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *  IRC data structures implementation
 */

#include "data.h"
#include "logging.h"

#include <cctype>

#include <string>



/* converts message to a string (including the CRLF message ending) */
Message::operator std::string() const
{
    std::string str = "";

    /* add the prefix to the string */
    if (prefix != nullptr)
    {
        str += ":" + (*prefix);
    }

    /* add the command to the string */
    str += command;

    /* add the params to the string */
    for (std::string param : params)
    {
        str += " " + param;
    }

    if (str.size() > 512)
    {
        warn(
            "messages cannot exceed 512 characters!"
            "(message is %lu chars long)",
            str.size());
    }
    return str;
}

Message::Message(
        std::string *prefix_,
        std::string command_,
        std::vector<std::string> params_)
:   prefix(prefix_),
    command(command_),
    params(params_)
{
}

/* convert a string into a message */
Message::Message(std::string &str)
{
    size_t str_idx = 0;

    /* prefix */
    if (str[0] == ':')
    {
        str_idx += 1;

        size_t prefix_length = str.find(' ');
        if (prefix_length == std::string::npos)
        {
            error("prefix with no command!");
            prefix_length = str.size();
        }
        prefix_length -= str_idx;

        prefix = new std::string(str.substr(str_idx, prefix_length));
        str_idx += prefix_length + 1;
    }
    else
    {
        prefix = nullptr;
    }

    /* command */
    size_t command_len = str.find(' ', str_idx);
    if (command_len == std::string::npos)
    {
        command_len = str.size();
    }
    command_len -= str_idx;

    command = str.substr(str_idx, command_len);
    str_idx += command_len + 1;

    /* parameters */
    while (str_idx < str.size())
    {
        size_t param_len = str.find(' ', str_idx);
        if (param_len == std::string::npos)
        {
            param_len = str.size();
        }
        param_len -= str_idx;
        std::string param = str.substr(str_idx, param_len);

        /* a parameter beginning with a ':' is a trailing
         * parameter; it will always be the final parameter,
         * and may contain spaces */
        if (param[0] == ':')
        {
            /* skip the ':' */
            str_idx++;
            param = str.substr(str_idx);
            str_idx = str.size();
        }
        /* \r\n, aka CRLF, marks the end of an IRC message */
        else if (param[0] == '\r' && param.size() >= 2)
        {
            if (param[1] == '\n')
            {
                break;
            }
        }

        str_idx += param_len + 1;
        params.push_back(param);
    }
}

