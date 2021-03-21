#include <stdarg.h>
#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

VM vm;

// Set top of stack as beginning of stack, therefore marking the rest as unused
static void resetStack()
{
	vm.stackTop = vm.stack;
}

// Print formatted error with line number to STDERR
static void runtimeError(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	size_t instruction = vm.ip - vm.chunk->code - 1;
	int line = vm.chunk->lines[instruction];
	fprintf(stderr, "[line %d] in script\n", line);

	resetStack();
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

// Peek value that is "distance" items below stackTop
static Value peek(int distance)
{
	return vm.stackTop[-1 - distance];
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

// Perform binary operation on top two items in the stack
// Do number typecheck
// "do {} while (false)" makes it so that all of the statements end up in the same scope
#define BINARY_OP(valueType, op)                    \
	do                                                \
	{                                                 \
		if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
		{                                               \
			runtimeError("Operands must be numbers.");    \
			return INTERPRET_RUNTIME_ERROR;               \
		}                                               \
		double b = AS_NUMBER(pop());                    \
		double a = AS_NUMBER(pop());                    \
		push(valueType(a op b));                        \
	} while (false)

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
			BINARY_OP(NUMBER_VAL, +);
			break;
		case OP_SUBTRACT:
			BINARY_OP(NUMBER_VAL, -);
			break;
		case OP_MULTIPLY:
			BINARY_OP(NUMBER_VAL, *);
			break;
		case OP_DIVIDE:
			BINARY_OP(NUMBER_VAL, /);
			break;
		case OP_NEGATE:
			if (!IS_NUMBER(peek(0)))
			{
				runtimeError("Operand must be a number.");
				return INTERPRET_RUNTIME_ERROR;
			}

			// Pop last value from stack, negate it, and then push it back on
			push(NUMBER_VAL(-AS_NUMBER(pop())));
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