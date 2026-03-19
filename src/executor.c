#include "executor.h"
#include "ast.h"
#include "tokenizer.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int execute_ast(Executor *self, AstNode *ast);

void executor_init(Executor *self, Arena *arena, int last_status) {
  self->last_status = last_status;
  self->arena = arena;
}

int executor_run(Executor *self, AstNode *ast, int last_status) {
  self->last_status = last_status;

  return execute_ast(self, ast);
}

static int get_flags(bool condition) {
  if (condition) {
    return O_WRONLY | O_CREAT | O_TRUNC;
  } else {
    return O_WRONLY | O_CREAT | O_APPEND;
  }
}

static void apply_redirections(RedirectionNode *head) {
  RedirectionNode *curr = head;

  while (curr != NULL) {
    int fd;

    switch (curr->operation) {
    case TOK_LESS:
      fd = open(curr->target, O_RDONLY);
      if (fd < 0) {
        perror("open");
        exit(1);
      }
      dup2(fd, STDIN_FILENO);
      close(fd);
      break;
    case TOK_GREATER:
    case TOK_GREATER2:
      fd = open(curr->target, get_flags(curr->operation == TOK_GREATER), 0664);
      if (fd < 0) {
        perror("open");
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
      break;
    case TOK_BANG_GREATER:
    case TOK_BANG_GREATER2:
      fd = open(curr->target, get_flags(curr->operation == TOK_BANG_GREATER),
                0664);
      if (fd < 0) {
        perror("open");
        exit(1);
      }
      dup2(fd, STDERR_FILENO);
      close(fd);
      break;
    case TOK_AMPERSAND_GREATER:
    case TOK_AMPERSAND_GREATER2:
      fd = open(curr->target,
                get_flags(curr->operation == TOK_AMPERSAND_GREATER), 0644);
      if (fd < 0) {
        perror("open");
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
      close(fd);
      break;
    default:
      fprintf(stderr, "Executor: Unhandled redirection type\n");
      exit(1);
    }

    curr = curr->next;
  }
}

static void run_command(Executor *self, CommandPayload *cmd) {
  apply_redirections(cmd->redirections);

  if (cmd->arguments[0] == NULL) {
    exit(0);
  }

  for (size_t i = 0; cmd->arguments[i] != NULL; i += 1) {
    if (strcmp(cmd->arguments[i], "$?") == 0) {
      char *buffer = arena_calloc(self->arena, 128, sizeof(char));
      snprintf(buffer, 128, "%d", self->last_status);
      cmd->arguments[i] = buffer;
    }
  }

  signal(SIGINT, SIG_DFL);
  execvp(cmd->arguments[0], cmd->arguments);

  fprintf(stderr, "tinysh: command not found: %s\n", cmd->arguments[0]);
  exit(127);
}

int execute_command(Executor *self, CommandPayload *cmd) {
  if (strcmp(cmd->arguments[0], "cd") == 0) {
    const char *dir = cmd->arguments[1];
    const char *home_path = getenv("HOME");

    if (cmd->arguments[1] == NULL) {
      if (home_path == NULL) {
        fprintf(stderr, "tinysh: HOME variable not set\n");
        return 1;
      }

      dir = home_path;
    }

    int status = chdir(dir);
    if (status == -1) {
      perror("Changing directories failed.");
      return 1;
    }

    return 0;
  }

  if (strcmp(cmd->arguments[0], "exit") == 0) {
    exit(0);
  }

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork failed");
    return 1;
  }

  if (pid == 0) {
    run_command(self, cmd);
  }

  int status;
  waitpid(pid, &status, 0);

  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }

  return 1;
}

static int execute_pipeline(Executor *self, PipelinePayload *pipeline) {
  int pipe_fds[2];
  if (pipe(pipe_fds) != 0) {
    fprintf(stderr, "tinysh: failed to create pipe\n");
    return 1;
  }

  pid_t left_pid, right_pid;
  switch (pipeline->operator->tag) {
  case TOK_PIPE:
  case TOK_AMPERSAND_PIPE:
    left_pid = fork();
    if (left_pid < 0) {
      perror("fork failed");
      return 1;
    }

    if (left_pid == 0) {
      close(pipe_fds[0]);
      dup2(pipe_fds[1], STDOUT_FILENO);
      if (pipeline->operator->tag != TOK_PIPE)
        dup2(pipe_fds[1], STDERR_FILENO);
      close(pipe_fds[1]);

      int status = execute_ast(self, pipeline->left);
      exit(status);
    }

    right_pid = fork();
    if (right_pid < 0) {
      perror("fork failed");
      return 1;
    }

    if (right_pid == 0) {
      close(pipe_fds[1]);
      dup2(pipe_fds[0], STDIN_FILENO);
      close(pipe_fds[0]);

      int status = execute_ast(self, pipeline->right);
      exit(status);
    }

    close(pipe_fds[0]);
    close(pipe_fds[1]);

    int status1, status2;
    waitpid(left_pid, &status1, 0);
    waitpid(right_pid, &status2, 0);

    return WIFEXITED(status2) ? WEXITSTATUS(status2) : 1;
  default:
    fprintf(stderr, "tinysh: unknown operator called with pipe\n");
    return 1;
  }
}

int execute_logical(Executor *self, LogicalPayload *logical) {
  int status = execute_ast(self, logical->left);

  switch (logical->operator->tag) {
  case TOK_AND:
    if (status == 0) {
      return execute_ast(self, logical->right);
    }
    return status;
  case TOK_OR:
    if (status != 0) {
      return execute_ast(self, logical->right);
    }
    return 0;
  default:
    fprintf(stderr, "tinysh: unknown logical operator\n");
    return 1;
  }
}

static int execute_list(Executor *self, ListPayload *list) {
  pid_t pid;
  switch (list->operator->tag) {
  case TOK_SEMI:
    execute_ast(self, list->left);
    return execute_ast(self, list->right);
  case TOK_AMPERSAND:
    pid = fork();
    if (pid == -1) {
      perror("fork failed");
      return 1;
    }

    if (pid == 0) {
      execute_ast(self, list->left);
      exit(0);
    } else {
      return execute_ast(self, list->right);
    }
  default:
    fprintf(stderr, "tinysh: unknown list operator\n");
    return 1;
  }

  return 0;
}

static int execute_ast(Executor *self, AstNode *ast) {
  if (!ast)
    return 0;

  switch (ast->type) {
  case NODE_COMMAND:
    return execute_command(self, &ast->as.command);
  case NODE_PIPELINE:
    return execute_pipeline(self, &ast->as.pipeline);
  case NODE_LOGICAL:
    return execute_logical(self, &ast->as.logical);
  case NODE_LIST:
    return execute_list(self, &ast->as.list);
  }

  return 0;
}
