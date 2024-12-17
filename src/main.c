#include <stdio.h>
#include <stdlib.h>
#include "input_cmd.h"
#include "lexical_parser.h"
#include "syntax_parser.h"
#include "cmd_executor.h"
#include "free_ast.h"

int main()
{
    while (1)
    {
        char *cmd = read_input();

        int token_count = 0;
        char **tokens = lexical_parse(cmd, &token_count);

        // 调用语法解析
        ASTNode *ast = parse(tokens, token_count);

        execute_node(ast);
        free_ast(ast);

        // 释放内存
        free_tokens(tokens, token_count);
        free(cmd);
    }

    return 0;
}
