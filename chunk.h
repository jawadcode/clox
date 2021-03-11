#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"

// Enum of all possible opcodes
typedef enum
{
	OP_RETURN,
} OpCode;

/* Chunk of bytecode with:
   - "count" holding the index of the next code to be inserted
   - "capacity" holding the allocated size of "code" (in bytes)
   - "code" holding the actual opcodes
 */
typedef struct
{
	int count;
	int capacity;
	uint8_t *code;
} Chunk;

// Initialise "chunk"
void initChunk(Chunk *chunk);
// Free all memory used by "chunk->code" and then reinitialise "chunk"
void freeChunk(Chunk *chunk);
// Write "byte" to "chunk"
void writeChunk(Chunk *chunk, uint8_t byte);

#endif