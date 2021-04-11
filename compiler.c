#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

/* Parser State:
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

// Different levels of precedence
typedef enum
{
	PREC_NONE,
	PREC_ASSIGNMENT, // =
	PREC_OR,				 // or
	PREC_AND,				 // and
	PREC_EQUALITY,	 // == !=
	PREC_COMPARISON, // < > <= >=
	PREC_TERM,			 // + -
	PREC_FACTOR,		 // * /
	PREC_UNARY,			 // ! -
	PREC_CALL,			 // . ()
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

/* A Parser Rule
	 - "prefix" is a pointer to a parser function for the token if it were a prefix
	 - "info" is a pointer to a parser function for the token if it were infix
	 - "precedence" is the parsing precedence of the token relative to other tokens
 */
typedef struct
{
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

/* A Local Variable
	 - "name" is the Token storing the name of the variable
	 - "depth" is the scope depth of the local variable
 */
typedef struct
{
	Token name;
	int depth;
} Local;

/* The two different types of function:
	 - "TYPE_FUNCTION" is just a normal function
	 - "TYPE_SCRIPT" is the top level of a script but is still treated as a function
 */
typedef enum
{
	TYPE_FUNCTION,
	TYPE_SCRIPT,
} FunctionType;

/* Compiler state
	 - "function" is the current function
	 - "type" is the type of the current function
	 - "locals" stores all of the local variables
	 - "localCount" is the current number of local variables in "locals"
	 - "scopeDepth" is the current scope depth
 */
typedef struct
{
	ObjFunction *function;
	FunctionType type;

	Local locals[UINT8_COUNT];
	int localCount;
	int scopeDepth;
} Compiler;

Parser parser;

Chunk *compilingChunk;

Compiler *current = NULL;

static Chunk *currentChunk()
{
	return &current->function->chunk;
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

// Advance to next token
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

// Check the if the TokenType of the current token is equal to "token"
static inline bool check(TokenType type)
{
	return parser.current.type == type;
}

static bool match(TokenType type)
{
	if (!check(type))
		return false;
	advance();
	return true;
}

// Write single OpCode to current chunk with line number
static void emitByte(uint8_t byte)
{
	writeChunk(currentChunk(), byte, parser.previous.line);
}

// For instructions that span two bytes (second byte is an operand)
static void emitBytes(uint8_t byte1, uint8_t byte2)
{
	emitByte(byte1);
	emitByte(byte2);
}

// Write "OP_LOOP" instruction
static void emitLoop(int loopStart)
{
	emitByte(OP_LOOP);
	// Size of loop: end minus start plus the size of the "OP_LOOP" instruction itself
	int offset = currentChunk()->count - loopStart + 2;
	if (offset > UINT16_MAX)
		error("Loop body is too large");

	// Split 16 bit integer into two 8 bit integers
	emitByte((offset >> 8) & 0xff);
	emitByte(offset & 0xff);
}

// Emit jump instruction followed by a 16 bit placeholder
// and then return the position of the instruction
static int emitJump(uint8_t instruction)
{
	emitByte(instruction);
	emitByte(0xff);
	emitByte(0xff);
	return currentChunk()->count - 2;
}

// Emit "OP_RETURN" instruction and end program
static void emitReturn()
{
	emitByte(OP_RETURN);
}

// Add value to constants array
static uint8_t makeConstant(Value value)
{
	int constant = addConstant(currentChunk(), value);
	if (constant > UINT8_MAX)
	{
		error("Too many constants in a single chunk");
		return 0;
	}

	return (uint8_t)constant;
}

// Emit "OP_CONSTANT" instruction using "value" as the constant
static void emitConstant(Value value)
{
	emitBytes(OP_CONSTANT, makeConstant(value));
}

// Patch a jump instruction with the correct value
static void patchJump(int offset)
{
	// -2 to adjust for the jump offset's bytecode itself
	int jump = currentChunk()->count - offset - 2;

	if (jump > UINT16_MAX)
		error("Too much code to jump over");

	currentChunk()->code[offset] = (jump >> 8) & 0xff;
	currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler *compiler, FunctionType type)
{
	compiler->function = NULL;
	compiler->type = type;
	compiler->localCount = 0;
	compiler->scopeDepth = 0;
	compiler->function = newFunction();
	current = compiler;

	Local *local = &current->locals[current->localCount++];
	local->depth = 0;
	local->name.start = "";
	local->name.length = 0;
}

static ObjFunction *endCompiler()
{
	emitReturn();
	ObjFunction *function = current->function;
#ifdef DEBUG_PRINT_CODE
	if (!parser.hadError)
		disassembleChunk(currentChunk(), function->name != NULL
																				 ? function->name->chars
																				 : "<script>");
#endif
	return function;
}

// Begins new scope and increments "current->scopeDepth"
static void beginScope()
{
	current->scopeDepth++;
}

// End the current scope and decrement "current->scopeDepth"
static void endScope()
{
	current->scopeDepth--;

	while (current->localCount > 0 &&
				 current->locals[current->localCount - 1].depth >
						 current->scopeDepth)
	{
		emitByte(OP_POP);
		current->localCount--;
	}
}

// Forward declarations

// Parse and compile expression
static void expression();
// Parse and compile statement
static void statement();
// Parse and compile declaration
static void declaration();
// Get ParseRule from TokenType
static ParseRule *getRule(TokenType type);
// Parse tokens until the token's precedence is higher than "precedence"
static void parsePrecedence(Precedence precedence);

// Takes in token "name" and adds its lexeme to the chunk's constants table as a string
// its index in the table is then returned
static uint8_t identifierConstant(Token *name)
{
	return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static void addLocal(Token name)
{
	if (current->localCount == UINT8_COUNT)
	{
		error("Too many local variables in function");
		return;
	}
	// "localCount"'s value is used before incrementing

	Local *local = &current->locals[current->localCount++];
	local->name = name;
	local->depth = -1;
}

// Checks if 2 identifier tokens are equal
static bool identifiersEqual(Token *a, Token *b)
{
	if (a->length != b->length)
		return false;
	return memcmp(a->start, b->start, a->length) == 0;
}

// Walk list of locals backwards to find most recently declared variable with the identifier
static int resolveLocal(Compiler *compiler, Token *name)
{
	for (int i = compiler->localCount - 1; i >= 0; i--)
	{
		Local *local = &compiler->locals[i];
		if (identifiersEqual(name, &local->name))
		{
			if (local->depth == -1)
				error("Can't read local variable from its own initializer");

			return i;
		}
	}

	// Variable does not exist
	return -1;
}

static void declareVariable()
{
	// Check if global variable
	if (current->scopeDepth == 0)
		return;
	// else
	Token *name = &parser.previous;
	// Walk current scope's list of locals backwards to check the variable has already been declared
	for (int i = current->localCount - 1; i >= 0; i--)
	{
		Local *local = &current->locals[i];
		if (local->depth != -1 && local->depth < current->scopeDepth)
			break;

		if (identifiersEqual(name, &local->name))
			error("Variable already exists in current scope");
	}

	addLocal(*name);
}

// Requires next token to be identifier and then returns index of constant in table from "identifierConstant()"
static uint8_t parseVariable(const char *errorMessage)
{
	consume(TOKEN_IDENTIFIER, errorMessage);

	declareVariable();
	if (current->scopeDepth > 0)
		return 0;

	return identifierConstant(&parser.previous);
}

// Mark variable as usable by setting the scope depth
static void markInitialized()
{
	current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global)
{
	if (current->scopeDepth > 0)
	{
		return markInitialized();
	}

	emitBytes(OP_DEFINE_GLOBAL, global);
}

// Parse and compile "and" operation
static void and_(bool canAssign)
{
	// "and" operator short circuits (if the left operand is false) so there needs to be a jump if false

	int endJump = emitJump(OP_JUMP_IF_FALSE);

	emitByte(OP_POP);
	parsePrecedence(PREC_AND);

	patchJump(endJump);
}

// Compile binary expression
static void binary(bool canAssign)
{
	// Remember the operator
	TokenType operatorType = parser.previous.type;

	// Compile the operator to the right
	ParseRule *rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->precedence + 1));

	// Emit the operator instruction
	switch (operatorType)
	{
	case TOKEN_BANG_EQUAL:
		emitBytes(OP_EQUAL, OP_NOT);
		break;
	case TOKEN_EQUAL_EQUAL:
		emitByte(OP_EQUAL);
		break;
	case TOKEN_GREATER:
		emitByte(OP_GREATER);
		break;
	case TOKEN_GREATER_EQUAL:
		emitBytes(OP_LESS, OP_NOT);
		break;
	case TOKEN_LESS:
		emitByte(OP_LESS);
		break;
	case TOKEN_LESS_EQUAL:
		emitBytes(OP_GREATER, OP_NOT);
		break;
	case TOKEN_PLUS:
		emitByte(OP_ADD);
		break;
	case TOKEN_MINUS:
		emitByte(OP_SUBTRACT);
		break;
	case TOKEN_STAR:
		emitByte(OP_MULTIPLY);
		break;
	case TOKEN_SLASH:
		emitByte(OP_DIVIDE);
		break;
	default:
		return; // Unreachable.
	}
}

static void literal(bool canAssign)
{
	switch (parser.previous.type)
	{
	case TOKEN_FALSE:
		emitByte(OP_FALSE);
		break;
	case TOKEN_NIL:
		emitByte(OP_NIL);
		break;
	case TOKEN_TRUE:
		emitByte(OP_TRUE);
		break;

	default:
		return; // Unreachable
	}
}

// Compile grouping wrapped in parentheses
static void grouping(bool canAssign)
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression");
}

