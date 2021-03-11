#include <stdio.h>

#include "debug.h"

void disassembleChunk(Chunk *chunk, const char *name)
{
	printf("=== %s ===\n", name);

	for (int offset = 0; offset < chunk->count;)
	{
		offset = disassembleInstruction(chunk, offset);
	}
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

	// Get instruction code at offset
	uint8_t instruction = chunk->code[offset];

	// Handle all different types of instructions
	switch (instruction)
	{
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