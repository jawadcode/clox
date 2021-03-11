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

static int constantInstruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t constant = chunk->code[offset + 1];
	printf("%-16s %4d '", name, constant);
	printValue(chunk->constants.values[constant]);
	printf("'\n");
	return offset + 2;
}

// Display simple instruction without any arguments
static int simpleInstruction(const char *name, int offset)
{
	printf("%s\n", name);
	return offset + 1;
}

int disassembleInstruction(Chunk *chunk, int offset)
{
	// Print offset in 4-digit format with left aligned zero-padding
	printf("%04d ", offset);

	// If line number is the same as previous one then display "   | "
	if (offset > 0 &&
			chunk->lines[offset] == chunk->lines[offset - 1])
	{
		printf("   | ");
	}
	else
	{
		// If line number is different then print it as a 4 digit number with left aligned zero-padding
		printf("%4d ", chunk->lines[offset]);
	}

	// Get instruction code at offset
	uint8_t instruction = chunk->code[offset];

	// Handle all different types of instruction
	switch (instruction)
	{
	case OP_CONSTANT:
	{
		return constantInstruction("OP_CONSTANT", chunk, offset);
	}
	case OP_RETURN:
	{
		return simpleInstruction("OP_RETURN", offset);
	}
	// Handle unknown instruction gracefully
	default:
	{
		printf("Unknown opcode %d\n", instruction);
		return offset + 1;
	}
	}
}