static void number(bool canAssign)
{
	double value = strtod(parser.previous.start, NULL);
	emitConstant(NUMBER_VAL(value));
}

// Parse and compile "or" operation
static void or_(bool canAssign)
{
	int elseJump = emitJump(OP_JUMP_IF_FALSE);
	int endJump = emitJump(OP_JUMP);

	patchJump(elseJump);
	emitByte(OP_POP);

	parsePrecedence(PREC_OR);
	patchJump(endJump);
}

static void string(bool canAssign)
{
	emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
																	parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign)
{
	uint8_t getOp, setOp;
	int arg = resolveLocal(current, &name);
	if (arg != -1)
	{
		getOp = OP_GET_LOCAL;
		setOp = OP_SET_LOCAL;
	}
	else
	{
		arg = identifierConstant(&name);
		getOp = OP_GET_GLOBAL;
		setOp = OP_SET_GLOBAL;
	}

	if (canAssign && match(TOKEN_EQUAL))
	{
		expression();
		emitBytes(setOp, (uint8_t)arg);
	}
	else
		emitBytes(getOp, (uint8_t)arg);
}

static void variable(bool canAssign)
{
	namedVariable(parser.previous, canAssign);
}

// Compile unary expression
static void unary(bool canAssign)
{
	TokenType operatorType = parser.previous.type;

	// Compile the operand
	parsePrecedence(PREC_UNARY);

	// Emit the operator instruction
	switch (operatorType)
	{
	case TOKEN_BANG:
		emitByte(OP_NOT);
		break;
	case TOKEN_MINUS:
		emitByte(OP_NEGATE);
		break;

	default:
		return; // Unreachable
	}
}

