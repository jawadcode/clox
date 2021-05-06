#ifndef clox_object_h
#define clox_object_h

#include "chunk.h"
#include "common.h"
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
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

// Different types of heap allocated objects
typedef enum {
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_STRING,
  OBJ_UPVALUE,
} ObjType;

/*
   Every other ObjThing type can be safely casted to type "Obj"
   and the "type" field can be accessed because of how C arranges memory
   for a struct Therefore, every other object type is (in the OOP sense) also an
   "Obj" (but that does not mean every object type can safely be converted to
   any other object type)
*/

struct Obj {
  ObjType type;
  bool isMarked;
  struct Obj *next;
};

typedef struct {
  Obj obj;
  int arity;
  int upvalueCount;
  Chunk chunk;
  ObjString *name;
} ObjFunction;

// Pointer to native/built-in function
typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct {
  Obj obj;
  NativeFn function;
} ObjNative;

struct ObjString {
  Obj obj;
  int length;
  char *chars;
  uint32_t hash;
};

// Captures value from stack as upvalue for use in closures
typedef struct ObjUpvalue {
  Obj obj;
  Value *location;
  Value closed;
  struct ObjUpvalue *next;
} ObjUpvalue;

/* Closure which wraps every function:
    - "obj" allows us to upcast to an "Obj"
    - "function" is the function that the closure is wrapping
    - "upvalues" is an array of upvalues captured from the value stack
    - "upvalueCount" is the number of upvalues
 */
typedef struct {
  Obj obj;
  ObjFunction *function;
  ObjUpvalue **upvalues;
  int upvalueCount;
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

// Turn value in stack into upvalue so it can be used in a closure
ObjUpvalue *newUpvalue(Value *slot);

// Print any Object
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
