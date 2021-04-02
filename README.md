# CLox Bytecode Interpreter

This is a bytecode interpreter written in C for the Lox programming language following [this amazing book](https://craftinginterpreters.com)

## Prerequisites:
- GCC (Clang will probably work too)
- C STD Lib Header files (included in all Linux distros)

## How to run:
- `gcc *.c -o clox` to compile the source into an executable called `clox`
- `./clox` to launch the REPL
- or `./clox file.lox` to run `file.lox` (in the current directory)