#include "tokenizer.h"
#include "util.h"

inline struct TokenList CreateTokenList(void) {
  const size_t capacity = 32;
  return (struct TokenList) {
    .capacity = capacity,
    .length = 0,
    .items = malloc(sizeof(struct Token) * capacity),
  };
}

inline void TokenListAdd(struct TokenList *l, struct Token t) {
  if (l->length >= l->capacity) {
    l->capacity <<= 1;
    l->items = realloc(l->items, sizeof(struct Token) * l->capacity);
  }
  l->items[l->length] = t;
  l->length += 1;
}

inline struct Token CreateTokenFromString(enum TokenType type, char *s) {
  return (struct Token) {
    .type = type,
    .value = s,
  };
}

inline struct TokenIterator CreateTokenIterator(FILE *f) {
  return (struct TokenIterator) {
    .f = f,
    .ln = 1,
    .col = 1,
  };
}

inline int GetNextToken(struct TokenIterator *ti, struct Token *t) {
  char c = fgetc(ti->f);
  if (c == EOF) {
    return 0;
  }
  if (c == '\r') {
    return GetNextToken(ti, t);
  }
  t->ln = ti->ln;
  t->col = ti->col;
  if (c == '\n') {
    ungetc(c, ti->f);
    t->type = TT_NEWLINE;
    t->value = ReadNewLineToken(ti->f);
  }
  else if (CharIsWhiteSpace(c)) {
    ungetc(c, ti->f);
    t->type = TT_WHITESPACE;
    t->value = ReadWhiteSpaceToken(ti->f);
  }
  else if (c == '=') {
    t->type = TT_EQUALS;
    t->value = CharToString(c);
  }
  else if (c == ';') { // TODO: Add | token that works like the semicolon
    t->type = TT_SEMICOLON;
    t->value = CharToString(c);
  }
  else if (c == '{') {
    t->type = TT_LBRACE;
    t->value = CharToString(c);
  }
  else if (c == '}') {
    t->type = TT_RBRACE;
    t->value = CharToString(c);
  }
  else if (c == '[') {
    t->type = TT_LBRACK;
    t->value = CharToString(c);
  }
  else if (c == ']') {
    t->type = TT_RBRACK;
    t->value = CharToString(c);
  }
  else if (c == '(') {
    t->type = TT_LPAREN;
    t->value = CharToString(c);
  }
  else if (c == ')') {
    t->type = TT_RPAREN;
    t->value = CharToString(c);
  }
  else if (CharIsAlphabetic(c) || c == '_') {
    ungetc(c, ti->f);
    t->type = TT_TEXT;
    t->value = ReadTextToken(ti->f);
  }
  else if (c == '"') {
    t->type = TT_STRING;
    t->value = ReadStringToken(ti->f);
  }
  else if (CharIsDigit(c)) {
    ungetc(c, ti->f);
    t->type = TT_NUMBER;
    t->value = ReadNumberToken(ti->f);
  }
  else if (CharIsOther(c)) {
    t->type = TT_OTHER;
    t->value = CharToString(c);
  }
  else {
    t->type = TT_UNDEFINED;
    t->value = CharToString(c);
  }
  if (t->type == TT_NEWLINE) {
    ti->ln += strlen(t->value);
    ti->col = 1;
  }
  else {
    ti->col += strlen(t->value);
  }
  return 1;
}

inline void UngetToken(struct TokenIterator *ti, struct Token *t) {
  size_t length = strlen(t->value);
  // TODO: This doesn't actually work for lines, but maybe it covers most cases?
  if (t->type == TT_NEWLINE) {
    ti->ln -= length;
  }
  else {
    ti->col -= length;
  }
  for (int i = length - 1; i >= 0; i--) {
    ungetc(t->value[i], ti->f);
  }
}

struct TokenList Tokenize(FILE* f) {
  struct TokenList l = CreateTokenList();
  struct TokenIterator ti = CreateTokenIterator(f);
  struct Token t;
  while (GetNextToken(&ti, &t)) {
    TokenListAdd(&l, t);
  }
  return l;
}

