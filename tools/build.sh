#! /usr/bin/env bash

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --preset Debug
cp ./build/Debug/compile_commands.json .
cmake --build --target pdm --preset Debug
