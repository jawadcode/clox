#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"
#include "object.h"

// Compile source code and then write the bytecode to "chunk"
bool compile(const char *source, Chunk *chunk);

#endif