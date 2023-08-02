add_library(irc STATIC
    IRCClient.cpp
    Message.cpp
)
target_include_directories(irc PUBLIC .)
target_link_libraries(irc PUBLIC util)