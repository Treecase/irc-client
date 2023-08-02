/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "Frontend.hpp"

#include <util/debug.hpp>
#include <LuaInteractions.hpp>
#include <LuaMessage.hpp>

#include <cctype>


/** Convert a string to lowercase. */
static std::string lowercase(std::string const &str)
{
    std::string out{};
    for (unsigned char ch : str)
        out.push_back(std::tolower(ch));
    return out;
}


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
    luaL_requiref(L, "Interaction", luaopen_interaction, 1);
    lua_pop(L, 2);

    lua_newtable(L);
    lua_setglobal(L, "IRC");

    lua_pushcfunction(L, debug_lua_print);
    lua_setglobal(L, "print");

    _guard(luaL_dofile(L, "scripts/responses.lua"));
}


std::shared_ptr<Interaction>
FrontendMessageHandler::execute(Frontend *f, Message const &msg)
{
    std::shared_ptr<Interaction> result{nullptr};
    int irc_T = lua_getglobal(L, "IRC");
    lua_assert(irc_T == LUA_TTABLE);
    auto const cmd = lowercase(msg.command);
    int fn_T = lua_getfield(L, -1, cmd.c_str());
    if (fn_T == LUA_TFUNCTION)
    {
        lua_pushmessage(L, msg);
        _guard(lua_pcall(L, 1, 1, 0));
        if (!lua_isnil(L, -1))
            result = luaL_checkinteraction(L, -1);
    }
    else
    {
        lua_pop(L, 1);
        result = std::make_shared<PrintInteraction>(msg);
    }
    lua_pop(L, 1);
    return result;
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
