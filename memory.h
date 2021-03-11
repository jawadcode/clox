#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

// GROW_CAPACITY grows a given capacity to either 8 if capacity < 8 or double if capacity >= 8
#define GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity)*2)

// GROW_ARRAY grows an array by reallocating <newCount> more bytes of memory for "pointer"
#define GROW_ARRAY(type, pointer, oldCount, newCount)    \
	(type *)reallocate(pointer, sizeof(type) * (oldCount), \
										 sizeof(type) * (newCount))

// FREE_ARRAY uses "reallocate" to resize the array to 0
#define FREE_ARRAY(type, pointer, oldCount) \
	reallocate(pointer, sizeof(type) * (oldCount), 0)

/* Reallocates space for an array based on these cases:
	 - if oldSize == 0 and newSize != 0 then allocate a new block
	 - if oldSize != 0 and newSize == 0 then free the allocation
	 - if oldSize != 0 and newSize < oldSize then shrink the existing allocation
	 - and finally, if oldSize != 0 and newSize > oldSize then grow the existing allocation.
	 - (exits program if pointer returned from realloc is NULL)
	 - (returns pointer to array)
 */
void *reallocate(void *pointer, size_t oldSize, size_t newSize);

#endif