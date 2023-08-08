/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "Frontend.hpp"

#include <util/debug.hpp>
#include <util/strings.hpp>
#include <LuaBackend.hpp>
#include <LuaChannel.hpp>
#include <LuaMessage.hpp>

#include <cctype>


/** Replacement Lua `print` function. Outputs to `debugstream` instead. */
static int debug_lua_print(lua_State *L)
{
    int const n = lua_gettop(L);
    debugstream << "print: ";
    for (int i = 1; i <= n; ++i)
    {
        auto const str = luaL_tolstring(L, i, nullptr);
        lua_pop(L, 1);
        debugstream << str << ' ';
    }
    debugstream << std::endl;
    return 0;
}



FrontendMessageHandler::FrontendMessageHandler()
:   _L_actual{luaL_newstate()}
,   L{_L_actual.get()}
{
    if (!L)
        throw std::runtime_error{"luaL_newstate() failed"};
    luaL_checkversion(L);
    luaL_openlibs(L);

    luaL_requiref(L, "Message", luaopen_message, 1);
    luaL_requiref(L, "Backend", luaopen_backend, 1);
    luaL_requiref(L, "Channel", luaopen_channel, 1);
    lua_pop(L, 3);

    lua_newtable(L);
    lua_setglobal(L, "IRC");

    lua_pushcfunction(L, debug_lua_print);
    lua_setglobal(L, "print");

    _guard(luaL_dofile(L, "scripts/responses.lua"));
}


void FrontendMessageHandler::execute(Backend &b, Message const &msg)
{
    auto const cmd = lowercase(msg.command);
    auto const pre = lua_gettop(L);

    lua_getglobal(L, "IRC");
    lua_getfield(L, -1, cmd.c_str());

    lua_pushbackend(L, b);
    lua_pushmessage(L, msg);
    try {
        _guard(lua_pcall(L, 2, 0, 0));
    }
    catch (std::runtime_error const &e) {
        debugstream << "!!Error in '" << cmd << "' handler: " << e.what()
            << std::endl;
        b.get_active_channel().push_message(msg);
    }
    lua_pop(L, lua_gettop(L) - pre);
}



void FrontendMessageHandler::_guard(int status) const
{
    if (status != LUA_OK)
    {
        std::string const errmsg = luaL_tolstring(L, -1, nullptr);
        throw std::runtime_error{"Lua Error -- " + errmsg};
    }
}



/* ===[ LuaStateDeleter ]=== */
void FrontendMessageHandler::LuaStateDeleter::operator()(lua_State *L) const
{
    lua_close(L);
}
