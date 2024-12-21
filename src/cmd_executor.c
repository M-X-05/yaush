#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "syntax_parser.h"

#define MAX_PATH_LENGTH 1024

// 自定义的 PATH
const char *PATH[] = {
    "/usr/local/mybin", // 示例路径
    "/usr/bin",         // 示例路径
    NULL                // 结束符
};

// 执行echo命令
void execute_echo(ASTNode *node)
{
    for (int i = 1; i < node->command.arg_count; i++)
    {
        printf("%s", node->command.args[i]);
        if (i < node->command.arg_count - 1)
        {
            printf(" "); // 添加空格分隔参数
        }
    }
    printf("\n");
}

// 执行 cd 命令
void execute_cd(ASTNode *node)
{
    const char *path = (node->command.arg_count > 1) ? node->command.args[1] : getenv("HOME");

    if (chdir(path) == -1)
    {
        perror("cd");
    }
}

// 执行内置命令
int execute_builtin_command(ASTNode *node, int input_fd, int output_fd)
{
    // 保存标准输入输出的文件描述符
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);
    int is_builtin_command = 0;

    if (input_fd != -1)
    {
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            close(input_fd);
            return;
        }
    }
    if (output_fd != -1)
    {
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(output_fd);
            return;
        }
    }

    if (strcmp(node->command.command, "echo") == 0)
    {
        execute_echo(node);
        is_builtin_command = 1;
    }
    else if (strcmp(node->command.command, "cd") == 0)
    {
        execute_cd(node);
        is_builtin_command = 1;
    }
    // 恢复标准输入输出
    if (dup2(saved_stdin, STDIN_FILENO) == -1)
    {
        perror("dup2 restore stdin");
    }
    if (dup2(saved_stdout, STDOUT_FILENO) == -1)
    {
        perror("dup2 restore stdout");
    }
    // 关闭保存的文件描述符
    close(saved_stdin);
    close(saved_stdout);
    return is_builtin_command; // 不是内置命令
}

// 查找命令的可执行文件
char *find_command_in_PATH(const char *command)
{
    for (int i = 0; PATH[i] != NULL; i++)
    {
        // 构造完整的命令路径
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", PATH[i], command);

        // 检查文件是否存在并且可执行
        struct stat st;
        if (stat(full_path, &st) == 0 && (st.st_mode & S_IXUSR))
        {
            return strdup(full_path); // 返回可执行文件的路径
        }
    }
    return NULL; // 如果找不到，返回 NULL
}

// 检查命令是否为可执行文件路径
int is_executable_path(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 && (st.st_mode & S_IXUSR));
}

// 在PATH中查找可执行文件
char *find_executable_in_path(const char *command, const char *path_env)
{
    char *path_copy = strdup(path_env);
    if (!path_copy)
    {
        perror("strdup");
        return NULL;
    }

    char *dir = strtok(path_copy, ":");
    while (dir != NULL)
    {
        // 拼接路径和命令
        size_t len = strlen(dir) + strlen(command) + 2; // +2 for '/' and '\0'
        char *full_path = malloc(len);
        if (!full_path)
        {
            perror("malloc");
            free(path_copy);
            return NULL;
        }

        snprintf(full_path, len, "%s/%s", dir, command);

        // 如果找到可执行文件，返回该路径
        if (is_executable_path(full_path))
        {
            free(path_copy);
            return full_path;
        }

        free(full_path);
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL; // 未找到
}

char *get_command_path(ASTNode *node)
{
    // 先尝试在自定义 PATH 中查找命令
    char *command_path = find_command_in_PATH(node->command.command);

    if (command_path == NULL)
    {
        // 如果没有找到，检查是否为路径
        if (is_executable_path(node->command.command))
        {
            command_path = strdup(node->command.command); // 直接使用给定路径
        }
    }

    return command_path;
}

void execute_command(ASTNode *node, char *command_path, int input_fd, int output_fd)
{
    // 保存标准输入输出的文件描述符
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    if (input_fd != -1)
    {
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            close(input_fd);
            return;
        }
    }
    if (output_fd != -1)
    {
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(output_fd);
            return;
        }
    }

    node->command.args[0] = command_path;
    if (execv(command_path, node->command.args) == -1)
    {
        perror("execv");
        exit(1);
    }
    // 恢复标准输入输出
    if (dup2(saved_stdin, STDIN_FILENO) == -1)
    {
        perror("dup2 restore stdin");
    }
    if (dup2(saved_stdout, STDOUT_FILENO) == -1)
    {
        perror("dup2 restore stdout");
    }
    // 关闭保存的文件描述符
    close(saved_stdin);
    close(saved_stdout);
}

