cmake_minimum_required(VERSION 3.26)
project(mNginx C)

set(CMAKE_C_STANDARD 11)

add_executable(mNginx
        main.c
        header_edit.h
        header_edit.c
        data_trans.h
        data_trans.c)
