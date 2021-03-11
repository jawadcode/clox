#ifndef clox_value_h
#define clox_value_h

#include "common.h"

// Lox number type
typedef double Value;

/* Struct containing array of constant Values with:
	- "count" as the index of the next "Value" to be added
	- "capacity" as the allocated size of the array
	- "values" as the actual, dynamically sized array of "Value"s
 */
typedef struct
{
	int count;
	int capacity;
	Value* values;
} ValueArray;

// Initialise "array" by zeroing-out values
void initValueArray(ValueArray *array);
// Free all memory used by "array->values" and reinitialise "array"
void freeValueArray(ValueArray *array);
// Write "value" to "array->values"
void writeValueArray(ValueArray *array, Value value);

// Print constant value
void printValue(Value value);

#endif