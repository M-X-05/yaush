// free_ast.h
#ifndef FREE_AST_H
#define FREE_AST_H

#include "syntax_parser.h"

void free_command_node(ASTNode *node);
void free_redirect_node(ASTNode *node);
void free_pipe_node(ASTNode *node);
void free_ast(ASTNode *node);

#endif
