add_library(handler-lua STATIC
    LuaBackend.cpp
    LuaChannel.cpp
    LuaMessage.cpp
)
target_link_libraries(handler-lua
    PUBLIC
        irc
        ${LUA_LIBRARIES}
    PRIVATE
        util
        backend
)
target_include_directories(handler-lua
    PUBLIC
        .
        ${LUA_INCLUDE_DIR}
)