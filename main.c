#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

static void repl() {
  // Max size of line is 1024
  char line[1024];

  for (;;) {
    // Print prompt
    printf("> ");

    // Write STDIN to "line" and check for manual EOF
    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line);
  }
}

static char *readFile(const char *path) {
  // Open file
  FILE *file = fopen(path, "rb");

  // Real error handling done by a real programmer 😎
  if (file == NULL) {
    fprintf(stderr, "Couldn't open file \"%s\"\n", path);
    exit(74);
  }

  // Seek to end of file
  fseek(file, 0L, SEEK_END);
  // Get distance from start of file to get fileSize
  size_t fileSize = ftell(file);
  // Rewind to the start of the file
  rewind(file);

  // Allocate a buffer of chars of size "fileSize + 1"
  char *buffer = (char *)malloc(fileSize + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\"\n", path);
    exit(74);
  }
  // Read file into the buffer and get the number of bytes read
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read the file \"%s\"\n", path);
    exit(74);
  }
  // Add null terminator as last element in buffer
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

static void runFile(const char *path) {
  char *source = readFile(path);
  InterpretResult result = interpret(source);
  free(source);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

int main(int argc, const char *argv[]) {
  // Init
  initVM();

  if (argc == 1)
    repl();
  else if (argc == 2)
    runFile(argv[1]);
  else {
    fprintf(stderr, "Usage: clox <path>\n");
    exit(64);
  }

  // Free
  freeVM();
  return 0;
}