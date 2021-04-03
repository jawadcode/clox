#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk *chunk, const char *name)
{
	printf("=== %s ===\n", name);

	// Loop through every instruction in "chunk" and pretty-print each one
	for (int offset = 0; offset < chunk->count;)
	{
		offset = disassembleInstruction(chunk, offset);
	}
}

// Display operation of setting constant
static int constantInstruction(const char *name, Chunk *chunk, int offset)
{
	// Grab constant value from position "offset + 1" since OP_CONSTANT takes 2 bytes
	uint8_t constant = chunk->code[offset + 1];
	// Print operation name and constant value
	printf("%-16s %4d '", name, constant);
	printValue(chunk->constants.values[constant]);
	printf("'\n");
	// Skip ahead by an extra byte because of the size of the instruction
	return offset + 2;
}

// Display simple instruction without any arguments
static int simpleInstruction(const char *name, int offset)
{
	printf("%s\n", name);
	return offset + 1;
}

static int byteInstruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t slot = chunk->code[offset + 1];
	printf("%-16s %4d\n", name, slot);
	return offset + 2;
}

int disassembleInstruction(Chunk *chunk, int offset)
{
	// Print offset in 4-digit format with left aligned zero-padding
	printf("%04d ", offset);

	// If line number is the same as previous instruction then display "   | "
	if (offset > 0 &&
			chunk->lines[offset] == chunk->lines[offset - 1])
	{
		printf("   | ");
	}
	else
	{
		// If line number is different then print it
		printf("%4d ", chunk->lines[offset]);
	}

	// Get instruction code at offset
	uint8_t instruction = chunk->code[offset];

	// Handle all different types of instruction
	switch (instruction)
	{
	case OP_CONSTANT:
		return constantInstruction("OP_CONSTANT", chunk, offset);
	case OP_NIL:
		return simpleInstruction("OP_NIL", offset);
	case OP_TRUE:
		return simpleInstruction("OP_TRUE", offset);
	case OP_FALSE:
		return simpleInstruction("OP_FALSE", offset);
	case OP_POP:
		return simpleInstruction("OP_POP", offset);
	case OP_GET_LOCAL:
		return byteInstruction("OP_GET_LOCAL", chunk, offset);
	case OP_SET_LOCAL:
		return byteInstruction("OP_SET_LOCAL", chunk, offset);
	case OP_GET_GLOBAL:
		return constantInstruction("OP_GET_GLOBAL", chunk, offset);
	case OP_DEFINE_GLOBAL:
		return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
	case OP_SET_GLOBAL:
		return constantInstruction("OP_SET_GLOBAL", chunk, offset);
	case OP_EQUAL:
		return simpleInstruction("OP_EQUAL", offset);
	case OP_GREATER:
		return simpleInstruction("OP_GREATER", offset);
	case OP_LESS:
		return simpleInstruction("OP_LESS", offset);
	case OP_ADD:
		return simpleInstruction("OP_ADD", offset);
	case OP_SUBTRACT:
		return simpleInstruction("OP_SUBTRACT", offset);
	case OP_MULTIPLY:
		return simpleInstruction("OP_MULTIPLY", offset);
	case OP_DIVIDE:
		return simpleInstruction("OP_DIVIDE", offset);
	case OP_NOT:
		return simpleInstruction("OP_NOT", offset);
	case OP_NEGATE:
		return simpleInstruction("OP_NEGATE", offset);
	case OP_PRINT:
		return simpleInstruction("OP_PRINT", offset);
	case OP_RETURN:
		return simpleInstruction("OP_RETURN", offset);
	// Handle unknown instruction gracefully
	default:
	{
		printf("Unknown opcode %d\n", instruction);
		return offset + 1;
	}
	}
}
// Fix "21.4 Assignment" code, example lox code doesn't work :(