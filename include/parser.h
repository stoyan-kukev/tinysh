#ifndef TINYSH_PARSER_H
#define TINYSH_PARSER_H

#define MAX_TOKENS 64

typedef struct Parser {
  char* input_string;
  char* tokens[MAX_TOKENS];
  int token_count;
} Parser;

// Initializes parser state
void parser_init(Parser* parser, char* input);

// Performs tokenization. Returns 0 on success, -1 on error
int parser_tokenize(Parser* self);

#endif
