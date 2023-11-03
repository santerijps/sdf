#include "main.h"
#include "parser.h"
#include "tokenizer.h"

static inline void SubString(char *buffer, char *s, int start, int stop) {
  int length = stop - start;
  sprintf(buffer, "%.*s", length, s + start);
}

int main(int argc, char **argv) {

  for (int i = 1; i < argc; i++) {
    FilePathToJSON(argv[i]);
  }

  if (StdinIsReadable()) {
    FILE *f = fopen("sdf.tmp", "wb+");
    if (f == NULL) {
      StdErrorLog("Failed to create tmp file!");
    }
    struct StringBuilder sb = CreateStringBuilder();
    ReadStdinContent(&sb);
    fwrite(sb.string, sizeof(char), sb.length, f);
    rewind(f);
    FileToJSON(f);
    fclose(f);
    remove("sdf.tmp");
    free(sb.string);
  }

  return 0;
}

inline int StdinIsReadable(void) {
  fseek(stdin, 0, SEEK_END);
  int is_readable = ftell(stdin) > 0;
  rewind(stdin);
  return is_readable;
}

inline void ReadStdinContent(struct StringBuilder *sb) {
  while (1) {
    char buffer[BUFFER_SIZE + 1] = {0};
    size_t n = fread(buffer, sizeof(char), BUFFER_SIZE, stdin);
    StringBuilderAddSubString(sb, buffer, 0, n);
    if (n < BUFFER_SIZE) {
      break;
    }
  }
}

inline void FilePathToJSON(const char *file_path) {
  FILE *f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "Error! Failed to open file: %s\n", file_path);
    return;
  }
  FileToJSON(f);
  fclose(f);
}

inline void FileToJSON(FILE *f) {
  struct TokenIterator ti = CreateTokenIterator(f);
  struct SDF_Object o = ParseObject(&ti);
  struct StringBuilder sb = CreateStringBuilder();
  SDFObjectToString(&o, &sb);
  puts(sb.string);
  StringBuilderClear(&sb);
  free(sb.string);
}
