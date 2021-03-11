#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

// Disassemble "chunk" and display in human readable format labelled with "name"
void disassembleChunk(Chunk *chunk, const char* name);
// Disassemble instruction in "chunk->code" at offset and display in human readable format
int disassembleInstruction(Chunk* chunk, int offset);

#endif