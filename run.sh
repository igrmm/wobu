#!/bin/sh

mkdir -p build
cd build
if [ "$1" = "debug" ]; then
    echo "Running debug mode."
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build .
    valgrind \
        --suppressions=../valgrind.sup \
        --leak-check=full \
        --show-reachable=yes \
        --show-leak-kinds=all \
        --error-limit=no \
        --gen-suppressions=all \
        --track-origins=yes \
        --keep-debuginfo=yes \
        --log-file=supdata.log \
        ./wobu
else
    echo "Running normal mode."
    cmake ..
    cmake --build .
    ./wobu
fi
