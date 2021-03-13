#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

// Enum of all possible opcodes
typedef enum
{
	OP_CONSTANT,
	OP_NEGATE,
	OP_RETURN,
} OpCode;

/* Chunk of bytecode:
   - "count" holds the index of the next code to be inserted
   - "capacity" is the allocated size of "code" (in bytes)
   - "code" is the actual array of opcodes
	 - "constants" contains all of the constant values
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