#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk *chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
}

void writeChunk(Chunk *chunk, uint8_t byte)
{
	// If capacity will be exceeded after next write then grow "chunk->code"
	if (chunk->capacity < chunk->count + 1)
	{
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
	}
	// If capacity will still be greater, then, do nothing
	// Add byte to the chunk's codes and increment the counter
	chunk->code[chunk->count] = byte;
	chunk->count++;
}

void freeChunk(Chunk *chunk)
{
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	// Re-initialise to leave chunk in well-defined state
	initChunk(chunk);
}