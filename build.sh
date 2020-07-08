#!/bin/sh

# Exit if any command fails.
set -e

rm -rf ./build
mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
