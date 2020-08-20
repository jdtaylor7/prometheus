#!/bin/sh

# Exit if any command fails.
set -e

while getopts "re:th" flag
do
    case $flag in
    r) r_flag=1 ;;
    e) e_flag=1
       e_val="$OPTARG" ;;
    t) t_flag=1 ;;
    h | ?) echo "Usage: $0: [-r][-t target]"
           echo "   -r: Rebuild build directory"
           echo "   -e: Specify build executable"
           echo "   -t: Build in test mode"
           exit 2;;
    esac
done

if [ ! -z "$r_flag" ]; then
    echo "Rebuilding build directory"
    rm -rf ./build
fi
mkdir -p build
cd build

# Add make command line variables.
compile_flags="-DCMAKE_BUILD_TYPE=Release -Wno-dev"
if [ ! -z "$t_flag" ]; then
    echo "Building in test mode"
    compile_flags="$compile_flags -DTEST_MODE=ON"
fi

cmake $compile_flags ..

# Add cmake build command line variables.
build_flags=""
if [ ! -z "$e_flag" ]; then
    echo "Building target $e_val"
    build_flags="$build_flags --target $e_val"
else
    echo "Building all targets"
fi

cmake --build . $build_flags
