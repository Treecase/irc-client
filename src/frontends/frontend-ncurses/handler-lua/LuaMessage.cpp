/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "LuaMessage.hpp"

#include <util/debug.hpp>

#include <cstring> // memset


/**
 * Message.new(string) -> IRC.Message
 *
 * Construct a new IRC.Message from a string.
 * Raises an error if the string is not a valid IRC message.
 */
static int message_new(lua_State *L);

static int message_dunder_tostring(lua_State *L);
/**
 * Message:prefix() -> string|nil
 *
 * Get the Message's prefix.
 */
static int message__prefix(lua_State *L);
/**
 * Message:command() -> string
 *
 * Get the Message's command.
 */
static int message__command(lua_State *L);
/**
 * 1. Message:params() -> array[string]
 * 2. Message:params(int) -> string
 *
 * 1. Get the Message's parameter list.
 * 2. Get the Message's Nth parameter.
 */
static int message__params(lua_State *L);


static const luaL_Reg messagelib_f[] = {
    {"new", message_new},
    {nullptr, nullptr}
};

static const luaL_Reg messagelib_m[] = {
    {"__tostring", message_dunder_tostring},
    {"prefix", message__prefix},
    {"command", message__command},
    {"params", message__params},
    {nullptr, nullptr}
};


int luaopen_message(lua_State *L)
{
    luaL_newmetatable(L, "IRC.Message");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, messagelib_m, 0);
    lua_pop(L, 1);

    luaL_newlib(L, messagelib_f);
    return 1;
}


void lua_pushmessage(lua_State *L, Message const &msg)
{
    Message *ptr = static_cast<Message *>(
        lua_newuserdatauv(L, sizeof(Message), 0));
    std::memset(ptr, 0, sizeof(Message));
    *ptr = msg;
    luaL_setmetatable(L, "IRC.Message");
}


Message *luaL_checkmessage(lua_State *L, int arg)
{
    auto msg = static_cast<Message *>(luaL_checkudata(L, arg, "IRC.Message"));
    luaL_argcheck(L, msg != nullptr, arg, "`Message' expected");
    return msg;
}



static int message_new(lua_State *L)
{
    std::string const str = luaL_checkstring(L, 1);
    Message msg;
    try {
        msg = Message{str};
    } catch (std::runtime_error const &e) {
        luaL_error(L, "Failed to construct Message: %s", e.what());
    }
    lua_pushmessage(L, msg);
    return 1;
}


static int message_dunder_tostring(lua_State *L)
{
    auto const msg = luaL_checkmessage(L, 1);
    std::string str{*msg};
    lua_pushstring(L, str.c_str());
    return 1;
}


static int message__prefix(lua_State *L)
{
    auto const msg = luaL_checkmessage(L, 1);
    if (msg->prefix.has_value())
        lua_pushstring(L, msg->prefix.value().c_str());
    else
        lua_pushnil(L);
    return 1;
}


static int message__command(lua_State *L)
{
    auto const msg = luaL_checkmessage(L, 1);
    lua_pushstring(L, msg->command.c_str());
    return 1;
}


static int message__params(lua_State *L)
{
    auto const msg = luaL_checkmessage(L, 1);
    if (lua_gettop(L) == 2)
    {
        auto const i = luaL_checkinteger(L, 2);
        if (i > msg->params.size())
            return luaL_error(L, "out of range (%I/%d)", i, msg->params.size());
        lua_pushstring(L, msg->params.at(i-1).c_str());
    }
    else
    {
        lua_newtable(L);
        for (size_t i = 0; i < msg->params.size(); ++i)
        {
            lua_pushstring(L, msg->params.at(i).c_str());
            lua_seti(L, -2, i+1);
        }
    }
    return 1;
}
