cmake_minimum_required(VERSION 3.29)
project(clist2html)

set(CMAKE_CXX_STANDARD 20)

add_executable(clist2html main.cpp
        utils.cpp
        utils.h)
