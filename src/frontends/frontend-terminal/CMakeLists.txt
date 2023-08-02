add_library(frontend-terminal STATIC FrontendTerminal.cpp)
target_link_libraries(frontend-terminal PUBLIC irc util)
target_include_directories(frontend-terminal PUBLIC .)