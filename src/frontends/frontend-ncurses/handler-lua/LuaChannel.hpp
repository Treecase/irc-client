/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTENDNCURSES_LUACHANNEL_HPP
#define FRONTENDNCURSES_LUACHANNEL_HPP

#include <Channel.hpp>

#include <lua.hpp>


/** Open the Channel library. */
int luaopen_channel(lua_State *L);
/** Push a Channel onto the stack. */
void lua_pushchannel(lua_State *L, Channel &c);
/** Checks whether stack item ARG is a Channel and returns it. */
Channel *luaL_checkchannel(lua_State *L, int arg);


#endif
