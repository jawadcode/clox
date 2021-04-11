#ifndef clox_compiler_hprintprint
#define clox_compiler_h

#include "vm.h"
#include "object.h"

// Compile source code and then returns "ObjFunction" that represents the script
ObjFunction *compile(const char *source);

#endif