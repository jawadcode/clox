#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

void compile(const char *source)
{
	initScanner(source);

	// Little trick to make it print the first line number
	int line = -1;

	for (;;)
	{
		Token token = scanToken();
		// Print line number ("token.line") if it differs with the previous ("line")
		// Otherwise, print "   | "
		if (token.line != line)
		{
			printf("%4d ", token.line);
			line = token.line;
		}
		else
		{
			printf("   | ");
		}

		// Print token information
		printf("%2d '%.*s'\n", token.type, token.length, token.start);

		// End Of File
		if (token.type == TOKEN_EOF)
			break;
	}
}