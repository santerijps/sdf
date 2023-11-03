#include "parser.h"

inline void ParserValueToString(struct ParserValue *pv, struct StringBuilder *sb) {
  switch (pv->type) {
    case PVT_STRING:
      StringBuilderAddChar(sb, '"');
      StringBuilderAddString(sb, pv->data.as_string);
      StringBuilderAddChar(sb, '"');
      break;
    case PVT_NUMBER: {
      float is_int = (float)((int)pv->data.as_float) == pv->data.as_float;
      if (is_int) {
        char buffer[256] = {0};
        itoa(pv->data.as_float, buffer, 10);
        StringBuilderAddString(sb, buffer);
      }
      else {
        char buffer[256] = {};
        sprintf(buffer, "%f", pv->data.as_float);
        StringBuilderAddString(sb, buffer);
      }
      break;
    }
    case PVT_OBJECT:
      SDFObjectToString(&(pv->data.as_object), sb);
      break;
    case PVT_LIST:
      SDFListToString(&(pv->data.as_list), sb);
      break;
    default:
      return;
  }
}

inline struct ParserValue CreateParserValueString(char *s) {
  return (struct ParserValue) {
    .type = PVT_STRING,
    .data.as_string = s,
  };
}

inline struct ParserValue CreateParserValueNumber(float f) {
  return (struct ParserValue) {
    .type = PVT_NUMBER,
    .data.as_float = f,
  };
}

inline struct ParserValue CreateParserValueObject(struct SDF_Object o) {
  return (struct ParserValue) {
    .type = PVT_OBJECT,
    .data.as_object = o,
  };
}

inline struct ParserValue CreateParserValueList(struct SDF_List l) {
  return (struct ParserValue) {
    .type = PVT_LIST,
    .data.as_list = l,
  };
}

inline struct ParserValueList* NewParserValueList(void) {
  const size_t capacity = 32;
  struct ParserValueList *pvl = malloc(sizeof(struct ParserValueList));
  pvl->capacity = capacity;
  pvl->length = 0;
  pvl->items = malloc(sizeof(struct ParserValue) * capacity);
  return pvl;
}

inline void ParserValueListAdd(struct ParserValueList *pvl, struct ParserValue pv) {
  if (pvl->length >= pvl->capacity) {
    pvl->capacity <<= 1;
    pvl->items = realloc(pvl->items, sizeof(struct ParserValue) * pvl->capacity);
  }
  pvl->items[pvl->length] = pv;
  pvl->length += 1;
}

inline struct SDF_Object CreateSDFObject(void) {
  return (struct SDF_Object) {
    .keys = NewStringList(),
    .values = NewParserValueList(),
  };
}

inline void SDFObjectToString(struct SDF_Object *o, struct StringBuilder *sb) {
  StringBuilderAddChar(sb, '{');
  for (size_t i = 0; i < o->keys->length; i++) {
    StringBuilderAddChar(sb, '"');
    StringBuilderAddString(sb, o->keys->items[i]);
    StringBuilderAddString(sb, "\":");
    ParserValueToString(&(o->values->items[i]), sb);
    if (i < o->keys->length - 1) {
      StringBuilderAddChar(sb, ',');
    }
  }
  StringBuilderAddChar(sb, '}');
}

inline struct SDF_List CreateSDFList(void) {
  return (struct SDF_List) {
    .schema = NewStringList(),
    .items = NewParserValueList(),
  };
}

inline void SDFListToString(struct SDF_List *l, struct StringBuilder *sb) {
  StringBuilderAddChar(sb, '[');
  for (size_t i = 0; i < l->items->length; i++) {
    ParserValueToString(&(l->items->items[i]), sb);
    if (i < l->items->length - 1) {
      StringBuilderAddChar(sb, ',');
    }
  }
  StringBuilderAddChar(sb, ']');
}

inline struct SDF_Object ParseObject(struct TokenIterator *ti) {
  struct SDF_Object o = CreateSDFObject();
  struct Token t = {};
  struct StringList schema = CreateStringList();
  struct StringBuilder sb = CreateStringBuilder();

  while (GetNextToken(ti, &t)) switch (t.type) {
    case TT_TEXT:
    case TT_NUMBER:
    case TT_STRING:
    case TT_OTHER:
      StringBuilderAddString(&sb, t.value);
      if (o.keys->length > o.values->length) {
        InvalidTokenError(t);
      }
      ParseKeyText(ti, &sb);
      StringListAdd(o.keys, StringBuilderTrim(&sb));
      StringBuilderClear(&sb);
      break;

    case TT_EQUALS: {
      ParseValueText(ti, &sb);
      char *value = StringBuilderTrim(&sb);
      if (StringIsNumber(value)) {
        ParserValueListAdd(o.values, CreateParserValueNumber(strtof(value, NULL)));
      }
      else {
        ParserValueListAdd(o.values, CreateParserValueString(value));
      }
      StringBuilderClear(&sb);
      break;
    }

    case TT_LBRACE:
      if (o.keys->length == o.values->length) {
        InvalidTokenError(t);
      }
      ParserValueListAdd(o.values, CreateParserValueObject(ParseObject(ti)));
      break;

    case TT_LBRACK:
      if (o.keys->length == o.values->length) {
        InvalidTokenError(t);
      }
      ParserValueListAdd(o.values, CreateParserValueList(ParseList(ti, &schema)));
      break;

    case TT_LPAREN:
      ParseSchema(ti, &schema);
      break;

    case TT_NEWLINE:
    case TT_WHITESPACE:
      continue;

    case TT_RBRACE:
      goto FunctionReturn;

    default:
      InvalidTokenError(t);
  }

FunctionReturn:
  if (o.keys->length > o.values->length) {
   NoMatchingValueError(o.keys->items[o.keys->length - 1]);
  }
  free(sb.string);
  return o;
}

