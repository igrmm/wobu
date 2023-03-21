#!/bin/sh

mkdir -p build
cd build
if [ "$1" = "debug" ]; then
    echo "Running debug mode."
    cmake -DCMAKE_BUILD_TYPE=Debug ..
else
    echo "Running normal mode."
    cmake ..
fi
cmake --build .
./wobu
