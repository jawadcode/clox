#include <stdio.h>
#include <string.h>

#include "object.h"
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
	switch (value.type)
	{
	case VAL_BOOL:
		printf(AS_BOOL(value) ? "true" : "false");
		break;
	case VAL_NIL:
		printf("nil");
		break;
	case VAL_NUMBER:
		printf("%g", AS_NUMBER(value));
		break;
	case VAL_OBJ:
		printObject(value);
		break;
	}
}

bool valuesEqual(Value a, Value b)
{
	// Checks equality of types too because we don't want another JS
	if (a.type != b.type)
		return false;

	switch (a.type)
	{
	case VAL_BOOL:
		return AS_BOOL(a) == AS_BOOL(b);
	case VAL_NIL:
		return true;
	case VAL_NUMBER:
		return AS_NUMBER(a) == AS_NUMBER(b);
	case VAL_OBJ:
		return AS_OBJ(a) == AS_OBJ(b);
	default:
		return false; // Unreachable
	}
}