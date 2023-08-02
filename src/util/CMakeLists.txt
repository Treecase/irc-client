add_library(util STATIC
    debug.cpp
    sockets.cpp
)
target_include_directories(util PUBLIC .)