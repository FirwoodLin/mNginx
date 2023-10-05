cmake_minimum_required(VERSION 3.26)
project(mNginx C)

set(CMAKE_C_STANDARD 11)

add_executable(mNginx
        src/main.c
        src/header_edit.h
        src/header_edit.c
        src/data_trans.h
        src/data_trans.c
        src/http_response.c
        src/http_response.h
        src/config.c
        src/config.h
        src/proxy.c
        src/proxy.h
        src/util.c
        src/util.h
)
