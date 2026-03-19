#ifndef TINYSH_EXECUTOR_H
#define TINYSH_EXECUTOR_H

#include "ast.h"

typedef struct Executor {
  char* path;
} Executor;

// Initializes the state of the Executor
void executor_init(Executor* self, char* path);

// Runs the given AST. Returns 0 if success, -1 if error
int executor_run(AstNode* ast);

#endif
