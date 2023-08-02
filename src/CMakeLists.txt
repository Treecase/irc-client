add_executable(ircc args.cpp main.cpp MainLoop.cpp)
target_compile_features(ircc PRIVATE cxx_std_17)
target_include_directories(ircc PRIVATE "${PROJECT_BINARY_DIR}/include")
target_link_libraries(ircc PRIVATE irc util "${FRONTEND_LIBRARY}")

configure_file(config.hpp.in "${PROJECT_BINARY_DIR}/include/config.hpp")

add_subdirectory(frontends)
add_subdirectory(irc)
add_subdirectory(util)