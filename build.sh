#!/usr/bin/env sh

set -xe

gcc -o agolic gameoflife.c -Wextra -Werror -pedantic -lm -lSDL2
