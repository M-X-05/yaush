#ifndef LEXICAL_PARSER_H
#define LEXICAL_PARSER_H

// 函数声明
char **lexical_parse(const char *input, int *token_count);
void free_tokens(char **tokens, int token_count);

#endif // LEXICAL_PARSER_H
