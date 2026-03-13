#include "parser.h"
#include "executor.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** _) {
  if (argc > 1) {
    printf("Usage: tinysh\n");
    exit(0);
  }

  char path[255] = {0};
  getcwd(path, 255);

  char* input = NULL;
  size_t n = 0;

  while (true) {
    printf("tinysh:%s$ ", path);
    ssize_t read_chars = getline(&input, &n, stdin);
    if (read_chars == -1) {
        printf("\nexit\n");
        break;
    }

    Parser parser;
    parser_init(&parser, input);
    parser_tokenize(&parser);

    Executor executor;
    executor_init(&executor, parser.tokens, path);
    executor_run(&executor);
  }
  
  free(input);

  return 0;
}
