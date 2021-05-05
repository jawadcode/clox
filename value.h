#ifndef clox_value_h
#define clox_value_h

#include "common.h"

// Begin Forward Declarations:

// Heap Allocated Object type
typedef struct Obj Obj;
// String Object
typedef struct ObjString ObjString;

// End Forward Declarations

// All of Lox's data types
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ, // Heap allocated values
} ValueType;

// Tagged union that represents Lox value
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj *obj;
  } as;
} Value;

// Type check macros

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

// Conversion macros

#define AS_OBJ(value) ((value).as.obj)
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)object}})

/* Struct containing array of constant Values with:
        - "count" as the index of the next "Value" to be added
        - "capacity" as the allocated size of the array
        - "values" as the actual, dynamically sized array of "Value"s
 */
typedef struct {
  int count;
  int capacity;
  Value *values;
} ValueArray;

// Check if values "a" and "b" are equal
bool valuesEqual(Value a, Value b);

// Initialise "array" by zeroing-out values
void initValueArray(ValueArray *array);
// Free all memory used by "array->values" and reinitialise "array"
void freeValueArray(ValueArray *array);
// Write "value" to "array->values"
void writeValueArray(ValueArray *array, Value value);

// Print constant value
void printValue(Value value);

#endif