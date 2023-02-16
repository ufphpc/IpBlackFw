cmake_minimum_required(VERSION 2.8)
project(test)

add_compile_options(-std=c++11 -Wall -g)

aux_source_directory(. DIR_CLIENT_SRCS)

add_executable(
    test
    ${DIR_CLIENT_SRCS}
)
