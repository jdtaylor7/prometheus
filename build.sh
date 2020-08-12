#!/bin/sh

# Exit if any command fails.
set -e

while getopts "rt:h" flag
do
    case $flag in
    r) r_flag=1 ;;
    t) t_flag=1
       t_val="$OPTARG" ;;
    h | ?) echo "Usage: $0: [-r][-t target]"
           echo "   -r: Rebuild build directory"
           echo "   -t: Specify build target"
           exit 2;;
    esac
done

if [ ! -z "$r_flag" ]; then
    echo "Rebuilding build directory"
    rm -rf ./build
fi
mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release -Wno-dev ..

if [ ! -z "$t_flag" ]; then
    echo "Building target $t_val"
    cmake --build . --target "$t_val"
else
    echo "Building all targets"
    cmake --build .
fi
