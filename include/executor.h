#ifndef TINYSH_EXECUTOR_H
#define TINYSH_EXECUTOR_H

typedef struct Executor {
  char** tokens;
} Executor;

// Initializes the state of the Executor
void executor_init(Executor* self, char** tokens);

// Parses the tokens and runs the requested command. Returns 0 if success, -1 if error
int executor_run(Executor* self);

#endif
