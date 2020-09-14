cmake_minimum_required(VERSION 3.2)

project(Blaze-HTTP LANGUAGES C)

add_executable(server.out
    src/main.c
    src/C-Thread-Pool/thpool.c
)

target_link_libraries(server.out pthread)