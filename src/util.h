#ifndef UTIL_H
#define UTIL_H

#ifndef _INC_STDLIB
#include <stdlib.h>
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

#ifndef _INC_ERRNO
#include <errno.h>
#endif

#define DebugLog(FormatString, ...)\
  fprintf(stderr, "DEBUG [%s:%d %s] " FormatString "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define ErrorLog(FormatString, ...)\
  fprintf(stderr, "ERROR [%s:%d %s] " FormatString "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define FatalLog(FormatString, ...)\
  do {\
    fprintf(stderr, "FATAL [%s:%d %s] " FormatString "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    exit(1);\
  } while (0)

#define StdErrorLog(FormatString, ...)\
  fprintf(stderr, "ERROR [%s:%d %s] %d %s" FormatString "\n", __FILE__, __LINE__, __func__, errno, strerror(errno), ##__VA_ARGS__)

struct StringBuilder {
  char *string;
  size_t capacity, length;
};

struct StringBuilder CreateStringBuilder(void);
void StringBuilderAddChar(struct StringBuilder *sb, char c);
void StringBuilderAddString(struct StringBuilder *sb, char *s);
char* StringBuilderTrim(struct StringBuilder *sb);
void StringBuilderClear(struct StringBuilder *sb);
void StringBuilderRecreate(struct StringBuilder *sb);

struct StringList {
  char **items;
  size_t capacity, length;
};

struct StringList CreateStringList(void);
struct StringList* NewStringList(void);
void StringListAdd(struct StringList *sl, char *s);
void StringBuilderAddSubString(struct StringBuilder *sb, char *s, int start, int stop);
char* StringListToString(struct StringList *sl);

char* CharToString(char c);
int CharIsAlphabetic(char c);
int CharIsDigit(char c);
int CharIsOther(char c);
int CharIsWhiteSpace(char c);

int StringIsNumber(char *s);

#endif