#include <stdlib.h>

#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include "debug.h"
#include <stdio.h>
#endif

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
  if (newSize > oldSize) {
    // Bad for performance but exposes any bugs since the GC is triggered at
    // every moment
#ifdef DEBUG_STRESS_GC
    collectGarbage();
#endif
  }
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

void markObject(Obj *object) {
  if (object == NULL)
    return;
#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void *)object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif
  object->isMarked = true;
}

void markValue(Value value) {
  if (IS_OBJ(value))
    markObject(AS_OBJ(value));
}

static void freeObject(Obj *object) {
#ifdef DEBUG_LOG_GC
  printf("%p free type %d\n", (void *)object, object->type);
#endif
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

static void markRoots() {
  for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
    markValue(*slot);
  }

  for (int i = 0; i < vm.frameCount; i++) {
    markObject((Obj *)vm.frames[i].closure);
  }

  for (ObjUpvalue *upvalue = vm.openUpvalues; upvalue != NULL;
       upvalue = upvalue->next) {
    markObject((Obj *)upvalue);
  }

  markTable(&vm.globals);
  markCompilerRoots();
}

void collectGarbage() {
#ifdef DEBUG_LOG_GC
  printf("-- gc begin\n");
#endif
  markRoots();
#ifdef DEBUG_LOG_GC
  printf("-- gc end\n");
#endif
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
