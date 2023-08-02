add_library(handler-lua STATIC
    LuaMessage.cpp
    LuaInteractions.cpp
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