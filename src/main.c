#include "arena.h"
#include "ast.h"
#include "executor.h"
#include "parser.h"

#include <bits/types/sigset_t.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void handler(int sig) {
  (void)sig;
}

int main(int argc, char **argv) {
  (void)argv;

  if (argc > 1) {
    printf("Usage: tinysh\n");
    exit(0);
  }

  char path[255] = {0};

  struct sigaction int_action;
  int_action.sa_handler = handler;
  int_action.sa_flags = 0;
  sigemptyset(&int_action.sa_mask);
  sigaction(SIGINT, &int_action, NULL);

  char *input = NULL;
  size_t n = 0;

  Arena arena;
  arena_init(&arena, 0);

  while (true) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    
    getcwd(path, 255);
    printf("tinysh:%s$ ", path);

    ssize_t read_chars = getline(&input, &n, stdin);
    if (read_chars == -1) {
      if (feof(stdin)) {
        printf("\nexit\n");
        break;
      }

      if (errno == EINTR) {
        clearerr(stdin);
        printf("\n");
        continue;
      }

      perror("getline failed");
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
