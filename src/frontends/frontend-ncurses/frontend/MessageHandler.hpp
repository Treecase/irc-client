/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTENDNCURSES_MESSAGEHANDLER_HPP
#define FRONTENDNCURSES_MESSAGEHANDLER_HPP

#include <Interactions.hpp>
#include <irc/Message.hpp>

#include <lua.hpp>

#include <memory>


class Frontend;

/**
 * Handles IRC messages for the Frontend.
 *
 * Handling is done through Lua scripts. The handler calls a Lua function
 * corresponding to the command, which then returns a response message (or nil
 * to do nothing).
 *
 * ### In Lua:
 *  Handler function format: `fn(Message): Message|nil`
 *
 *  A global table named `IRC` is where handlers should be installed. Handler
 *  functions must be named after the command they handle, in lowercase.
 *  Example 'PING' handler:
 *
 * ```Lua
 * function IRC.ping(msg)
 *   return Message.new("PONG :"..msg.params(1))
 * end
 * ```
 */
class FrontendMessageHandler
{
    struct LuaStateDeleter
    {
        void operator()(lua_State *L) const;
    };
    std::unique_ptr<lua_State, LuaStateDeleter> const _L_actual;
    lua_State *const L; // alias for _L_actual

    /** Catches a Lua error and re-throws it as a C++ exception. */
    void _guard(int status) const;

public:
    FrontendMessageHandler();
    /** Execute the handler for `msg`. */
    std::shared_ptr<Interaction> execute(Frontend *f, Message const &msg);
};


#endif
