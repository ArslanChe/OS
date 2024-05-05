cmake_minimum_required(VERSION 3.28)
project(hw10)

set(CMAKE_CXX_STANDARD 17)

add_executable(hw10 server.c
        client1.c
        client2.c)
