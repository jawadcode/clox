#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

// Enum of all possible opcodes
typedef enum
{
	OP_RETURN,
	OP_CONSTANT,
} OpCode;

/* Chunk of bytecode with:
   - "count" holding the index of the next code to be inserted
   - "capacity" holding the allocated size of "code" (in bytes)
   - "code" holding the actual opcodes
	 - "constants" holding all of the constant values
 */
typedef struct
{
	int count;
	int capacity;
	uint8_t *code;
	int *lines;
	ValueArray constants;
} Chunk;

// Initialise "chunk" by zeroing-out values
void initChunk(Chunk *chunk);
// Free all memory used by "chunk->code" and then reinitialise "chunk"
void freeChunk(Chunk *chunk);
// Write "byte" to "chunk" with line number in "line"
void writeChunk(Chunk *chunk, uint8_t byte, int line);
// Add constant "value" to "chunk->constants" and return the index of it
int addConstant(Chunk *chunk, Value value);

#endif