char* ReadTextToken(FILE *f) {
  struct StringBuilder sb = CreateStringBuilder();
  while (1) {
    char c = fgetc(f);
    if (CharIsAlphabetic(c) || CharIsDigit(c) || c == '_') {
      StringBuilderAddChar(&sb, c);
    }
    else {
      ungetc(c, f);
      return sb.string;
    }
  }
}

char* ReadNumberToken(FILE *f) {
  struct StringBuilder sb = CreateStringBuilder();
  int dot = 0;
  while (1) {
    char c = fgetc(f);
    if (c == '.') {
      if (dot) {
        goto Return;
      }
      else {
        dot = 1;
        StringBuilderAddChar(&sb, c);
      }
    }
    else if (CharIsDigit(c) || c == '_') {
      StringBuilderAddChar(&sb, c);
    }
    else {
Return:
      ungetc(c, f);
      return sb.string;
    }
  }
}

char* ReadStringToken(FILE *f) {
  struct StringBuilder sb = CreateStringBuilder();
  int escape = 0;
  while (1) {
    char c = fgetc(f);
    if (c == '\\') {
      if (escape) {
        StringBuilderAddChar(&sb, c);
        escape = 0;
      }
      else {
        escape = 1;
      }
    }
    else if (c == '"') {
      if (escape) {
        StringBuilderAddChar(&sb, c);
        escape = 0;
      }
      else {
        return sb.string;
      }
    }
    else {
      if (escape) {
        StringBuilderAddChar(&sb, '\\');
        escape = 0;
      }
      StringBuilderAddChar(&sb, c);
    }
  }
}

char* ReadWhiteSpaceToken(FILE *f) {
  struct StringBuilder sb = CreateStringBuilder();
  while (1) {
    char c = fgetc(f);
    if (CharIsWhiteSpace(c)) {
      StringBuilderAddChar(&sb, c);
    }
    else {
      ungetc(c, f);
      return sb.string;
    }
  }
}

char* ReadNewLineToken(FILE *f) {
  struct StringBuilder sb = CreateStringBuilder();
  while (1) {
    char c = fgetc(f);
    if (c == '\r') {
      continue;
    }
    else if (c == '\n') {
      StringBuilderAddChar(&sb, c);
    }
    else {
      ungetc(c, f);
      return sb.string;
    }
  }
}

char* TokenToString(struct Token t) {
  const int padding = 32;
  char *buffer = calloc(strlen(t.value) + padding, sizeof(char));
  if (t.type == TT_NEWLINE || t.type == TT_WHITESPACE) {
    sprintf(buffer, "{%s %lld}", TokenTypeToString(t.type), strlen(t.value));
  }
  else {
    sprintf(buffer, "{%s `%s`}", TokenTypeToString(t.type), t.value);
  }
  return buffer;
}

char* TokenTypeToString(enum TokenType t) {
  switch (t) {
  case TT_TEXT:
    return "TT_TEXT";
  case TT_STRING:
    return "TT_STRING";
  case TT_NUMBER:
    return "TT_NUMBER";
  case TT_WHITESPACE:
    return "TT_WHITESPACE";
  case TT_NEWLINE:
    return "TT_NEWLINE";
  case TT_EQUALS:
    return "TT_EQUALS";
  case TT_SEMICOLON:
    return "TT_SEMICOLON";
  case TT_LBRACE:
    return "TT_LBRACE";
  case TT_RBRACE:
    return "TT_RBRACE";
  case TT_LBRACK:
    return "TT_LBRACK";
  case TT_RBRACK:
    return "TT_RBRACK";
  case TT_LPAREN:
    return "TT_LPAREN";
  case TT_RPAREN:
    return "TT_RPAREN";
  case TT_OTHER:
    return "TT_OTHER";
  default:
    return "TT_UNDEFINED";
  }
}
