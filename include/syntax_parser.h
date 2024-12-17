#ifndef SYNTAX_PARSER_H
#define SYNTAX_PARSER_H

// 重定向类型
enum
{
    INPUT_REDIRECT,
    OUTPUT_REDIRECT,
    OUTPUT_APPEND_REDIRECT
};

// ASTNode结构体定义
typedef struct ASTNode
{
    enum
    {
        COMMAND_NODE,
        PIPE_NODE,
        REDIRECT_NODE,
    } type;
    union
    {
        struct
        {
            char *command;
            char **args;
            int arg_count;
        } command;
        struct
        {
            struct ASTNode *left;
            struct ASTNode *right;
        } pipe;
        struct
        {
            struct ASTNode *left;
            char *file;
            int type;
        } redirect;
    };

    int is_background;
} ASTNode;

// 语法解析函数声明
static char *current_token();
static void advance_token();
static int is_token(const char *token);
ASTNode *parse_command();
ASTNode *parse_redirect();
ASTNode *parse_pipe();
ASTNode *parse(char **input_tokens, int count);

#endif // SYNTAX_PARSER_H
