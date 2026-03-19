#include "tokenizer.h"

#include "arena.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void tokenizer_init(Tokenizer *tokenizer, char *source, size_t length,
                    Arena *arena) {
  tokenizer->source = source;
  tokenizer->index = 0;
  tokenizer->length = length;
  tokenizer->arena = arena;
}

Token *tokenizer_next(Tokenizer *tokenizer) {
  Token *token = arena_alloc(tokenizer->arena, sizeof(Token));

  while (!is_eof(tokenizer)) {
    token->loc.start = tokenizer->index;

    const char c = tokenizer->source[tokenizer->index];

    if (c == ' ' || c == '\t' || c == '\n') {
      tokenizer->index++;
      continue;
    }

    if (c == ';') {
      token->tag = TOK_SEMI;
      tokenizer->index += 1;
      token->loc.end = tokenizer->index;
      return token;
    }

    if (c == '|') {
      if (tokenizer_peek(tokenizer) == '|') {
        token->tag = TOK_OR;
        tokenizer->index += 2;
      } else {
        token->tag = TOK_PIPE;
        tokenizer->index += 1;
      }
      token->loc.end = tokenizer->index;
      return token;
    }

    if (c == '&') {
      if (tokenizer_peek(tokenizer) == '|') {
        token->tag = TOK_AMPERSAND_PIPE;
        tokenizer->index += 2;
      } else if (tokenizer_peek(tokenizer) == '>') {
        if (tokenizer->index + 2 < tokenizer->length &&
            tokenizer->source[tokenizer->index + 2] == '>') {
          token->tag = TOK_AMPERSAND_GREATER2;
          tokenizer->index += 3;
        } else {
          token->tag = TOK_AMPERSAND_GREATER;
          tokenizer->index += 2;
        }
      } else if (tokenizer_peek(tokenizer) == '&') {
        token->tag = TOK_AND;
        tokenizer->index += 2;
      } else {
        token->tag = TOK_AMPERSAND;
        tokenizer->index += 1;
      }

      token->loc.end = tokenizer->index;
      return token;
    }

    if (c == '!') {
      if (tokenizer_peek(tokenizer) == '>') {
        if (tokenizer->index + 2 < tokenizer->length &&
            tokenizer->source[tokenizer->index + 2] == '>') {
          token->tag = TOK_BANG_GREATER2;
          tokenizer->index += 3;
        } else {
          token->tag = TOK_BANG_GREATER;
          tokenizer->index += 2;
        }
      } else {
        fprintf(stderr, "ERROR: Invalid usage of bang redirect.\n");
        return NULL;
      }

      token->loc.end = tokenizer->index;
      return token;
    }

    if (c == '>') {
      if (tokenizer_peek(tokenizer) == '>') {
        token->tag = TOK_GREATER2;
        tokenizer->index += 2;
      } else {
        token->tag = TOK_GREATER;
        tokenizer->index += 1;
      }

      token->loc.end = tokenizer->index;
      return token;
    }

    if (c == '<') {
      if (tokenizer_peek(tokenizer) == '<') {
        token->tag = TOK_LESS2;
        tokenizer->index += 2;
      } else {
        token->tag = TOK_LESS;
        tokenizer->index += 1;
      }

      token->loc.end = tokenizer->index;
      return token;
    }

    if (c == '"') {
      token->tag = TOK_STRING;

      // Skip opening quote
      tokenizer->index += 1;

      while (!is_eof(tokenizer)) {
        const char c = tokenizer->source[tokenizer->index];
        if (c == '"')
          break;
        tokenizer->index += 1;
      }

      if (is_eof(tokenizer)) {
        fprintf(stderr, "ERROR: Unterminated quotes in input\n");
        return NULL;
      }

      // Skip ending quote
      tokenizer->index += 1;

      token->loc.end = tokenizer->index;
      return token;
    }

    token->tag = TOK_WORD;
    while (!is_eof(tokenizer)) {
      char curr = tokenizer->source[tokenizer->index];
      if (curr == '|' || curr == '>' || curr == ' ' || curr == '!' ||
          curr == '"' || curr == ';' || curr == '&' || curr == '\n' ||
          curr == '\t') {
        break;
      }
      tokenizer->index += 1;
    }

    token->loc.end = tokenizer->index;
    return token;
  }

  token->loc.start = tokenizer->index;
  token->loc.end = tokenizer->index;
  token->tag = TOK_EOF;
  return token;
}

static const char *token_tag_names[] = {
    "TOK_WORD",
    "TOK_STRING",
    "TOK_SEMI",
    "TOK_AMPERSAND",
    "TOK_GREATER",
    "TOK_GREATER2",
    "TOK_BANG_GREATER",
    "TOK_BANG_GREATER2",
    "TOK_AMPERSAND_GREATER",
    "TOK_AMPERSAND_GREATER2",
    "TOK_PIPE",
    "TOK_AMPERSAND_PIPE",
    "TOK_AND",
    "TOK_OR",
    "TOK_LESS",
    "TOK_LESS2",
    "TOK_LPAREN",
    "TOK_RPAREN",
    "TOK_EOF",
};

void token_print(Tokenizer *tokenizer, Token *token) {
  const size_t tags_len = sizeof(token_tag_names) / sizeof(token_tag_names[0]);
  if (token->tag < tags_len) {
    printf("[%s:", token_tag_names[token->tag]);
  } else {
    printf("Unknown token value: %d", token->tag);
  }

  const int len = token->loc.end - token->loc.start;
  printf("%.*s] ", len, tokenizer->source + token->loc.start);
}

bool is_eof(Tokenizer *tokenizer) {
  return tokenizer->index >= tokenizer->length;
}

char tokenizer_peek(Tokenizer *tokenizer) {
  if (tokenizer->index + 1 < tokenizer->length) {
    return tokenizer->source[tokenizer->index + 1];
  }

  return '\0';
}
