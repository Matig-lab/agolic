#!/usr/bin/env sh

set -xe

gcc -o build/agolic_optimized gameoflife.c -Wextra -Werror -pedantic -lm -lSDL2
