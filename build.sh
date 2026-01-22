#!/bin/bash
set -xe

CFLAGS="-Wall -Wextra -g"
mkdir -p build

SFML_DIR=./sfml

# Compile the program
g++ main.cpp -o build/main $CFLAGS \
    -I"$SFML_DIR/include" -L"$SFML_DIR/lib" \
    -lsfml-graphics -lsfml-window -lsfml-system -lm
