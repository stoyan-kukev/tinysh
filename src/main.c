#include "parser.h"
#include "executor.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(void) {
  char* input = NULL;
  size_t n = 0;

  while (true) {
    printf("tinysh> ");
    ssize_t read_chars = getline(&input, &n, stdin);
    if (read_chars == -1) {
        printf("\nexit\n");
        break;
    }

    Parser parser;
    parser_init(&parser, input);
    parser_tokenize(&parser);

    Executor executor;
    executor_init(&executor, parser.tokens);
    executor_run(&executor);
  }
  
  free(input);

  return 0;
}
