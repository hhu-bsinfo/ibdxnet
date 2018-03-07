#!/bin/bash

mkdir -p build
cd build

COMPILER_FLAGS=""

for flag in "$@"; do
    COMPILER_FLAGS="$COMPILER_FLAGS -D$flag"
done

if [ "$COMPILER_FLAGS" ]; then
    echo "Specified additional flags:$COMPILER_FLAGS"
fi

cmake -D CMAKE_CXX_FLAGS="$COMPILER_FLAGS" ..
make -j 8
