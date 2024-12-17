#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <termios.h>

#define MAX_CMD_LENGTH 1024
#define MAX_DIR_LENGTH 1024

// 函数：恢复终端模式
void reset_terminal_mode()
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ECHO | ICANON); // 启用回显和规范模式
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// 函数：读取用户输入
char *read_input()
{
    // 获取当前工作目录
    char cwd[MAX_DIR_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd failed");
        exit(1);
    }

    // 动态构造提示符，格式为：当前目录 >
    char prompt[MAX_DIR_LENGTH + 3]; // +3 为 " >" 和空字符
    snprintf(prompt, sizeof(prompt), "%s > ", cwd);

    // 在调用 readline 前，确保恢复终端模式（避免原始模式导致的显示问题）
    reset_terminal_mode();

    // 使用 GNU Readline 获取输入，显示当前工作目录作为提示符
    char *input = readline(prompt);

    // 如果输入为空，则返回 NULL
    if (input == NULL)
    {
        return NULL; // 如果输入为空
    }

    // 检查是否输入了内容，如果输入了，就加入历史记录
    if (*input)
    {
        add_history(input);
    }

    return input;
}
