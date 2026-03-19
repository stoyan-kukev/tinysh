#include "arena.h"
#include "ast.h"
#include "executor.h"
#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  (void)argv;

  if (argc > 1) {
    printf("Usage: tinysh\n");
    exit(0);
  }

  char path[255] = {0};
  getcwd(path, 255);

  char *input = NULL;
  size_t n = 0;

  Arena arena;
  arena_init(&arena, 0);

  while (true) {
    printf("tinysh:%s$ ", path);
    ssize_t read_chars = getline(&input, &n, stdin);
    if (read_chars == -1) {
      printf("\nexit\n");
      break;
    }

    Parser parser;
    parser_init(&parser, input, read_chars, &arena);

    if (setjmp(parser.error_env) == 0) {
      AstNode *ast = parser_parse(&parser);
      Executor executor;
      executor_init(&executor, path);
      executor_run(ast);
    }

    arena_reset(&arena);
  }

  free(input);
  arena_deinit(&arena);

  return 0;
}
