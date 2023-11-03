#ifndef PARSER_H
#define PARSER_H

#ifndef _INC_STDLIB
#include <stdlib.h>
#endif

#include "tokenizer.h"
#include "util.h"

#define InvalidTokenError(t)\
  fprintf(\
    stderr,\
    "Error occurred in file %s, line %d:\n"\
    "Unexpected token (Ln: %d, Col: %d): %s\n",\
    __FILE__, __LINE__, t.ln, t.col, TokenToString(t)\
  );\
  exit(1);

#define UndefinedKeyError(t, key)\
  fprintf(\
    stderr,\
    "Error occurred in file %s, line %d:\n"\
    "Undefined key: key not found in schema (Ln: %d, Col: %d): %s\n",\
    __FILE__, __LINE__, t.ln, t.col, key\
  );\
  exit(1);

#define NoMatchingValueError(key)\
  fprintf(\
    stderr,\
    "Error occurred in file %s, line %d:\n"\
    "No matching value found for key: '%s'\n",\
    __FILE__, __LINE__, key\
  );\
  exit(1);

struct SDF_Object {
  struct StringList *keys;
  struct ParserValueList *values;
};

struct SDF_Object CreateSDFObject(void);
void SDFObjectToString(struct SDF_Object *o, struct StringBuilder *sb);

struct SDF_List {
  struct StringList *schema;
  struct ParserValueList *items;
};

struct SDF_List CreateSDFList(void);
void SDFListToString(struct SDF_List *l, struct StringBuilder *sb);

enum ParserValueType {
  PVT_STRING,
  PVT_NUMBER,
  PVT_OBJECT,
  PVT_LIST,
};

union ParserData {
  char *as_string;
  float as_float;
  struct SDF_Object as_object;
  struct SDF_List as_list;
};

struct ParserValue {
  enum ParserValueType type;
  union ParserData data;
};

void ParserValueToString(struct ParserValue *pv, struct StringBuilder *sb);
struct ParserValue CreateParserValueString(char *s);
struct ParserValue CreateParserValueNumber(float f);
struct ParserValue CreateParserValueObject(struct SDF_Object o);
struct ParserValue CreateParserValueList(struct SDF_List l);

struct ParserValueList {
  struct ParserValue *items;
  size_t capacity, length;
};

struct ParserValueList* NewParserValueList(void);
void ParserValueListAdd(struct ParserValueList *pvl, struct ParserValue pv);

void ParseKeyText(struct TokenIterator *ti, struct StringBuilder *sb);
void ParseValueText(struct TokenIterator *ti, struct StringBuilder *sb);
struct SDF_Object ParseObject(struct TokenIterator *ti);
struct SDF_List ParseList(struct TokenIterator *ti, struct StringList *schema);
void ParseSchema(struct TokenIterator *ti, struct StringList *sl);

#endif
