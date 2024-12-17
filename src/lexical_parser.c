#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexical_parser.h"

#define MAX_TOKENS 128

// 函数：词法解析，拆分命令字符串为 token 列表
char **lexical_parse(const char *input, int *token_count)
{
    char **tokens = malloc(MAX_TOKENS * sizeof(char *));
    if (tokens == NULL)
    {
        perror("malloc failed");
        exit(1);
    }

    const char *ptr = input;
    int count = 0;

    while (*ptr != '\0')
    {
        // 跳过空格
        while (isspace(*ptr))
        {
            ptr++;
        }

        if (*ptr == '\0')
        {
            break;
        }

        // 解析一个普通的单词（命令或参数）
        const char *start = ptr;
        while (*ptr != '\0' && !isspace(*ptr))
        {
            ptr++;
        }
        size_t length = ptr - start;
        tokens[count] = malloc((length + 1) * sizeof(char));
        if (tokens[count] == NULL)
        {
            perror("malloc failed");
            exit(1);
        }
        strncpy(tokens[count], start, length);
        tokens[count][length] = '\0';
        count++;
    }

    *token_count = count;
    return tokens;
}

// 函数：释放token内存
void free_tokens(char **tokens, int token_count)
{
    for (int i = 0; i < token_count; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}
