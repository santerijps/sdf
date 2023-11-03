#ifndef TOKENIZER_H
#define TOKENIZER_H

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef _INC_STDLIB
#include <stdlib.h>
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

enum TokenType {
  TT_UNDEFINED,   // Undefined/unconfigured tokens
  TT_TEXT,        // Alphanumeric text, underscores
  TT_STRING,      // Text inside double quotes
  TT_NUMBER,      // Ints, floats, underscores
  TT_WHITESPACE,  // Spaces and tabs, consumed
  TT_NEWLINE,     // Newlines, consumed
  TT_EQUALS,      // =
  TT_SEMICOLON,   // ;
  TT_LBRACE,      // {
  TT_RBRACE,      // }
  TT_LBRACK,      // [
  TT_RBRACK,      // ]
  TT_LPAREN,      // (
  TT_RPAREN,      // )
  TT_OTHER,       // Symbols, punctuation, etc.
};

char* TokenTypeToString(enum TokenType t);

struct Token {
  enum TokenType type;
  char* value;
  int ln, col;
};

char* TokenToString(struct Token t);
struct Token CreateTokenFromString(enum TokenType type, char *s);

struct TokenList {
  struct Token *items;
  size_t capacity, length;
};

// Use TokenIterator for better performance
struct TokenList Tokenize(FILE* f);
struct TokenList CreateTokenList(void);
void TokenListAdd(struct TokenList *l, struct Token t);

struct TokenIterator {
  FILE *f;
  size_t ln, col;
};

struct TokenIterator CreateTokenIterator(FILE *f);
int GetNextToken(struct TokenIterator *ti, struct Token *t);
void UngetToken(struct TokenIterator *ti, struct Token *t);

char* ReadTextToken(FILE *f);
char* ReadNumberToken(FILE *f);
char* ReadStringToken(FILE *f);
char* ReadWhiteSpaceToken(FILE *f);
char* ReadNewLineToken(FILE *f);

#endif
