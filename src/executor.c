#include "executor.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

void executor_init(Executor* self, char** tokens, char* path) {
  self->tokens = tokens;
  self->path = path;
}

int executor_run(Executor* self) {
  if (self->tokens[0] == NULL) return 0;
  
  if (strcmp(self->tokens[0], "cd") == 0) {
    if (self->tokens[2] != NULL) {
      printf("cd: Too many arguments\n");
      return 0;
    }

    if (self->tokens[1] == NULL) {
      const char* home_dir = getenv("HOME");
      if (home_dir == NULL) return 0;

      int status = chdir(home_dir);
      if (status != 0) {
        printf("cd: Failed to change current working directory\n%s\n", strerror(errno));
        return 0;
      }

      strncpy(self->path, home_dir, 255);

      return 0;
    }

    int status = chdir(self->tokens[1]);
    if (status != 0) {
      printf("cd: Failed to change current working directory\n%s\n", strerror(errno));
      return 0;
    }

    char new_path[255];
    getcwd(new_path, 255);
    strncpy(self->path, new_path, 255);

    return 0;
  }

  if (strcmp(self->tokens[0], "exit") == 0) {
    if (self->tokens[1] != NULL) {
      printf("exit: Too many arguments\n");
      return 0;
    }

    exit(0);
  }

  char redirect_file_name[255] = {0};
  bool is_overwriting = false;

  int i = 0;
  while (self->tokens[i] != NULL) {
    const bool has_overwrite = strcmp(self->tokens[i], ">") == 0;
    const bool has_append = strcmp(self->tokens[i], ">>") == 0;
    if (has_overwrite || has_append) {
      if (self->tokens[i+1] == NULL) {
        printf("ERROR: Bad piping. Missing file to pipe to.\n");
        return 0;
      }

      strncpy(redirect_file_name, self->tokens[i + 1], 255);

      if (has_overwrite) is_overwriting = true;

      self->tokens[i] = NULL;
    }

    i += 1;
  }
  
  const pid_t id = fork();
  if (id == -1) {
    printf("ERROR: Failed to create new process\n");
    exit(-1);
  }

  if (id == 0) {
    int fd;
    if (redirect_file_name[0] != 0) {
      if (is_overwriting) {
        fd = open(redirect_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0664);
      } else {
        fd = open(redirect_file_name, O_WRONLY | O_CREAT | O_APPEND, 0664);
      }

      if (fd == -1) {
        printf("ERROR: Failed to open file %s\n%s\n", redirect_file_name, strerror(errno));
        exit(-1);
      }

      dup2(fd, STDOUT_FILENO);
    }
    
    int status = execvp(self->tokens[0], &self->tokens[0]);
    if (status != 0) {
      printf("ERROR: Failed to execute command\n%s\n", strerror(errno));
      exit(-1);
    }

    if (redirect_file_name[0] != 0) close(fd);
  } else {
    int ret_code;
    waitpid(id, &ret_code, 0);
  }
  
  return 0;
}
