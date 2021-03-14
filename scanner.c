#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

Scanner scanner;

void initScanner(const char *source)
{
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}