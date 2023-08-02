/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef IRC_MESSAGE_HPP
#define IRC_MESSAGE_HPP

#include <optional>
#include <ostream>
#include <string>
#include <vector>


/**
 * Represents an IRC message. PREFIX contains the message prefix (if any),
 * COMMAND has the command, and PARAMS holds any parameters.
 */
class Message
{
public:
    /** Does not include leading ':'. */
    std::optional<std::string> prefix{};
    std::string command{};
    std::vector<std::string> params{};

    /**
     * Construct a Message from IRC data. MESSAGE can contain extra data after
     * the IRC message. If the parse was successful, MESSAGE will be changed to
     * only contain this extra data.
     */
    static Message parse(std::string &message);

    /**
     * Convert the message to IRC data. Includes the CRLF. Throws if the
     * message if longer than 512 characters.
     */
    std::string to_irc() const;

    /** Does not include CRLF. */
    operator std::string() const;

    Message()=default;
    Message(Message const &)=default;
    Message(
        std::string const &command,
        std::vector<std::string> const &params);
    Message(
        std::string const &prefix,
        std::string const &command,
        std::vector<std::string> const &params);
    Message(std::string message);
};

std::ostream &operator<<(std::ostream &os, Message const &msg);


#endif
