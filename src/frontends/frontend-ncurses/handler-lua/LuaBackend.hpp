/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTENDNCURSES_LUABACKEND_HPP
#define FRONTENDNCURSES_LUABACKEND_HPP

#include <Backend.hpp>

#include <lua.hpp>


/** Open the Backend library. */
int luaopen_backend(lua_State *L);
/** Push a Backend onto the stack. */
void lua_pushbackend(lua_State *L, Backend &b);
/** Checks whether stack item ARG is a backend and returns it. */
Backend *luaL_checkbackend(lua_State *L, int arg);


#endif
