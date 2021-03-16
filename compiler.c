#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

/* State for parser:
 * - "current" is the current token being parsed
 * - "previous" is the previous token
 * - "hadError" is passed to the caller of "compile"
 * - "panicMode" is used for error recovery
 */
typedef struct
{
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;

Parser parser;

Chunk *compilingChunk;

static Chunk *currentChunk()
{
	return compilingChunk;
}

static void errorAt(Token *token, const char *message)
{
	// If there has already been an error then supress this one
	if (parser.panicMode)
		return;
	// Set panic mode to true to supress further errors
	parser.panicMode = true;

	// Print line at which the error occurred
	fprintf(stderr, "[line %d] Error", token->line);

	// Positional part of error
	if (token->type == TOKEN_EOF)
		fprintf(stderr, " at end");
	else if (token->type == TOKEN_ERROR)
		0; // Nothing
	else
		fprintf(stderr, " at '%.*s'", token->length, token->start);

	// Print actual error
	fprintf(stderr, ": %s\n", message);
	parser.hadError = true;
}

static void errorAtCurrent(const char *message)
{
	errorAt(&parser.current, message);
}

static void error(const char *message)
{
	errorAt(&parser.previous, message);
}

static void advance()
{
	parser.previous = parser.current;

	for (;;)
	{
		parser.current = scanToken();
		if (parser.current.type != TOKEN_ERROR)
			break;
		// else
		errorAtCurrent(parser.current.start);
	}
}

// Consume until the desired token
static void consume(TokenType type, const char *message)
{
	if (parser.current.type == type)
	{
		advance();
		return;
	}

	errorAtCurrent(message);
}

// Write single OpCode to current chunk with line number
static void emitByte(uint8_t byte)
{
	writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
	emitByte(byte1);
	emitByte(byte2);
}

// Emit "OP_RETURN" instruction and end program
static void emitReturn()
{
	emitByte(OP_RETURN);
}

static void endCompiler()
{
	emitReturn();
}

bool compile(const char *source, Chunk *chunk)
{
	initScanner(source);
	compilingChunk = chunk;

	parser.hadError = false;
	parser.panicMode = false;

	advance();
	expression();
	consume(TOKEN_EOF, "Expected end of expression");

	endCompiler();	

	return !parser.hadError;
}