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
  TOK_SEMI,
  TOK_AMPERSAND,
  TOK_GREATER,
  TOK_GREATER2,
  TOK_BANG_GREATER,
  TOK_BANG_GREATER2,
  TOK_AMPERSAND_GREATER,
  TOK_AMPERSAND_GREATER2,
  TOK_PIPE,
  TOK_AMPERSAND_PIPE,
  TOK_AND,
  TOK_OR,
  TOK_LESS,
  TOK_LESS2,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_EOF,
  TOK_ERROR,
} TokenTag;

typedef struct Token {
  TokenTag tag;
  Location loc;  
} Token;

void tokenizer_init(Tokenizer* tokenizer, char* source, size_t length, Arena* arena);
Token* tokenizer_next(Tokenizer* tokenizer);
char tokenizer_peek(Tokenizer* tokenizer);
void token_print(Tokenizer* tokenizer, Token* token);
bool is_eof(Tokenizer* tokenizer);

#endif
