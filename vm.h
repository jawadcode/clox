#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

/* State for the Virtual Machine:
	 - "chunk" holds the bytecode to be executed
	 - "ip" is the instruction pointer and points to an address within "chunk"
	 - "stack" is the Virtual Machine's stack
	 - "stackTop" is a pointer pointing just past the last element in "stack"
 */
typedef struct
{
	Chunk *chunk;
	uint8_t *ip;
	Value stack[STACK_MAX];
	Value *stackTop;
} VM;

typedef enum
{
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpretResult;

// Initialise Virtual Machine and zero-out all fields
void initVM();
// Free all manually allocated fields in the Virtual Machine
void freeVM();
// Interpret "chunk" containing bytecode
InterpretResult interpret(Chunk *chunk);
// Push "value" on the top of "vm.stack" and increment "vm.stackTop" pointer
void push(Value value);
// Decrements the "vm.stackTop" pointer and returns the value of what was at the top before it is overwritten
Value pop();

#endif