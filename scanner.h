#ifndef clox_scanner_h
#define clox_scanner_h

#include "common.h"
#include "scanner.h"

/* Scanner state:
 * "start" points to the beginning of the current lexeme being scanned
 * "current" points to the current character being looked at
 * "line" is the current line number
 */
typedef struct
{
	const char *start;
	const char *current;
	int line;
} Scanner;

// Initialises global scanner state
void initScanner(const char *source);

#endif