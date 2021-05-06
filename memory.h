#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"
#include "object.h"

// Alloc space for array of type "type" and length "count"
#define ALLOCATE(type, count)                                                  \
  (type *)reallocate(NULL, 0, sizeof(type) * (count))

// Resize array to 0 bytes
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

// GROW_CAPACITY grows a given capacity to either 8 if capacity < 8 or double if
// capacity >= 8
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)

// GROW_ARRAY grows an array by reallocating <newCount> more bytes of memory for
// "pointer"
#define GROW_ARRAY(type, pointer, oldCount, newCount)                          \
  (type *)reallocate(pointer, sizeof(type) * (oldCount),                       \
                     sizeof(type) * (newCount))

// FREE_ARRAY uses "reallocate" to resize the array to 0
#define FREE_ARRAY(type, pointer, oldCount)                                    \
  reallocate(pointer, sizeof(type) * (oldCount), 0)

/* Reallocates space for an array based on these cases:
         - if oldSize == 0 and newSize != 0 then allocate a new block
         - if oldSize != 0 and newSize == 0 then free the existing allocation
         - if oldSize != 0 and newSize < oldSize then shrink the existing
   allocation
         - and finally, if oldSize != 0 and newSize > oldSize then grow the
   existing allocation.
 */
void *reallocate(void *pointer, size_t oldSize, size_t newSize);

// Check if an object is no longer in use
void markObject(Obj *object);

// Mark value if it is heap allocated and out of use
void markValue(Value value);

// Collects unused memory
void collectGarbage();

// Free all of the object references in the linked list
void freeObjects();

#endif
