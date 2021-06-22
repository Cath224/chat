cmake_minimum_required(VERSION 3.19)
project(PingPongGame C)

set(CMAKE_C_STANDARD 11)

add_executable(PingPongGame main.c)

if(MINGW)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(PingPongGame PRIVATE wsock32 ws2_32 Threads::Threads)
endif()