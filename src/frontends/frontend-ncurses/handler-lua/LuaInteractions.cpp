/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#include "LuaInteractions.hpp"
#include "LuaMessage.hpp"

#include <cstring> // memset


static int fn_respond(lua_State *L);


static const luaL_Reg interactionlib_f[] = {
    {"respond", fn_respond},
    {nullptr, nullptr}
};


int luaopen_interaction(lua_State *L)
{
    luaL_newmetatable(L, "IRCC.Interaction");
    lua_pop(L, 1);
    luaL_newlib(L, interactionlib_f);
    return 1;
}


std::shared_ptr<Interaction> luaL_checkinteraction(lua_State *L, int arg)
{
    auto const i = static_cast<std::shared_ptr<Interaction> *>(
        luaL_checkudata(L, arg, "IRCC.Interaction"));
    luaL_argcheck(L, i != nullptr, arg, "`Interaction' expected");
    return *i;
}



int fn_respond(lua_State *L)
{
    auto const msg = luaL_checkmessage(L, 1);

    auto ptr = static_cast<std::shared_ptr<Interaction> *>(
        lua_newuserdatauv(L, sizeof(std::shared_ptr<Interaction>), 0));
    std::memset(ptr, 0, sizeof(std::shared_ptr<Interaction>));
    *ptr = std::make_shared<RespondInteraction>(*msg);

    luaL_setmetatable(L, "IRCC.Interaction");
    return 1;
}
