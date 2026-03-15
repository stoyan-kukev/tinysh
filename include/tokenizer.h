#ifndef TINYSH_TOKENIZER_H
#define TINYSH_TOKENIZER_H

#include "arena.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct Tokenizer {
  char* source;
  size_t index;
  size_t length;
  Arena* arena;
} Tokenizer;

typedef struct Location {
  size_t start;
  size_t end;
} Location;

typedef enum TokenTag {
  TOK_WORD,
  TOK_STRING,

  TOK_PIPE,
  TOK_AND,
  TOK_OR,
  TOK_GREATER,
  TOK_GREATER2,
  TOK_LESS,
  TOK_LESS2,
  TOK_SEMI,
  TOK_AMPERSAND,

  TOK_LPAREN,
  TOK_RPAREN,

  TOK_EOF,
} TokenTag;

typedef struct Token {
  TokenTag tag;
  Location loc;  
} Token;

void tokenizer_init(Tokenizer* tokenizer, char* source, size_t length, Arena* arena);
Token* tokenizer_next(Tokenizer* tokenizer);
void token_print(Tokenizer* tokenizer, Token* token);
bool is_eof(Tokenizer* tokenizer);

#endif
