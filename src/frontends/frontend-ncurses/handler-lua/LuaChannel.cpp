/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "LuaChannel.hpp"


/**
 * Channel:name() -> String
 *
 * Get the channel's name.
 */
static int channel__name(lua_State *L);

/**
 * Channel:write(msg: String)
 *
 * Write some text to the channel.
 */
static int channel__write(lua_State *L);

/**
 * Channel:add_user(user: String)
 *
 * Add a user to the channel.
 */
static int channel__add_user(lua_State *L);

/**
 * Channel:remove_user(user: String)
 *
 * Remove a user from the channel.
 */
static int channel__remove_user(lua_State *L);


static const luaL_Reg channellib_m[] = {
    {"write", channel__write},
    {"add_user", channel__add_user},
    {"remove_user", channel__remove_user},
    {nullptr, nullptr}
};


int luaopen_channel(lua_State *L)
{
    luaL_newmetatable(L, "IRCC.Channel");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, channellib_m, 0);
    lua_pop(L, 1);
    return 0;
}


void lua_pushchannel(lua_State *L, Channel &c)
{
    Channel **ptr = static_cast<Channel **>(
        lua_newuserdatauv(L, sizeof(Channel *), 0));
    *ptr = &c;
    luaL_setmetatable(L, "IRCC.Channel");
}


Channel *luaL_checkchannel(lua_State *L, int arg)
{
    auto c = static_cast<Channel **>(luaL_checkudata(L, arg, "IRCC.Channel"));
    luaL_argcheck(L, c != nullptr, arg, "`Channel' expected");
    return *c;
}



static int channel__name(lua_State *L)
{
    auto const c = luaL_checkchannel(L, 1);
    lua_pushstring(L, c->name.c_str());
    return 1;
}

static int channel__write(lua_State *L)
{
    auto const c = luaL_checkchannel(L, 1);
    auto const str = luaL_checkstring(L, 2);
    c->push_message(str);
    return 0;
}

static int channel__add_user(lua_State *L)
{
    auto const c = luaL_checkchannel(L, 1);
    auto const user = luaL_checkstring(L, 2);
    c->add_user(user);
    return 0;
}

static int channel__remove_user(lua_State *L)
{
    auto const c = luaL_checkchannel(L, 1);
    auto const user = luaL_checkstring(L, 2);
    c->remove_user(user);
    return 0;
}
