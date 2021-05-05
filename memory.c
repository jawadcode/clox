#include <stdlib.h>

#include "memory.h"
#include "object.h"
#include "vm.h"

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
  // Free all space used and return null pointer if newSize is 0
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }

  // Resize array efficiently with realloc
  void *result = realloc(pointer, newSize);
  // If there is no more memory left to allocate then exit early to prevent any
  // weird behaviour
  if (result == NULL)
    exit(1);
  // Return pointer to reallocated array
  return result;
}

static void freeObject(Obj *object) {
  switch (object->type) {
  case OBJ_CLOSURE: {
    ObjClosure *closure = (ObjClosure *)object;
    FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
    FREE(ObjClosure, object);
    break;
  }
  case OBJ_FUNCTION: {
    ObjFunction *function = (ObjFunction *)object;
    freeChunk(&function->chunk);
    FREE(ObjFunction, object);
    break;
  }
  case OBJ_NATIVE:
    FREE(ObjNative, object);
    break;
  case OBJ_STRING: {
    ObjString *string = (ObjString *)object;
    FREE_ARRAY(char, string->chars, string->length + 1);
    // FREE instead of free to help the VM track memory usage
    FREE(ObjString, object);
    break;
  }
  case OBJ_UPVALUE:
    FREE(ObjUpvalue, object);
    break;
  }
}

void freeObjects() {
  Obj *object = vm.objects;
  // Walk through linked list and free objects
  while (object != NULL) {
    Obj *next = object->next;
    freeObject(object);
    object = next;
  }
}
