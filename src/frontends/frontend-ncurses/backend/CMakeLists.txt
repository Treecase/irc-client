add_library(backend STATIC Backend.cpp Channel.cpp)
target_link_libraries(backend PUBLIC irc util)
target_include_directories(backend PUBLIC .)