// 执行外部命令
void execute_external_command(ASTNode *node, int input_fd, int output_fd)
{
    // 先尝试在自定义 PATH 中查找命令
    char *command_path = get_command_path(node);

    if (command_path != NULL)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {

            // 子进程执行外部命令
            execute_command(node, command_path, input_fd, output_fd);
        }
        else
        {
            // 如果是前台进程，等待子进程完成
            if (node->is_background == 0)
            {
                waitpid(pid, NULL, 0);
            }
        }

        free(command_path); // 释放命令路径
    }
    else
    {
        fprintf(stderr, "command not found: %s\n", node->command.command);
    }
}

// 执行命令
void execute_command_node(ASTNode *node, int input_fd, int output_fd)
{

    if (!execute_builtin_command(node, input_fd, output_fd))
    {
        // 执行外部命令
        execute_external_command(node, input_fd, output_fd);
    }
}

// 执行重定向命令
void execute_redirect_node(ASTNode *node, int input_fd, int output_fd)
{

    if (node->redirect.type == INPUT_REDIRECT)
    {
        // 输入重定向
        input_fd = open(node->redirect.file, O_RDONLY);
        if (input_fd == -1)
        {
            perror("open input file");
            return;
        }
    }
    if (node->redirect.type == OUTPUT_REDIRECT)
    {
        // 输出重定向
        output_fd = open(node->redirect.file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1)
        {
            perror("open output file");
            return;
        }
    }
    else if (node->redirect.type == OUTPUT_APPEND_REDIRECT)
    {
        // 追加输出重定向
        output_fd = open(node->redirect.file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (output_fd == -1)
        {
            perror("open output file");
            return;
        }
    }

    // 关闭文件描述符
    execute_command_node(node->redirect.left, input_fd, output_fd);
}

// 执行外部命令并处理管道
void execute_pipe_node(ASTNode *node, int input_fd)
{
    if (node == NULL)
        return;

    int pipe_fds[2]; // 管道文件描述符
    if (node->pipe.right != NULL)
    {
        // 创建管道
        if (pipe(pipe_fds) == -1)
        {
            perror("pipe");
            exit(1);
        }
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        close(pipe_fds[0]);
        // 子进程执行
        if (node->pipe.left->type == REDIRECT_NODE)
        {
            execute_redirect_node(node->pipe.left, input_fd, pipe_fds[1]);
        }
        else
        {
            // 先尝试在自定义 PATH 中查找命令
            char *command_path = get_command_path(node->pipe.left);

            if (command_path != NULL)
            {
                // 子进程执行外部命令
                execute_command(node->pipe.left, command_path, input_fd, pipe_fds[1]);
                free(command_path); // 释放命令路径
            }
            else
            {
                fprintf(stderr, "command not found: %s\n", node->command.command);
            }
        }

        // 关闭管道的文件描述符
        close(pipe_fds[1]);
    }
    else
    {
        // 父进程执行
        // 关闭管道的写端

        close(pipe_fds[1]);

        if (node->pipe.right->type == PIPE_NODE)
        {
            execute_pipe_node(node->pipe.right, pipe_fds[0]);
        }
        else if (node->pipe.right->type == REDIRECT_NODE)
        {
            execute_redirect_node(node->pipe.right, pipe_fds[0], -1);
        }
        else
        {
            execute_command_node(node->pipe.right, pipe_fds[0], -1);
        }
        // 关闭管道的读端
        close(pipe_fds[0]);
    }
}

// 执行节点的主函数
void execute_node(ASTNode *node)
{
    switch (node->type)
    {
    case COMMAND_NODE:
        // 执行内置命令或外部命令
        execute_command_node(node, -1, -1);
        break;

    case REDIRECT_NODE:
        // 执行重定向命令
        execute_redirect_node(node, -1, -1);
        break;

    case PIPE_NODE:
        execute_pipe_node(node, -1);
        break;

    default:
        fprintf(stderr, "Unsupported node type: %d\n", node->type);
        break;
    }
}