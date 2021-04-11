#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/* The Call Frame for a function invocation:
	 - "function" is a pointer to the function being invoked
	 - "ip" is a relative instruction pointer for this invocation
	 - "slots" points into the virtual machine's value stack at the first slot the invocation can use
 */
typedef struct
{
	ObjFunction *function;
	uint8_t *ip;
	Value *slots;
} CallFrame;

/* State for the Virtual Machine:
	 - "frames" is a stack that holds all of the current call frames
	 - "frameCount" is the current height of "frames"
	 - "stack" is the Virtual Machine's stack
	 - "stackTop" is a pointer pointing just past the last element in "stack"
	 - "strings" is a table of all of the interned strings
	 - "objects" is a linked list of references to objects
 */
typedef struct
{
	CallFrame frames[FRAMES_MAX];
	int frameCount;

	Value stack[STACK_MAX];
	Value *stackTop;
	Table globals;
	Table strings;

	Obj *objects;
} VM;

typedef enum
{
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

// Initialise Virtual Machine and zero-out all fields
void initVM();
// Free all manually allocated fields in the Virtual Machine
void freeVM();
// Interpret source code and return result
InterpretResult interpret(const char *source);
// Push "value" on the top of "vm.stack" and increment "vm.stackTop" pointer
void push(Value value);
// Decrements the "vm.stackTop" pointer and returns the value of what was at the top before it is overwritten
Value pop();

#endif