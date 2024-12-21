#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include "syntax_parser.h" // 引入语法分析模块

void execute_echo(ASTNode *node);
void execute_cd(ASTNode *node);
int execute_builtin_command(ASTNode *node, int input_fd, int output_fd);
char *find_command_in_PATH(const char *command);
int is_executable_path(const char *path);
char *find_executable_in_path(const char *command, const char *path_env);
char *get_command_path(ASTNode *node);
void execute_command(ASTNode *node, char *command_path, int input_fd, int output_fd);
void execute_external_command(ASTNode *node, int input_fd, int output_fd);
void execute_command_node(ASTNode *node, int input_fd, int output_fd);
void execute_redirect_node(ASTNode *node, int input_fd, int output_fd);
void execute_pipe_node(ASTNode *node, int input_fd);
void execute_node(ASTNode *node);

#endif