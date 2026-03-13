#include "parser.h"
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

    for (int i = 0; i < parser.token_count; i += 1) {
      printf("[%s]", parser.tokens[i]);
    }

    printf("\n");
  }
  
  free(input);

  return 0;
}
