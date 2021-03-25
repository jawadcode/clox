#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

// Type check macros

#define IS_STRING(value) isObjType(value, OBJ_STRING)

// Conversion macros

#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

// Different types of heap allocated objects
typedef enum
{
	OBJ_STRING,
} ObjType;

// Every other object type can be safely casted to type "Obj"
// and the "type" field can be accessed because of how C arranges memory for a struct
// Therefore, every other object type is (in the OOP sense) also an "Obj"
// (but that does not mean every object type can safely be converted to any other object type)

struct Obj
{
	ObjType type;
	struct Obj *next;
};

struct ObjString
{
	Obj obj;
	int length;
	char *chars;
};

// Take ownership of string (instead of copying) and wrap in string object
ObjString *takeString(char *chars, int length);

// Copy string over from source code string
// (to prevent trying to free parts of the original string)
// and return it wrapped in an "ObjString"
ObjString *copyString(const char *chars, int length);

// Print any Object
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type)
{
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif