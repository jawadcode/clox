#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

// Allocate space for any object type
#define ALLOCATE_OBJ(type, objectType) \
	(type *)allocateObject(sizeof(type), objectType)

// "Generic" function which allows you to allocate space for any object type but as a "Obj *" to avoid redundant casting from "void *"
static Obj *allocateObject(size_t size, ObjType type)
{
	Obj *object = (Obj *)reallocate(NULL, 0, size);
	object->type = type;

	object->next = vm.objects;
	vm.objects = object;
	return object;
}

// Construct ObjString from freshly copied C string
static ObjString *allocateString(char *chars, int length)
{
	ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
	string->length = length;
	string->chars = chars;

	return string;
}

ObjString *takeString(char *chars, int length)
{
	return allocateString(chars, length);
}

ObjString *copyString(const char *chars, int length)
{
	char *heapChars = ALLOCATE(char, length + 1);
	memcpy(heapChars, chars, length);
	// Manually null terminate because the lexeme points to a section of the original source string
	// and therefore won't have a null terminator
	heapChars[length] = '\0';

	return allocateString(heapChars, length);
}

void printObject(Value value)
{
	switch (OBJ_TYPE(value))
	{
	case OBJ_STRING:
		printf("%s", AS_CSTRING(value));
		break;
	}
}