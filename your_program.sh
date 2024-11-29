#!/bin/sh


set -e 
(
  cd "$(dirname "$0")"
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
  cmake --build ./build
)

exec $(dirname "$0")/build/git "$@"
