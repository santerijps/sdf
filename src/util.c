#include "util.h"

inline char* CharToString(char c) {
  char *s = calloc(2, sizeof(char));
  s[0] = c;
  return s;
}

inline int CharIsAlphabetic(char c) {
  switch (c) {
    case 'a' ... 'z':
    case 'A' ... 'Z':
      return 1;
    default:
      return 0;
  }
}

inline int CharIsDigit(char c) {
  switch (c) {
    case '0' ... '9':
      return 1;
    default:
      return 0;
  }
}

inline int CharIsOther(char c) {
  switch (c) {
    case '!' ... '/':
    case ':':
    case '<' ... '@':
    case '\\':
    case '^':
    case '_' ... '`':
    case '|':
    case '~':
      return 1;
    default:
      return 0;
  }
}

inline int CharIsWhiteSpace(char c) {
  switch (c) {
    case ' ':
    case '\t':
      return 1;
    default:
      return 0;
  }
}

inline struct StringBuilder CreateStringBuilder(void) {
  const size_t capacity = 32;
  return (struct StringBuilder) {
    .capacity = capacity,
    .length = 0,
    .string = calloc(capacity, sizeof(char)),
  };
}

inline void StringBuilderAddChar(struct StringBuilder *sb, char c) {
  if (sb->length >= sb->capacity) {
    while (sb->length >= sb->capacity) {
      sb->capacity <<= 1;
    }
    sb->string = realloc(sb->string, sizeof(char) * sb->capacity);
    memset(&(sb->string[sb->length]), '\0', sb->capacity - sb->length);
  }
  sb->string[sb->length] = c;
  sb->length += 1;
}

inline void StringBuilderAddString(struct StringBuilder *sb, char *s) {
  size_t s_length = strlen(s);
  if (s_length == 0) {
    return;
  }
  if (sb->length + s_length >= sb->capacity) {
    while (sb->length + s_length >= sb->capacity) {
      sb->capacity <<= 1;
    }
    sb->string = realloc(sb->string, sizeof(char) * sb->capacity);
    memset(&(sb->string[sb->length]), '\0', sb->capacity - sb->length);
  }
  strcat(sb->string, s);
  sb->length += s_length;
}

inline void StringBuilderAddSubString(struct StringBuilder *sb, char *s, int start, int stop) {
  size_t s_length = strlen(s);
  int sub_length = stop - start;
  if (s_length == 0 || sub_length == 0) {
    return;
  }
  if (sb->length + sub_length >= sb->capacity) {
    while (sb->length + sub_length >= sb->capacity) {
      sb->capacity <<= 1;
    }
    sb->string = realloc(sb->string, sizeof(char) * sb->capacity);
  }
  for (int i = start; i < stop; i++) {
    sb->string[sb->length] = s[i];
    sb->length += 1;
  }
}

inline char* StringBuilderTrim(struct StringBuilder *sb) {
  char *s = "";
  if (sb->length > 0) {
    size_t i, j;
    for (i = 0; i < sb->length; i++) {
      char c = sb->string[i];
      if (!CharIsWhiteSpace(c)) {
        break;
      }
    }
    for (j = sb->length - 1; j > i; j--) {
      char c = sb->string[j];
      if (!CharIsWhiteSpace(c)) {
        break;
      }
    }
    s = calloc(j - i + 2, sizeof(char));
    for (size_t si = 0, k = i; k <= j; si++, k++) {
      s[si] = sb->string[k];
    }
  }
  return s;
}

inline void StringBuilderClear(struct StringBuilder *sb) {
  sb->length = 0;
  memset(sb->string, '\0', sb->capacity);
}

inline void StringBuilderRecreate(struct StringBuilder *sb) {
  free(sb->string);
  sb->length = 0;
  sb->capacity = 32;
  sb->string = calloc(sb->capacity, sizeof(char));
}

inline struct StringList CreateStringList(void) {
  const size_t capacity = 32;
  return (struct StringList) {
    .capacity = capacity,
    .length = 0,
    .items = malloc(sizeof(char*) * capacity),
  };
}

inline struct StringList* NewStringList(void) {
  const size_t capacity = 32;
  struct StringList *sl = malloc(sizeof(struct StringList));
  sl->capacity = capacity;
  sl->length = 0;
  sl->items = malloc(sizeof(char*) * capacity);
  return sl;
}

inline void StringListAdd(struct StringList *sl, char *s) {
  if (sl->length >= sl->capacity) {
    sl->capacity <<= 1;
    sl->items = realloc(sl->items, sizeof(char*) * sl->capacity);
  }
  sl->items[sl->length] = s;
  sl->length += 1;
}

inline char* StringListToString(struct StringList *sl) {
  size_t size = 32;
  char *buffer = calloc(size, sizeof(char*));
  strcat(buffer, "[");
  for (size_t i = 0; i < sl->length; i++) {
    while (strlen(buffer) + strlen(sl->items[i]) >= size) {
      size <<= 1;
    }
    buffer = realloc(buffer, sizeof(char*) * size);
    memset(&(buffer[strlen(buffer)]), '\0', size - strlen(buffer));
    strcat(buffer, sl->items[i]);
    if (i < sl->length - 1) {
      strcat(buffer, ", ");
    }
  }
  // TODO: Unsafe, check for buffer overflow
  strcat(buffer, "]");
  return buffer;
}

inline int StringIsNumber(char *s) {
  int is_number = 1;
  int has_dot = 0;
  size_t length = strlen(s);
  if (length == 0) {
    is_number = 0;
  }
  else {
    for (size_t i = 0; i < length; i++) {
      char c = s[i];
      if (c == '.') {
        if (has_dot) {
          is_number = 0;
          break;
        }
        else {
          has_dot = 1;
        }
      }
      else if (!CharIsDigit(c)) {
        is_number = 0;
        break;
      }
    }
  }
  return is_number;
}
