set(CURSES_NEED_NCURSES TRUE)
find_package(Curses 6.4 REQUIRED)
find_package(Lua 5.4 REQUIRED)

add_library(frontend-ncurses STATIC
    FrontendNCurses.cpp
    MessageHandler.cpp
)
target_link_libraries(frontend-ncurses
    PUBLIC
        backend
        irc
        util
        ${CURSES_LIBRARIES}
        ${LUA_LIBRARIES}
    PRIVATE
        handler-lua
)
target_include_directories(frontend-ncurses
    PUBLIC
        .
        ${CURSES_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIR}
)
target_compile_definitions(frontend-ncurses PUBLIC ${CURSES_CFLAGS})