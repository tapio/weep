#!/bin/bash

ROOT="$(dirname "$(readlink -f "$0")")"/..

cppcheck --enable=all --quiet --verbose -D__cplusplus -j 8 "$ROOT/third-party/ecs" "$ROOT/engine" "$ROOT/game"


