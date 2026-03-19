#include "parser.h"
#include "arena.h"
#include "ast.h"
#include "tokenizer.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} ArgsArray;

static void args_array_init(ArgsArray *array) {
  array->items = NULL;
  array->count = 0;
  array->capacity = 0;
}

static void args_array_push(ArgsArray *array, char *arg, Arena *arena) {
  if (array->count >= array->capacity) {
    size_t new_capacity = array->capacity == 0 ? 8 : array->capacity * 2;

    char **new_items = arena_alloc(arena, new_capacity * sizeof(char *));
    if (array->count > 0) {
      memcpy(new_items, array->items, array->count * sizeof(char *));
    }

    array->items = new_items;
    array->capacity = new_capacity;
  }

  array->items[array->count] = arg;
  array->count++;
}

void parser_init(Parser *self, char *source, size_t length, Arena *arena) {
  self->arena = arena;
  Tokenizer *tokenizer = arena_alloc(arena, sizeof(Tokenizer));
  tokenizer_init(tokenizer, source, length, arena);
  self->tokenizer = tokenizer;
  self->curr = tokenizer_next(tokenizer);
  self->prev = NULL;
}

static void advance(Parser *self) {
  self->prev = self->curr;
  self->curr = tokenizer_next(self->tokenizer);
}

static bool check(Parser *self, TokenTag expected_tag) {
  if (self->curr->tag == TOK_EOF) {
    return false;
  }

  return self->curr->tag == expected_tag;
}

static bool match(Parser *self, TokenTag expected_tag) {
  if (check(self, expected_tag)) {
    advance(self);
    return true;
  }

  return false;
}

static Token *consume(Parser *self, TokenTag expected_tag,
                      const char *err_msg) {
  if (check(self, expected_tag)) {
    advance(self);
    return self->prev;
  }

  fprintf(stderr, "Parsing error near index: %lu: %s\n", self->curr->loc.start,
          err_msg);

  longjmp(self->error_env, 1);
  return NULL;
}

static bool check_is_redirection(Parser *self) {
  return check(self, TOK_GREATER) || check(self, TOK_GREATER2) ||
         check(self, TOK_BANG_GREATER) || check(self, TOK_BANG_GREATER2) ||
         check(self, TOK_AMPERSAND_GREATER) ||
         check(self, TOK_AMPERSAND_GREATER2) || check(self, TOK_LESS) ||
         check(self, TOK_LESS2);
}

static AstNode *parse_command(Parser *self) {
  ArgsArray args;
  args_array_init(&args);

  RedirectionNode *head = NULL;
  RedirectionNode *tail = NULL;

  while (check(self, TOK_WORD) || check(self, TOK_STRING)) {
    Token *token = consume(self, self->curr->tag, "Expected argument");

    const size_t length = token->loc.end - token->loc.start;
    char *arg = arena_alloc(self->arena, length + 1);

    memcpy(arg, self->tokenizer->source + token->loc.start, length);
    arg[length] = '\0';

    args_array_push(&args, arg, self->arena);
  }

  if (args.count == 0 && !check_is_redirection(self)) {
    fprintf(stderr, "Expected a command or redirection.\n");
    longjmp(self->error_env, 1);
  }

  while (check_is_redirection(self)) {
    RedirectionNode *node = arena_alloc(self->arena, sizeof(RedirectionNode));

    node->operation =
        consume(self, self->curr->tag, "Expected redirection operator")->tag;

    Token *target_token;
    if (check(self, TOK_WORD)) {
      target_token = consume(self, TOK_WORD, "Expected file target");
    } else if (check(self, TOK_STRING)) {
      target_token = consume(self, TOK_STRING, "Expected file target");
    } else {
      fprintf(stderr,
              "Syntax Error: Redirection must be followed by a file name.\n");
      longjmp(self->error_env, 1);
    }

    const size_t length = target_token->loc.end - target_token->loc.start;
    char *target_str = arena_alloc(self->arena, length + 1);
    memcpy(target_str, self->tokenizer->source + target_token->loc.start,
           length);
    target_str[length] = '\0';

    node->target = target_str;

    node->next = NULL;

    if (head == NULL) {
      head = node;
      tail = node;
    } else {
      tail->next = node;
      tail = node;
    }
  }

  if (check(self, TOK_WORD) || check(self, TOK_STRING)) {
    fprintf(stderr, "Syntax Error: Arguments must come before redirections.\n");
    longjmp(self->error_env, 1);
  }

  // Make args null-terminated in prep for execvp
  args_array_push(&args, NULL, self->arena);

  AstNode *node = arena_alloc(self->arena, sizeof(AstNode));
  node->type = NODE_COMMAND;
  node->as.command.arguments = args.items;
  node->as.command.redirections = head;

  return node;
}

static AstNode *parse_pipeline(Parser *self) {
  AstNode *left = parse_command(self);

  while (match(self, TOK_PIPE) || match(self, TOK_AMPERSAND_PIPE)) {
    Token *operator = self->prev;

    AstNode *right = parse_command(self);

    AstNode *new_node = arena_alloc(self->arena, sizeof(AstNode));
    new_node->type = NODE_PIPELINE;

    new_node->as.pipeline.left = left;
    new_node->as.pipeline.operator = operator;
    new_node->as.pipeline.right = right;

    left = new_node;
  }

  return left;
}

static AstNode *parse_logical(Parser *self) {
  AstNode *left = parse_pipeline(self);

  while (match(self, TOK_AND) || match(self, TOK_OR)) {
    Token *operator = self->prev;

    AstNode *right = parse_pipeline(self);

    AstNode *new_node = arena_alloc(self->arena, sizeof(AstNode));
    new_node->type = NODE_LOGICAL;

    new_node->as.logical.left = left;
    new_node->as.logical.operator = operator;
    new_node->as.logical.right = right;

    left = new_node;
  }

  return left;
}

static AstNode *parse_list(Parser *self) {
  AstNode *left = parse_logical(self);

  while (match(self, TOK_SEMI) || match(self, TOK_AMPERSAND)) {
    Token *operator = self->prev;

    AstNode *right = parse_logical(self);

    AstNode *new_node = arena_alloc(self->arena, sizeof(AstNode));
    new_node->type = NODE_LIST;

    new_node->as.list.left = left;
    new_node->as.list.operator = operator;
    new_node->as.list.right = right;

    left = new_node;
  }

  return left;
}

AstNode *parser_parse(Parser *self) { return parse_list(self); }
