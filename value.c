#include <stdio.h>

#include "memory.h"
#include "value.h"

void initValueArray(ValueArray *array)
{
	array->count = 0;
	array->capacity = 0;
	array->values = NULL;
}

void writeValueArray(ValueArray *array, Value value)
{
	// If the capacity will be exceeded after next write then grow "array->values"
	if (array->capacity < array->count + 1)
	{
		int oldCapacity = array->capacity;
		array->capacity = GROW_CAPACITY(oldCapacity);
		array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
	}
	// If the capacity will not be exceeded, do nothing
	// Add the value to the "array"'s values and then increment its counter
	array->values[array->count] = value;
	array->count++;
}

void freeValueArray(ValueArray *array)
{
	FREE_ARRAY(Value, array->values, array->capacity);
	// Reinit to leave "array" in well-defined state
	initValueArray(array);
}

void printValue(Value value)
{
	printf("%g", AS_NUMBER(value));
}