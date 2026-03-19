#ifndef TINYSH_EXECUTOR_H
#define TINYSH_EXECUTOR_H

#include "ast.h"

typedef struct Executor {
  int last_status;
  Arena *arena;
} Executor;

// Initializes the state of the Executor
void executor_init(Executor *self, Arena *arena, int last_status);

// Runs the given AST. Returns 0 if success, 1 if error
int executor_run(Executor *self, AstNode *ast, int last_status);

#endif
