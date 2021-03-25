#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
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

/* Checks if value is falsey, the two cases are:
 * - if the value is false
 * - if the value is nil
 */
static bool isFalsey(Value value)
{
	return IS_NIL(value) || (IS_BOOL(value) & !AS_BOOL(value));
}

// Pop last two strings off of stack, concatenate and then push the result
static void concatenate()
{
	// Last in, first out, so the first string will be the second one from the top of the stack
	ObjString *b = AS_STRING(pop());
	ObjString *a = AS_STRING(pop());

	// Resulting length of concatenated string
	int length = a->length + b->length;
	char *chars = ALLOCATE(char, length + 1);
	// Copy "a" to "chars"
	memcpy(chars, a->chars, a->length);
	// Copy "b" to "chars" but with offset of the length of "a"
	memcpy(chars + a->length, b->chars, b->length);
	// Null terminator 🙄
	chars[length] = '\0';

	// Create string object without copying and just taking ownership instead
	ObjString *result = takeString(chars, length);
	push(OBJ_VAL(result));
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

// Perform binary operation on top two items in the stack
// Do number typecheck
// "do {} while (false)" makes it so that all of the statements end up in the same scope
#define BINARY_OP(valueType, op)                       \
	do                                                   \
	{                                                    \
		if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)))    \
		{                                                  \
			runtimeError("Binary operands must be numbers"); \
			return INTERPRET_RUNTIME_ERROR;                  \
		}                                                  \
		double b = AS_NUMBER(pop());                       \
		double a = AS_NUMBER(pop());                       \
		push(valueType(a op b));                           \
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
		// Keyword constants
		case OP_NIL:
			push(NIL_VAL);
			break;
		case OP_TRUE:
			push(BOOL_VAL(true));
			break;
		case OP_FALSE:
			push(BOOL_VAL(false));
			break;
		case OP_EQUAL:
		{
			Value b = pop();
			Value a = pop();
			push(BOOL_VAL(valuesEqual(a, b)));
			break;
		}
		// Binary Operations
		case OP_GREATER:
			BINARY_OP(BOOL_VAL, >);
			break;
		case OP_LESS:
			BINARY_OP(BOOL_VAL, <);
			break;
		case OP_ADD:
			if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
				concatenate();
			else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
			{
				double b = AS_NUMBER(pop());
				double a = AS_NUMBER(pop());
				push(NUMBER_VAL(a + b));
			}
			else
			{
				runtimeError("Operands must be two numbers or two strings");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
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
		// Unary operations
		case OP_NOT:
			push(BOOL_VAL(isFalsey(pop())));
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
		// Special
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