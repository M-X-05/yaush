#include <stdio.h>
#include <stdlib.h>
#include "syntax_parser.h" // 引入语法解析模块

// 递归释放命令节点
void free_command_node(ASTNode *node)
{
    if (node == NULL)
        return;

    // 释放命令节点本身
    free(node);
}

// 释放重定向节点
void free_redirect_node(ASTNode *node)
{
    if (node == NULL)
        return;

    // 递归释放左侧命令节点
    free_command_node(node->redirect.left);

    // 释放重定向节点本身
    free(node);
}

// 释放管道节点
void free_pipe_node(ASTNode *node)
{
    if (node == NULL)
        return;

    // 递归释放左侧和右侧的子节点
    free_ast(node->pipe.left);
    free_ast(node->pipe.right);

    // 释放管道节点本身
    free(node);
}

// 释放 AST
void free_ast(ASTNode *node)
{
    if (node == NULL)
        return;

    // 根据节点类型进行释放
    switch (node->type)
    {
    case COMMAND_NODE:
        free_command_node(node);
        break;
    case REDIRECT_NODE:
        free_redirect_node(node);
        break;
    case PIPE_NODE:
        free_pipe_node(node);
        break;
    default:
        break;
    }
}
