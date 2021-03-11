#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk *chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	initValueArray(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte)
{
	// If the capacity will be exceeded after next write then grow "chunk->code"
	if (chunk->capacity < chunk->count + 1)
	{
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
	}
	// If the capacity will still be greater, do nothing
	// Add byte to the chunk's codes and increment the counter
	chunk->code[chunk->count] = byte;
	chunk->count++;
}

int addConstant(Chunk *chunk, Value value)
{
	writeValueArray(&chunk->constants, value);
	return chunk->constants.count - 1;
}

void freeChunk(Chunk *chunk)
{
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	freeValueArray(&chunk->constants);
	// Reinit to leave chunk in well-defined state
	initChunk(chunk);
}