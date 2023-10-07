#!/bin/bash

# 检测 build 文件夹是否存在
if [ -d "build" ]; then
    echo "清空 build 文件夹..."
    rm -rf build/*
else
    echo "创建 build 文件夹..."
    mkdir build
fi

# 进入 build 文件夹并执行 cmake 和 make
cd build || exit
cmake ..
make
cd ..
./build/mNginx