#ifndef TINYSH_PARSER_H
#define TINYSH_PARSER_H

#include "arena.h"
#include "ast.h"
#include "tokenizer.h"

#include <setjmp.h>
#include <stddef.h>

typedef struct Parser {
  Tokenizer *tokenizer;
  Arena *arena;
  Token *curr;
  Token *prev;
  jmp_buf error_env;
} Parser;

// Initializes parser state
void parser_init(Parser *parser, char *source, size_t length, Arena *arena);

// Parses raw input into an AST
AstNode *parser_parse(Parser *self);

#endif
