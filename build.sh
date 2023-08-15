#!/usr/bin/env sh

set -xe

gcc -o gol gameoflife.c -Wextra -Werror -pedantic -lm -lSDL2
