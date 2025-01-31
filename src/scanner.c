#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "scanner.h"
#include "error.h"
#include "keywords.h"

Scanner *scanner;

char *readFile(const char *path) {
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		fprintf(stderr, "Failed to open file '%s'\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char *buffer = (char*)malloc(fileSize + 1);
	if (buffer == NULL) {
		fprintf(stderr, "Failed to allocate memory for file '%s'\n", path);
		exit(74);
	}
	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize) {
		fprintf(stderr, "Failed to read file '%s'\n", path);
		exit(74);
	}
	buffer[bytesRead] = '\0';
	
	fclose(file);
	return buffer;
}

static bool reachEnd() {
    return *scanner->current == '\0';
}

static char advance() {
    scanner->current++;
    scanner->column++;
    return scanner->current[-1];
}

static void skipChar() {
    scanner->startColumn++;
    scanner->start++;
}

static bool match(char expected) {
    if (reachEnd())
        return false;
    if (*scanner->current != expected)
        return false;
    
    scanner->current++;
    scanner->column++;
    return true;
}

static char peek(int distance) {
    for (int i = 0; i < distance; i++)
        if (scanner->current[i] == '\0')
            return '\0';
    return scanner->current[distance];
}

static bool isAlpha(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           c == '_';
}

static bool isDigit(char c) {
    return '0' <= c && c <= '9';
}

static void pushIndent(int level) {
    scanner->indentationStack[scanner->indentationPointer++] = level;
}

static int popIndent() {
    return scanner->indentationStack[--scanner->indentationPointer];
}

static int peekIndent() {
    return scanner->indentationStack[scanner->indentationPointer - 1];
}

static void skipWhitespace(bool skip) {
    while (true) {
        char c = peek(0);

        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance();
                break;
            case '#':
                while (peek(0) != '\n')
                    advance();
                break;
            case '\\':
                advance();
                advance();
                scanner->line++;
                scanner->column = 1;  
                break;
            case '\n':
                if (!skip)
                    return;
                advance();
                break;
            default:
                return;
        }
    }
}

static Token createToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->startLine;
    token.column = scanner->startColumn;
    return token;
}

static Token createErrorToken(const char *message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner->line;
    token.column = scanner->column;
    return token;
}

static Token scanNumber() {
    while (isDigit(peek(0)))
        advance();

    if (peek(0) == '.') {
        if (isDigit(peek(1))) {
            advance();
            while (isDigit(peek(0)))
                advance();
        } else {
            return createErrorToken("Missing fractional part after '.");
        }
    }

    return createToken(TOKEN_NUMBER);
}

static Token scanIdentifier() {
    while (isAlpha(peek(0)) || isDigit(peek(0)))
        advance();

    int length = scanner->current - scanner->start;
    const struct Keyword *keyword = in_keyword_set(scanner->start, length);

    if (keyword)
        return createToken(keyword->type);
    return createToken(TOKEN_IDENTIFIER);
}

static Token scanString(char stop) {
    bool multi = false;
    skipChar();
    if (peek(0) == stop && peek(1) == stop)  {
        multi = true;
        skipChar();
        skipChar();
    }

    while (!reachEnd()) {
        if (peek(0) == '\n') {
            scanner->line++;
            scanner->column = 1;
            if (!multi)
                break;
        }
        if (multi && peek(0) == stop && peek(1) == stop && peek(2) == stop)
            break;
        if (!multi && peek(0) == stop)
            break;
        if (peek(0) == '\\' && peek(1) == stop)
            advance();
        advance();
    }

    Token string = createToken(TOKEN_STRING);
    
    if (multi && (!match(stop) || !match(stop) || !match(stop)))
        return createErrorToken("Unterminated string");
    if (!multi && !match(stop))
        return createErrorToken("Unterminated string");
    
    return string; 
}

static Token scanRawString(char stop) {
    skipChar();
    skipChar();

    while (peek(0) != stop && peek(0) != '\n' && !reachEnd()) {
        if (peek(0) == '\\' && peek(1) == stop)
            advance();
        advance();
    }

    Token string = createToken(TOKEN_RSTRING);
    
    if (!match(stop))
        return createErrorToken("Unterminated string");
    
    return string; 
}

static Token scanFormattedString(char stop) {
    skipChar();

    while (peek(0) != stop && peek(0) != '\n' && !reachEnd()) {
        if (peek(0) == '\\' && peek(1) == stop)
            advance();
        if (peek(0) == '{' && peek(1) != '{') {
            scanner->inFormattedString = true;
            scanner->stop = stop;
            Token token = createToken(TOKEN_FSTRING);
            advance();
            return token;
        }
        advance();
    }

    Token string = createToken(TOKEN_STRING);
    
    if (!match(stop))
        return createErrorToken("Unterminated string");
    
    scanner->inFormattedString = false;
    return string; 
}

static bool isQuote(char c) {
    return c == '\'' || c == '"';
}

void initScanner(Scanner *s, const char *source) {
    s->source = source;
    s->start = source;
    s->current = source;
    s->line = 1;
    s->column = 1;
    s->startLine = 1;
    s->startColumn = 1;
    s->enclosing = scanner;
    scanner = s;
    pushIndent(0);
}

