add_library(util STATIC
    debug.cpp
    sockets.cpp
    strings.cpp
)
target_include_directories(util PUBLIC .)