/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTENDNCURSES_LUAMESSAGE_HPP
#define FRONTENDNCURSES_LUAMESSAGE_HPP

#include <irc/Message.hpp>

#include <lua.hpp>


/** Open the IRC.Message library. */
int luaopen_message(lua_State *L);
/** Push an IRC.Message onto the stack. */
void lua_pushmessage(lua_State *L, Message const &msg);
/** Checks whether stack item ARG is an IRC.Message and returns it. */
Message *luaL_checkmessage(lua_State *L, int arg);


#endif
