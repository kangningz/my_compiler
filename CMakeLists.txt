cmake_minimum_required(VERSION 3.10)
project(MyCompiler)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 指定生成可执行文件，并包含两个 cpp 源文件
add_executable(my_compiler src/main.cpp src/lexer.cpp src/parser.cpp src/semantic.cpp)