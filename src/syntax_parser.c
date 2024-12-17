#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syntax_parser.h"
#include "lexical_parser.h" // 引入词法解析器

// 当前token的索引
static int current_token_index = 0;
static char **tokens;
static int token_count;

// 获取当前token
static char *current_token()
{
    if (current_token_index < token_count)
    {
        return tokens[current_token_index];
    }
    return NULL;
}

// 向前移动到下一个token
static void advance_token()
{
    if (current_token_index < token_count)
    {
        current_token_index++;
    }
}

// 判断当前token是否是特定的符号
static int is_token(const char *token)
{
    return current_token() != NULL && strcmp(current_token(), token) == 0;
}

// 解析命令
ASTNode *parse_command()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = COMMAND_NODE;

    // 解析命令的名字
    node->command.command = current_token();

    // 动态分配 args 数组
    node->command.args = malloc(sizeof(char *) * 10); // 初始分配10个位置
    int arg_count = 0;

    // 解析命令后的所有参数
    while (current_token() != NULL && !is_token("|") && !is_token(">") && !is_token("<") && !is_token(">>") && !is_token("&"))
    {
        node->command.args[arg_count++] = current_token(); // 保存参数
        advance_token();                                   // 跳到下一个 token
    }

    node->command.arg_count = arg_count; // 设置参数的数量
    return node;
}

// 解析重定向
ASTNode *parse_redirect()
{
    ASTNode *node = parse_command(); // 解析命令并返回命令节点

    // 如果当前 token 是重定向符号（> 或 <），处理重定向
    if (is_token(">") || is_token("<") || is_token(">>"))
    {
        ASTNode *redirect_node = malloc(sizeof(ASTNode));
        redirect_node->type = REDIRECT_NODE;
        redirect_node->redirect.left = node; // 被重定向的命令

        // 处理重定向符号
        if (is_token(">"))
        {
            redirect_node->redirect.type = OUTPUT_REDIRECT;
        }
        else if (is_token("<"))
        {
            redirect_node->redirect.type = INPUT_REDIRECT;
        }
        else if (is_token(">>"))
        {
            redirect_node->redirect.type = OUTPUT_APPEND_REDIRECT; // 输出追加重定向
        }

        advance_token(); // 跳过重定向符号
        if (current_token() != NULL)
        {
            redirect_node->redirect.file = current_token(); // 目标文件
            advance_token();                                // 跳到下一个 token
        }

        return redirect_node; // 返回包含重定向信息的节点
    }

    // 如果没有重定向符号，直接返回命令节点
    return node;
}

// 解析管道
ASTNode *parse_pipe()
{
    // 先解析命令或重定向命令
    ASTNode *left = parse_redirect();
    if (left == NULL)
        return NULL;

    // 检查管道左侧命令是否是后台进程
    if (is_token("&"))
    {
        left->is_background = 1; // 标记为后台进程
        if (left->type == REDIRECT_NODE)
        {
            (left->redirect).left->is_background = 1;
        }
        advance_token(); // 跳过 "&" 符号
    }

    // 如果当前没有管道符号，直接返回解析出的命令节点
    if (!is_token("|"))
        return left;

    // 如果有管道符号，开始处理管道
    while (is_token("|"))
    {
        advance_token(); // 跳过管道符号 "|"

        // 解析管道右侧的命令或重定向命令
        ASTNode *right = parse_pipe();
        if (right == NULL)
        {
            free(left);
            return NULL; // 如果右侧命令解析失败，返回 NULL
        }

        // 检查管道右侧命令是否是后台进程
        if (is_token("&"))
        {
            right->is_background = 1; // 标记为后台进程
            advance_token();          // 跳过 "&" 符号
            if (right->type == REDIRECT_NODE)
            {
                (right->redirect).left->is_background = 1;
            }
        }

        // 创建一个新的管道节点
        ASTNode *pipe_node = malloc(sizeof(ASTNode));
        pipe_node->type = PIPE_NODE;
        pipe_node->pipe.left = left;
        pipe_node->pipe.right = right;

        // 更新 left 为当前的管道节点，继续处理下一个管道
        left = pipe_node;
    }

    // 返回最终的管道节点或命令节点
    return left;
}

// 解析输入的命令序列
ASTNode *parse(char **input_tokens, int count)
{
    tokens = input_tokens;
    token_count = count;
    current_token_index = 0;

    ASTNode *ast = parse_pipe();
    return ast;
}
