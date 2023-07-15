#!/bin/sh

mkdir -p build
cd build
if [ "$1" = "debug" ]; then
    echo "Running debug mode."
    [ "$2" != "valgrind" ] && export SANITIZERS="-fsanitize=address,undefined"
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build .
    if [ "$2" = "valgrind" ]; then
        echo "Running VALGRIND, checkout build/supdata.log."
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
        ./wobu
    fi
else
    echo "Running normal mode."
    cmake ..
    cmake --build .
    ./wobu
fi
