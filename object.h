#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

// Type check macros

#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_STRING(value) isObjType(value, OBJ_STRING)

// Conversion macros

#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_NATIVE(value) \
	(((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

// Different types of heap allocated objects
typedef enum
{
	OBJ_CLOSURE,
	OBJ_FUNCTION,
	OBJ_NATIVE,
	OBJ_STRING,
} ObjType;

/*
	Every other ObjThing type can be safely casted to type "Obj"
	and the "type" field can be accessed because of how C arranges memory for a struct
	Therefore, every other object type is (in the OOP sense) also an "Obj"
	(but that does not mean every object type can safely be converted to any other object type)
*/

struct Obj
{
	ObjType type;
	struct Obj *next;
};

typedef struct
{
	Obj obj;
	int arity;
	Chunk chunk;
	ObjString *name;
} ObjFunction;

// Pointer to native/built-in function
typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct
{
	Obj obj;
	NativeFn function;
} ObjNative;

struct ObjString
{
	Obj obj;
	int length;
	char *chars;
	uint32_t hash;
};

typedef struct
{
	Obj obj;
	ObjFunction *function;
} ObjClosure;

// Wrap an "ObjFunction" in a closure
ObjClosure *newClosure(ObjFunction *function);

// Allocate and initialise a new function object and return it
ObjFunction *newFunction();

// Allocate and initialise native function
ObjNative *newNative(NativeFn function);

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