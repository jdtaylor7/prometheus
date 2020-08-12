# # !/bin/sh
#
# # Exit if any command fails.
# set -e
#
# rm -rf ./build
# mkdir build && cd build
#
# cmake -DCMAKE_BUILD_TYPE=Release -Wno-dev ..
# cmake --build .

#!/bin/sh

# Exit if any command fails.
set -e

cd build

cmake -DCMAKE_BUILD_TYPE=Release -Wno-dev ..
cmake --build . --target main
