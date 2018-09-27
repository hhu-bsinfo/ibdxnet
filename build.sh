#!/bin/bash

build_type="release"

if [ "$1" ]; then
    build_type="$1"
fi

FLAGS=""

case "$build_type" in
    debug)
        ;;
    release)
        FLAGS="IBNET_DISABLE_STATISTICS"
        ;;
    *)
        echo "Invalid build type \"$build_type\" specified"
        exit -1
esac

echo "Build type: $build_type"

mkdir -p build
cd build

for flag in "$FLAGS"; do
    COMPILER_FLAGS="$COMPILER_FLAGS -D$flag"
done

if [ "$COMPILER_FLAGS" ]; then
    echo "Specified additional flags:$COMPILER_FLAGS"
fi

cmake -D CMAKE_CXX_FLAGS="$COMPILER_FLAGS" ..
make -j 8
