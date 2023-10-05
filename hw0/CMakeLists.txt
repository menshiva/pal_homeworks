cmake_minimum_required(VERSION 3.26.4)
project(hw0 C)

set(CMAKE_C_STANDARD 17)

add_executable(hw0 main.c)
target_link_libraries(hw0 m)
