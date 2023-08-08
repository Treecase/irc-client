/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "LuaBackend.hpp"
#include "LuaChannel.hpp"
#include "LuaMessage.hpp"


/**
 * 1. Backend:active_channel() -> Channel
 * 2. Backend:active_channel(channel: String)
 *
 * 1. Get the active channel.
 * 2. Set the active channel.
 */
static int backend__active_channel(lua_State *L);

/**
 * Backend:add_channel(name: String) -> Channel
 *
 * Add a channel to the backend.
 */
static int backend__add_channel(lua_State *L);

/**
 * 1. Backend:prefix() -> array[Channel]
 * 2. Backend:prefix(channel_name: String) -> Channel
 *
 * 1. Get the list of active channels.
 * 2. Get a specific channel. Returns nil if the channel does not exist.
 */
static int backend__channels(lua_State *L);

/**
 * Backend:respond(msg: Message)
 *
 * Send a message over IRC.
 */
static int backend__respond(lua_State *L);


static const luaL_Reg backendlib_m[] = {
    {"active_channel", backend__active_channel},
    {"add_channel", backend__add_channel},
    {"channels", backend__channels},
    {"respond", backend__respond},
    {nullptr, nullptr}
};


int luaopen_backend(lua_State *L)
{
    luaL_newmetatable(L, "IRCC.Backend");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, backendlib_m, 0);
    return 0;
}


void lua_pushbackend(lua_State *L, Backend &b)
{
    Backend **ptr = static_cast<Backend **>(
        lua_newuserdatauv(L, sizeof(Backend *), 0));
    *ptr = &b;
    luaL_setmetatable(L, "IRCC.Backend");
}


Backend *luaL_checkbackend(lua_State *L, int arg)
{
    auto b = static_cast<Backend **>(luaL_checkudata(L, arg, "IRCC.Backend"));
    luaL_argcheck(L, b != nullptr, arg, "`Backend' expected");
    return *b;
}



static int backend__channels1(lua_State *L);
static int backend__channels2(lua_State *L);

static int backend__channels(lua_State *L)
{
    auto const top = lua_gettop(L);
    if (top == 1)
        return backend__channels1(L);
    else if (top == 2)
        return backend__channels2(L);
    else
        return luaL_error(L, "bad arg count");
}

static int backend__channels1(lua_State *L)
{
    auto const b = luaL_checkbackend(L, 1);

    lua_newtable(L);
    size_t i = 1;
    for (auto &kv : b->get_channels())
    {
        lua_pushchannel(L, kv.second);
        lua_seti(L, -2, i++);
    }
    return 1;
}

static int backend__channels2(lua_State *L)
{
    auto const b = luaL_checkbackend(L, 1);
    std::string const name{luaL_checkstring(L, 2)};

    auto &channels = b->get_channels();
    try {
        lua_pushchannel(L, channels.at(name));
    }
    catch (std::out_of_range const &e) {
        lua_pushnil(L);
    }
    return 1;
}


static int backend__active_channel(lua_State *L)
{
    auto const b = luaL_checkbackend(L, 1);
    switch (lua_gettop(L))
    {
    case 1:
        lua_pushchannel(L, b->get_active_channel());
        return 1;
    case 2:
        b->set_active_channel(luaL_checkstring(L, 2));
        return 0;
    }
    return luaL_error(L, "too many args");
}


static int backend__add_channel(lua_State *L)
{
    auto const b = luaL_checkbackend(L, 1);
    auto const s = luaL_checkstring(L, 2);
    auto kv = b->get_channels().emplace(s, s);
    auto &ch = kv.first->second;
    lua_pushchannel(L, ch);
    return 1;
}


static int backend__respond(lua_State *L)
{
    auto const b = luaL_checkbackend(L, 1);
    auto const msg = luaL_checkmessage(L, 2);
    b->send_response(*msg);
    return 0;
}
