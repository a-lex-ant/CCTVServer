cmake_minimum_required(VERSION 3.15)
project(serverProvaTCP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(serverProvaTCP main.cpp Utilita.cpp Utilita.h)