void endScanner() {
    scanner = scanner->enclosing;
}

Token scanToken(bool skip) {
    if (!skip) {
        if (scanner->column == 1) {
            int indent = 0;
            while (match(' '))
                indent++;
            if (peek(0) != '\n' && peek(0) != '#')
                scanner->indent = indent;
        }

        if (scanner->indent > peekIndent()) {
            pushIndent(scanner->indent);
            return createToken(TOKEN_INDENT);
        }

        if (scanner->indent < peekIndent()) {
            popIndent();
            return createToken(TOKEN_DEDENT);
        }
    }

    skipWhitespace(skip);

    scanner->start = scanner->current;
    scanner->startLine = scanner->line;
    scanner->startColumn = scanner->column;

    if (reachEnd())
        return createToken(TOKEN_EOF);

    char c = advance();

    if (scanner->inFormattedString && c == '}')
        return scanFormattedString(scanner->stop);

    if (isDigit(c))
        return scanNumber();

    if (isQuote(c)) 
        return scanString(c);
    
    if ((c == 'r' || c == 'R') && isQuote(peek(0)))
        return scanRawString(advance());
    
    if ((c == 'f' || c == 'F') && isQuote(peek(0))) {
        skipChar();
        return scanFormattedString(advance());
    }
    
    if (isAlpha(c))
        return scanIdentifier();

    switch (c) {
        case '(':
            return createToken(TOKEN_LEFT_PAREN);
        case ')':
            return createToken(TOKEN_RIGHT_PAREN);
        case '[':
            return createToken(TOKEN_LEFT_BRACKET);
        case ']':
            return createToken(TOKEN_RIGHT_BRACKET);
        case '{':
            return createToken(TOKEN_LEFT_BRACE);
        case '}':
            return createToken(TOKEN_RIGHT_BRACE);
        case '+':
            if (match('='))
                return createToken(TOKEN_PLUS_EQUAL);
            return createToken(TOKEN_PLUS);
        case '-':
            if (match('='))
                return createToken(TOKEN_MINUS_EQUAL);
            return createToken(TOKEN_MINUS);
        case '*':
            if (match('='))
                return createToken(TOKEN_STAR_EQUAL);
            if (match('*')) {
                if (match('='))
                    return createToken(TOKEN_DOUBLE_STAR_EQUAL);
                return createToken(TOKEN_DOUBLE_STAR);
            }
            return createToken(TOKEN_STAR);
        case '/':
            if (match('='))
                return createToken(TOKEN_SLASH_EQUAL);
            if (match('/')) {
                if (match('='))
                    return createToken(TOKEN_DOUBLE_SLASH_EQUAL);
                return createToken(TOKEN_DOUBLE_SLASH);
            }
            return createToken(TOKEN_SLASH);
        case '%':
            if (match('='))
                return createToken(TOKEN_PERCENT_EQUAL);
            return createToken(TOKEN_PERCENT);
        case '@':
            if (match('='))
                return createToken(TOKEN_AT_EQUAL);
            return createToken(TOKEN_AT);
        case '&':
            if (match('='))
                return createToken(TOKEN_AMPERSAND_EQUAL);
            return createToken(TOKEN_AMPERSAND);
        case '|':
            if (match('='))
                return createToken(TOKEN_PIPE_EQUAL);
            return createToken(TOKEN_PIPE);
        case '^':
            if (match('='))
                return createToken(TOKEN_CARET_EQUAL);
            return createToken(TOKEN_CARET);
        case '~':
            return createToken(TOKEN_TILDE);
        case '=':
            if (match('='))
                return createToken(TOKEN_DOUBLE_EQUAL);
            return createToken(TOKEN_EQUAL);
        case '!':
            if (match('='))
                return createToken(TOKEN_BANG_EQUAL);
            break;
        case '<':
            if (match('='))
                return createToken(TOKEN_LESS_EQUAL);
            if (match('<')) {
                if (match('='))
                    return createToken(TOKEN_LEFT_SHIFT_EQUAL);
                return createToken(TOKEN_LEFT_SHIFT);
            }
            return createToken(TOKEN_LESS);
        case '>':
            if (match('='))
                return createToken(TOKEN_GREATER_EQUAL);
            if (match('>')) {
                if (match('='))
                    return createToken(TOKEN_RIGHT_SHIFT_EQUAL);
                return createToken(TOKEN_RIGHT_SHIFT);
            }
            return createToken(TOKEN_GREATER);
        case ':':
            if (match('='))
                return createToken(TOKEN_COLON_EQUAL);
            return createToken(TOKEN_COLON);
        case ';':
            return createToken(TOKEN_SEMICOLON);
        case '.':
            return createToken(TOKEN_DOT);
        case ',':
            return createToken(TOKEN_COMMA);
        case '\n': {
            Token token = createToken(TOKEN_NEWLINE);
            scanner->line++;
            scanner->column = 1;
            return token;
        }
    }

    return createErrorToken("Invalid syntax");
}