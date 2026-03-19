#ifndef TINYSH_AST_H
#define TINYSH_AST_H

#include "tokenizer.h"

#include <stddef.h>

typedef struct AstNode AstNode;

typedef struct RedirectionNode {
  TokenTag operation;
  char *target;
  struct RedirectionNode *next;
} RedirectionNode;

typedef struct {
  char **arguments;
  RedirectionNode *redirections;
} CommandPayload;

typedef struct {
  AstNode *left;
  // | or &|
  Token *operator;
  AstNode *right;
} PipelinePayload;

typedef struct {
  AstNode *left;
  // && or ||
  Token *operator;
  AstNode *right;
} LogicalPayload;

typedef struct {
  AstNode *left;
  // ; or &
  Token *operator;
  AstNode *right;
} ListPayload;

typedef enum AstNodeType {
  NODE_COMMAND,
  NODE_PIPELINE,
  NODE_LOGICAL,
  NODE_LIST,
} AstNodeType;

struct AstNode {
  AstNodeType type;
  union {
    CommandPayload command;
    PipelinePayload pipeline;
    LogicalPayload logical;
    ListPayload list;
  } as;
};

#endif