ParseRule rules[] = {
		[TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_NONE},
		[TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
		[TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
		[TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
		[TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
		[TOKEN_DOT] = {NULL, NULL, PREC_NONE},
		[TOKEN_MINUS] = {unary, binary, PREC_TERM},
		[TOKEN_PLUS] = {NULL, binary, PREC_TERM},
		[TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
		[TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
		[TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
		[TOKEN_BANG] = {unary, NULL, PREC_NONE},
		[TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
		[TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
		[TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
		[TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
		[TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
		[TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
		[TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
		[TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
		[TOKEN_STRING] = {string, NULL, PREC_NONE},
		[TOKEN_NUMBER] = {number, NULL, PREC_NONE},
		[TOKEN_AND] = {NULL, and_, PREC_AND},
		[TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
		[TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
		[TOKEN_FALSE] = {literal, NULL, PREC_NONE},
		[TOKEN_FOR] = {NULL, NULL, PREC_NONE},
		[TOKEN_FUN] = {NULL, NULL, PREC_NONE},
		[TOKEN_IF] = {NULL, NULL, PREC_NONE},
		[TOKEN_NIL] = {literal, NULL, PREC_NONE},
		[TOKEN_OR] = {NULL, or_, PREC_OR},
		[TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
		[TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
		[TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
		[TOKEN_THIS] = {NULL, NULL, PREC_NONE},
		[TOKEN_TRUE] = {literal, NULL, PREC_NONE},
		[TOKEN_VAR] = {NULL, NULL, PREC_NONE},
		[TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
		[TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
		[TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static void parsePrecedence(Precedence precedence)
{
	advance();
	ParseFn prefixRule = getRule(parser.previous.type)->prefix;
	if (prefixRule == NULL)
	{
		error("Expected expression");
		return;
	}

	bool canAssign = precedence <= PREC_ASSIGNMENT;
	prefixRule(canAssign);

	while (precedence <= getRule(parser.current.type)->precedence)
	{
		advance();
		ParseFn infixRule = getRule(parser.previous.type)->infix;
		infixRule(canAssign);
	}

	if (canAssign && match(TOKEN_EQUAL))
		error("Invalid assignment target");
}

static ParseRule *getRule(TokenType type)
{
	return &rules[type];
}

// Parse and compile expression
static void expression()
{
	// Parse the lowest precedence level
	parsePrecedence(PREC_ASSIGNMENT);
}

static void block()
{
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
	{
		declaration();
	}

	consume(TOKEN_RIGHT_BRACE, "Expected '}' after block");
}

// Parse and compile variable declaration
static void varDeclaration()
{
	uint8_t global = parseVariable("Expected variable name");

	// If there is an "=" then parse the expression after it
	if (match(TOKEN_EQUAL))
		expression();
	// If it is just "var thing;" then assign nil to the variable
	else
		emitByte(OP_NIL);

	consume(TOKEN_SEMICOLON, "Expected ';' after variable declaration");
	defineVariable(global);
}

// Parse and compile an expression statement
static void expressionStatement()
{
	expression();
	consume(TOKEN_SEMICOLON, "Expected ';' after expression");
	emitByte(OP_POP);
}

static void forStatement()
{
	// Want to wrap entire loop in scope to prevent counter being accessed outside the loop
	beginScope();

	consume(TOKEN_LEFT_PAREN, "Expected '(' after 'for'");
	if (match(TOKEN_SEMICOLON))
	{
		// No initializer
	}
	else if (match(TOKEN_VAR))
		varDeclaration();
	else
		expressionStatement();

	int loopStart = currentChunk()->count;

	int exitJump = -1;
	// If there is a condition clause
	if (!match(TOKEN_SEMICOLON))
	{
		expression();
		consume(TOKEN_SEMICOLON, "Expected ';' after loop condition");

		// Jump out of loop if the condition is false
		exitJump = emitJump(OP_JUMP_IF_FALSE);
		// Pop result of loop condition from stack into the jump instruction
		emitByte(OP_POP);
	}

	// If there is an increment clause
	if (!match(TOKEN_RIGHT_PAREN))
	{
		// Emit jump to the end of the loop (because increment should run after the body)
		int bodyJump = emitJump(OP_JUMP);

		// Get position of the increment
		int incrementStart = currentChunk()->count;
		// Compile it
		expression();
		// Emit a pop to discard the value
		emitByte(OP_POP);
		consume(TOKEN_RIGHT_PAREN, "Expect ')' after for loop clauses");

		// Loop back
		emitLoop(loopStart);
		loopStart = incrementStart;
		patchJump(bodyJump);
	}

	statement();

	emitLoop(loopStart);

	// If there is a condition clause
	if (exitJump != -1)
	{
		patchJump(exitJump);
		// Pop result of condition from stack into jump instruction
		emitJump(OP_POP);
	}

	endScope();
}

// Parse and compile an if statement (including else if it is included)
static void ifStatement()
{
	// Compile the condition so the result is left on the top of the stack
	consume(TOKEN_LEFT_PAREN, "Expected '(' after 'if'");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expected ')' after condition");

	// Emit jump instruction with placeholder value
	// (because we don't know how far to jump ahead yet)
	int thenJump = emitJump(OP_JUMP_IF_FALSE);
	// Statements must have a net 0 impact on the stack
	// so emit a pop to clean up the conditional value
	emitByte(OP_POP);
	// Compile 'then' block
	statement();

	// Emit jump instruction to skip else statement
	int elseJump = emitJump(OP_JUMP);

	// Patch the jump instruction with the actual length to jump
	patchJump(thenJump);
	// Same as last one
	emitByte(OP_POP);

	// Look for "else" and then compile the following block/statement
	if (match(TOKEN_ELSE))
		statement();
	// Patch the jump instruction for the else statement
	patchJump(elseJump);
}

// Parses print statement and emits "OP_PRINT" instruction
static void printStatement()
{
	expression();
	consume(TOKEN_SEMICOLON, "Expected ';' after value");
	emitByte(OP_PRINT);
}

// Parse and compile while statement
static void whileStatement()
{
	// Position of the start of the loop (in terms of bytecode) so we can actually loop
	int loopStart = currentChunk()->count;

	consume(TOKEN_LEFT_PAREN, "Expetced '(' after 'while'");
	// Parse the condition
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expected ')' after condition");

	int exitJump = emitJump(OP_JUMP_IF_FALSE);

	emitByte(OP_POP);
	// Parse the block
	statement();

	emitLoop(loopStart);

	patchJump(exitJump);
	emitByte(OP_POP);
}

// Skips tokens indiscriminately until statement boundary
static void synchronize()
{
	parser.panicMode = false;
	while (parser.current.type != TOKEN_EOF)
	{
		if (parser.previous.type == TOKEN_SEMICOLON)
			return;

		switch (parser.current.type)
		{
		case TOKEN_CLASS:
		case TOKEN_FUN:
		case TOKEN_VAR:
		case TOKEN_FOR:
		case TOKEN_IF:
		case TOKEN_WHILE:
		case TOKEN_PRINT:
		case TOKEN_RETURN:
			return;

		default:
				// Do nothing.
				;
		}

		advance();
	}
}

static void declaration()
{
	if (match(TOKEN_VAR))
		varDeclaration();
	else
		statement();

	if (parser.panicMode)
		synchronize();
}

static void statement()
{
	if (match(TOKEN_PRINT))
		printStatement();
	else if (match(TOKEN_FOR))
		forStatement();
	else if (match(TOKEN_IF))
		ifStatement();
	else if (match(TOKEN_WHILE))
		whileStatement();
	else if (match(TOKEN_LEFT_BRACE))
	{
		beginScope();
		block();
		endScope();
	}
	else
		expressionStatement();
}

ObjFunction *compile(const char *source)
{
	initScanner(source);
	Compiler compiler;
	initCompiler(&compiler, TYPE_SCRIPT);

	parser.hadError = false;
	parser.panicMode = false;

	advance();

	while (!match(TOKEN_EOF))
		declaration();

	endCompiler();

	ObjFunction *function = endCompiler();
	return parser.hadError ? NULL : function;
}