#ifndef clox_compiler_h
#define clox_compiler_h

#include "object.h"
#include "vm.h"

// Compile source code and then returns "ObjFunction" that represents the script
ObjFunction *compile(const char *source);

#endif
