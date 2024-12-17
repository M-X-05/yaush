# 设置项目的最低 CMake 版本和项目名称
cmake_minimum_required(VERSION 3.10)
project(YAUSH C)

# 设置 C 标准为 C99
set(CMAKE_C_STANDARD 99)

# 设置构建类型为 Debug
set(CMAKE_BUILD_TYPE Debug)

# 为调试模式添加编译选项
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

# 添加 include 目录，供编译器查找头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/include)


# 指定可执行文件
add_executable(main
    ../src/input_cmd.c
    ../src/lexical_parser.c
    ../src/syntax_parser.c
    ../src/cmd_executor.c
    ../src/free_ast.c
    ../src/main.c
)

target_link_libraries(main readline) # 链接数学库

# 你可以根据需要添加其他源文件或目标
# target_sources(test_input_cmd PRIVATE another_file.c)
