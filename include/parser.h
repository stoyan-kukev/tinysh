#ifndef TINYSH_PARSER_H
#define TINYSH_PARSER_H

#include "tokenizer.h"

#include "arena.h"

#include <stddef.h>

typedef struct Parser {
  Tokenizer tokenizer;
  Arena* arena;
  Token* curr;
  Token* peek;
} Parser;

// Initializes parser state
void parser_init(Parser* parser, char* source, size_t length, Arena* arena);

// Parses raw input into an AST
void parser_build_ast(Parser* self);

// Advances the parser to the next token
void parser_advance(Parser* self);

#endif
