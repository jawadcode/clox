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

static bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

static bool isAtEnd()
{
	return *scanner.current == '\0';
}

// Consume the current character and then return it
static char advance()
{
	scanner.current++;
	return scanner.current[-1]; // Desugars to *(scanner.current - 1)
}

// Peek ahead without incrementing "scanner.current"
static char peek()
{
	return *scanner.current;
}

// Peek 1 past current character (mostly just for comments)
static char peekNext()
{
	// This check prevents ~~nasal demons~~ out of bounds indexing
	if (isAtEnd())
		return '\0';
	return scanner.current[1]; // Desugars to *(scanner.current + 1)
}

// Check if current character == "expected" and if so,
// advance to the next character and return true,
// if not, then return false
static bool match(char expected)
{
	if (isAtEnd())
		return false;
	if (*scanner.current != expected)
		return false;

	scanner.current++;
	return true;
}

// Make token from current lexeme using specified type
static Token makeToken(TokenType type)
{
	Token token;
	token.type = type;
	token.start = scanner.start;
	token.length = (int)(scanner.current - scanner.start);
	token.line = scanner.line;

	return token;
}

// Fancy error handling using the Token itself
static Token errorToken(const char *message)
{
	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = scanner.line;

	return token;
}

static void skipWhitespace()
{
	for (;;)
	{
		char c = peek();

		switch (c)
		{
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;
		case '\n':
			scanner.line++;
			advance();
			break;
		// Treat comments as whitespace too
		case '/':
			if (peekNext() == '/')
			{
				// "peek()" checks for the "\n" but does not consume it
				// this is so the '\n' case can increment the line number
				while (peek() != '\n' && !isAtEnd())
					advance();
			}
			else
				return;
			break;

		default:
			return;
		}
	}
}

static Token number()
{
	// Consume integer part of number
	while (isDigit(peek()))
		advance();

	// Look for decimal point
	if (peek() == '.' && isDigit(peekNext()))
	{
		advance();
		// Consume fractional part of number
		while (isDigit(peek()))
			advance();
	}

	// No conversion to double here (because that would require a union type)

	return makeToken(TOKEN_NUMBER);
}

static Token string()
{
	// Consume all of the characters wrapped by the quotes
	while (peek() != '"' && !isAtEnd())
	{
		// Manually increment line number because skipWhitespace() isn't being called
		if (peek() == '\n')
			scanner.line++;
		advance();
	}

	if (isAtEnd())
		return errorToken("Unterminated string constant");

	// Consume closing quote
	advance();
	return makeToken(TOKEN_STRING);
}

Token scanToken()
{
	skipWhitespace();

	scanner.start = scanner.current;

	if (isAtEnd())
		return makeToken(TOKEN_EOF);

	char c = advance();
	if (isDigit(c))
		return number();

	switch (c)
	{
	case '(':
		return makeToken(TOKEN_LEFT_PAREN);
	case ')':
		return makeToken(TOKEN_RIGHT_PAREN);
	case '{':
		return makeToken(TOKEN_LEFT_BRACE);
	case '}':
		return makeToken(TOKEN_RIGHT_BRACE);
	case ';':
		return makeToken(TOKEN_SEMICOLON);
	case ',':
		return makeToken(TOKEN_COMMA);
	case '.':
		return makeToken(TOKEN_DOT);
	case '-':
		return makeToken(TOKEN_MINUS);
	case '+':
		return makeToken(TOKEN_PLUS);
	case '/':
		return makeToken(TOKEN_SLASH);
	case '*':
		return makeToken(TOKEN_STAR);
	case '!':
		return makeToken(
				match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
	case '=':
		return makeToken(
				match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
	case '<':
		return makeToken(
				match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
	case '>':
		return makeToken(
				match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
	case '"':
		return string();
	}

	return errorToken("Unexpected character");
}