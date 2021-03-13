#include <stdlib.h>

#include "memory.h"

void *reallocate(void *pointer, size_t oldSize, size_t newSize)
{
	// Free all space used and return null pointer if newSize is 0
	if (newSize == 0)
	{
		free(pointer);
		return NULL;
	}

	// Resize array efficiently with realloc
	void *result = realloc(pointer, newSize);
	// If there is no more memory left to allocate then exit early to prevent any weird behaviour
	if (result == NULL)
		exit(1);
	// Return pointer to reallocated array
	return result;
}