/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef FRONTENDNCURSES_LUAINTERACTIONS_HPP
#define FRONTENDNCURSES_LUAINTERACTIONS_HPP

#include <Interactions.hpp>

#include <lua.hpp>

#include <memory>


/** Open the Interaction library. */
int luaopen_interaction(lua_State *L);
/** Checks whether stack item ARG is an Interaction and returns it. */
std::shared_ptr<Interaction> luaL_checkinteraction(lua_State *L, int arg);


#endif
