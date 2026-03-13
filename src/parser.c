#include "parser.h"
#include <string.h>
#include <stdbool.h>

void parser_init(Parser* self, char* input) {
  self->input_string = input;
  self->token_count = 0;

  for (int i = 0; i < MAX_TOKENS; i += 1) {
    self->tokens[i] = NULL;
  }
}

int parser_tokenize(Parser *self) {
  char* start = self->input_string;
  char* curr = start;
  bool in_quotes = false;

  while (*curr != '\0') {
    if (*curr == '"') in_quotes = !in_quotes;
    
    if ((*curr == ' ' || *curr == '\n') && !in_quotes) {
      if (curr != start) {
        self->tokens[self->token_count] = start;
        self->token_count += 1;
        *curr = '\0';
      }
      curr += 1;
      start = curr;
    } else {
      curr += 1;
    }
  }

  return 0;
}