inline struct SDF_List ParseList(struct TokenIterator *ti, struct StringList *schema) {
  struct SDF_List l = CreateSDFList();
  struct StringBuilder sb = CreateStringBuilder();
  struct Token t;
  int ignore_whitespace_and_newlines = 1;
  struct SDF_Object o = {};

  l.schema = schema;
  if (schema->length > 0) {
    o = CreateSDFObject();
  }

  while (GetNextToken(ti, &t)) {
    /*
      Ignore leading newlines and whitespace to avoid empty values
        in the beginning of the list.
    */
    if (ignore_whitespace_and_newlines) {
      if (t.type == TT_NEWLINE || t.type == TT_WHITESPACE) {
        continue;
      }
      else {
        ignore_whitespace_and_newlines = 0;
      }
    }

    switch (t.type) {
      case TT_TEXT:
      case TT_NUMBER:
      case TT_STRING:
      case TT_OTHER:
      case TT_WHITESPACE:
        StringBuilderAddString(&sb, t.value);
        break;

      case TT_NEWLINE:
      case TT_SEMICOLON: {
        char *value = StringBuilderTrim(&sb);
        if (schema->length > 0) {
          StringListAdd(o.keys, schema->items[o.keys->length]);
          if (StringIsNumber(value)) {
            ParserValueListAdd(o.values, CreateParserValueNumber(strtof(value, NULL)));
          }
          else {
            ParserValueListAdd(o.values, CreateParserValueString(value));
          }
          if (o.keys->length == schema->length) {
            ParserValueListAdd(l.items, CreateParserValueObject(o));
            o = CreateSDFObject();
          }
        }
        else {
          if (StringIsNumber(value)) {
            ParserValueListAdd(l.items, CreateParserValueNumber(strtof(value, NULL)));
          }
          else {
            ParserValueListAdd(l.items, CreateParserValueString(value));
          }
        }
        ignore_whitespace_and_newlines = 1;
        StringBuilderClear(&sb);
        break;
      }

      case TT_LBRACE: {
        ParserValueListAdd(l.items, CreateParserValueObject(ParseObject(ti)));
        ignore_whitespace_and_newlines = 1;
        break;
      }

      case TT_LBRACK: {
        ParserValueListAdd(l.items, CreateParserValueList(ParseList(ti, schema)));
        ignore_whitespace_and_newlines = 1;
        break;
      }

      case TT_RBRACK: {
        char *value = StringBuilderTrim(&sb);
        if (schema->length > 0) {
          if (o.keys->length < schema->length && strlen(value) > 0) {
            StringListAdd(o.keys, schema->items[o.keys->length]);
            if (StringIsNumber(value)) {
              ParserValueListAdd(o.values, CreateParserValueNumber(strtof(value, NULL)));
            }
            else {
              ParserValueListAdd(o.values, CreateParserValueString(value));
            }
          }
          if (o.keys->length > 0) {
            ParserValueListAdd(l.items, CreateParserValueObject(o));
          }
        }
        else if (strlen(value) > 0) {
          if (StringIsNumber(value)) {
            ParserValueListAdd(l.items, CreateParserValueNumber(strtof(value, NULL)));
          }
          else {
            ParserValueListAdd(l.items, CreateParserValueString(value));
          }
        }
        goto FunctionReturn;
      }

      default:
        InvalidTokenError(t);
    }
  }

FunctionReturn:
  free(sb.string);
  return l;
}

inline void ParseKeyText(struct TokenIterator *ti, struct StringBuilder *sb) {
  struct Token t;
  while (GetNextToken(ti, &t)) {
    switch (t.type) {
      case TT_TEXT:
      case TT_NUMBER:
      case TT_OTHER:
      case TT_WHITESPACE:
        StringBuilderAddString(sb, t.value);
        break;
      default:
        UngetToken(ti, &t);
        return;
    }
  }
}

inline void ParseValueText(struct TokenIterator *ti, struct StringBuilder *sb) {
  struct Token t;
  while (GetNextToken(ti, &t)) switch (t.type) {
    case TT_LBRACK:
    case TT_RBRACK:
    case TT_LBRACE:
      InvalidTokenError(t);
    case TT_NEWLINE:
    case TT_SEMICOLON:
      return;
    case TT_RBRACE:
      UngetToken(ti, &t);
      return;
    default:
      StringBuilderAddString(sb, t.value);
      break;
  }
}

inline void ParseSchema(struct TokenIterator *ti, struct StringList *schema) {
  struct StringBuilder sb = CreateStringBuilder();
  struct Token t;
  while (GetNextToken(ti, &t)) switch (t.type) {
    case TT_TEXT:
    case TT_NUMBER:
    case TT_OTHER:
    case TT_WHITESPACE:
      StringBuilderAddString(&sb, t.value);
      break;
    case TT_NEWLINE:
    case TT_SEMICOLON: {
      if (sb.length > 0) {
        char *key = StringBuilderTrim(&sb);
        StringListAdd(schema, key);
        StringBuilderClear(&sb);
      }
      else {
        InvalidTokenError(t);
        // TODO: Error = Missing key in schema!
      }
      break;
    }
    case TT_RPAREN: {
      if (sb.length > 0) {
        char *key = StringBuilderTrim(&sb);
        StringListAdd(schema, key);
        StringBuilderClear(&sb);
      }
      goto FunctionReturn;
    }
    default:
      InvalidTokenError(t);
  }
FunctionReturn:
  free(sb.string);
}
