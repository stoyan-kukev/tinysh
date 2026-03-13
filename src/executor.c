#include "executor.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

void executor_init(Executor* self, char** tokens) {
  self->tokens = tokens;
}

int executor_run(Executor* self) {
  const pid_t id = fork();
  if (id == -1) {
    printf("ERROR: Failed to create new process\n");
    exit(-1);
  }

  if (id == 0) {
    int status = execvp(self->tokens[0], &self->tokens[0]);
    if (status != 0) {
      printf("ERROR: Failed to execute command\n%s\n", strerror(status));
      exit(-1);
    }
  } else {
    int ret_code;
    waitpid(id, &ret_code, 0);
  }
  
  return 0;
}
