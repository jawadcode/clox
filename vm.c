#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

VM vm;

static void resetStack()
{
	vm.stackTop = vm.stack;
}

void initVM()
{
	resetStack();
}

void freeVM()
{
}

void push(Value value)
{
	*vm.stackTop = value;
	vm.stackTop++;
}

Value pop()
{
	vm.stackTop--;
	return *vm.stackTop;
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

// Perform binary operation on top two items in the stack
// do {} while (false) makes it so that all of the statements end up in the same scope
#define BINARY_OP(op) \
	do                  \
	{                   \
		double b = pop(); \
		double a = pop(); \
		push(a op b);     \
	} while (false);

	for (;;)
	{
#ifdef DEBUG_TRACE_EXECUTION
		printf("          ");
		// Print all values in stack
		for (Value *slot = vm.stack; slot < vm.stackTop; slot++)
		{
			printf("[ ");
			printValue(*slot);
			printf(" ]");
		}
		printf("\n");
		// Disassemble and display each instruction before execution
		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE())
		{
		case OP_CONSTANT:
		{
			Value constant = READ_CONSTANT();
			push(constant);
			break;
		}
		case OP_ADD:
			BINARY_OP(+);
			break;
		case OP_SUBTRACT:
			BINARY_OP(-);
			break;
		case OP_MULTIPLY:
			BINARY_OP(*);
			break;
		case OP_DIVIDE:
			BINARY_OP(/);
			break;
		case OP_NEGATE:
			// Pop last value from stack, negate it, and then push it back on
			push(-pop());
			break;
		case OP_RETURN:
			// Print value of last item in stack before exiting
			printValue(pop());
			printf("\n");
			return INTERPRET_OK;
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(const char *source)
{
	// Create & Init chunk
	Chunk chunk;
	initChunk(&chunk);

	// Compile source and write bytecode to "chunk" (if it fails then free the chunk and return a compiler error)
	if (!compile(source, &chunk))
	{
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	// Set up VM
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	// Run bytecode
	InterpretResult result = run();

	// Free chunk and return result of run
	freeChunk(&chunk);
	return result;
}