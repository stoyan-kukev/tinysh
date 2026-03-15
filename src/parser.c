#include "parser.h"
#include "arena.h"
#include "tokenizer.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void parser_init(Parser* self, char* source, size_t length, Arena* arena) {
  self->arena = arena;
  Tokenizer tokenizer;
  tokenizer_init(&tokenizer, source, length, arena);
  self->tokenizer = tokenizer;
  self->curr = tokenizer_next(&self->tokenizer);
  self->peek = tokenizer_next(&self->tokenizer);
}

void parser_build_ast(Parser* self) {
  while (self->curr != NULL && self->curr->tag != TOK_EOF) {
    token_print(&self->tokenizer, self->curr);
    parser_advance(self);
  }
  printf("\n");
}

void parser_advance(Parser* self) {
  self->curr = self->peek;
  self->peek = tokenizer_next(&self->tokenizer);
}
