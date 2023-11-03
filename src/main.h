#ifndef MAIN_H
#define MAIN_H

#ifndef UTIL_H
#include "util.h"
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#define BUFFER_SIZE 4096

int StdinIsReadable(void);
void ReadStdinContent(struct StringBuilder *sb);

void FilePathToJSON(const char *file_path);
void FileToJSON(FILE *f);

#endif