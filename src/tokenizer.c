#include "tokenizer.h"

#include "arena.h"

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

void tokenizer_init(Tokenizer* tokenizer, char* source, size_t length, Arena* arena) {
  tokenizer->source = source;
  tokenizer->index = 0;
  tokenizer->length = length;
  tokenizer->arena = arena;
}

Token* tokenizer_next(Tokenizer* tokenizer) {
  Token* token = arena_alloc(tokenizer->arena, sizeof(Token));

  while (!is_eof(tokenizer)) {
    token->loc.start = tokenizer->index;

    const char c = tokenizer->source[tokenizer->index];

    if (c == '"') {
      token->tag = TOK_STRING;
      tokenizer->index++;

      while (!is_eof(tokenizer)) {
        const char c = tokenizer->source[tokenizer->index];
        if (c == '"') break;
        tokenizer->index++;
      }

      if (is_eof(tokenizer)) {
        fprintf(stderr, "ERROR: Unterminated quotes in input\n");
        return NULL;
      }

      tokenizer->index++;

      token->loc.end = tokenizer->index;
      return token;
    }

    tokenizer->index++;
  }

  token->loc.start = tokenizer->index;
  token->loc.end = tokenizer->index;
  token->tag = TOK_EOF;
  return token;
}

static const char* token_tag_names[] = {
  "TOK_WORD",
  "TOK_STRING",
  "TOK_PIPE",
  "TOK_AND",
  "TOK_OR",
  "TOK_GREATER",
  "TOK_GREATER2",
  "TOK_LESS",
  "TOK_LESS2",
  "TOK_SEMI",
  "TOK_AMPERSAND",
  "TOK_LPAREN",
  "TOK_RPAREN",
  "TOK_EOF",
};

void token_print(Tokenizer* tokenizer, Token* token) {
  const size_t tags_len = sizeof(token_tag_names)/sizeof(token_tag_names[0]);
  if (token->tag < tags_len) {
    printf("[%s:", token_tag_names[token->tag]);
  } else {
    printf("Unknown token value: %d", token->tag);
  }

  const int len = token->loc.end - token->loc.start;
  printf("%.*s] ", len, tokenizer->source + token->loc.start);
}

bool is_eof(Tokenizer* tokenizer) {
  return tokenizer->index >= tokenizer->length;
}
