# my_compiler

一个基于 **C++17 + LLVM** 实现的 MiniC 编译器项目。  
当前项目支持从 MiniC 源文件出发，经过：

- 词法分析（Lexer）
- 递归下降语法分析（Parser）
- AST 构建
- 语义分析（Semantic Analysis）
- LLVM IR 生成（Code Generation）

并且可以通过 `lli` 直接执行生成的 LLVM IR，验证程序运行结果。

---

## 1. 项目目标

本项目的目标是从零实现一个简化版的小编译器，在实践中理解编译器前端与 LLVM IR 生成流程。

当前阶段重点：

- 手写 Lexer
- 手写递归下降 Parser
- 构建 AST
- 实现作用域语义检查
- 基于 LLVM 生成 LLVM IR
- 通过 `lli` 执行生成结果

---

## 2. 当前支持的语言特性

当前支持的 MiniC 子集包括：

- `int` 函数
- 局部变量声明与初始化
- 变量赋值
- 算术运算：`+ - * /`
- 比较运算：`< > <= >= == !=`
- `if / else`
- `while`
- `return`
- 函数定义
- 函数调用
- 函数调用作为单独语句，例如 `foo();`

---

## 3. 项目结构

my_compiler/
├── CMakeLists.txt
├── README.md
├── examples/
│   ├── add.mc
│   ├── if_test.mc
│   ├── while_test.mc
│   └── call_stmt.mc
├── src/
│   ├── ast.h
│   ├── lexer.h
│   ├── lexer.cpp
│   ├── parser.h
│   ├── parser.cpp
│   ├── semantic.h
│   ├── semantic.cpp
│   ├── codegen.h
│   ├── codegen.cpp
│   └── main.cpp
└── test/

---

## 4. 开发环境

Ubuntu 20.04.4
VSCode
CMake >= 3.16
C++17
LLVM 20

---

## 5. 编译方式

在项目根目录下执行：
mkdir -p build
cmake -S . -B build -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm
cmake --build build -j

编译完成后，可执行文件位于：
build/my_compiler

---

## 6. 使用方式

先进入构建目录：
cd build

打印 token
./my_compiler ../examples/add.mc --tokens

打印 AST
./my_compiler ../examples/add.mc --ast

打印语义分析
./my_compiler ../examples/add.mc --sem

生成LLVM IR
./my_compiler ../examples/add.mc --emit-llvm -o add.ll

执行LLVM IR
lli-20 add.ll
echo $?

---

## 7. 当前实现的编译流程

MiniC source (.mc)
-> Lexer
-> Parser
-> AST
-> Semantic Analysis
-> LLVM IR (.ll)
-> lli execution

---

## 8. 当前已完成的阶段性成果

完成手写词法分析器
完成递归下降语法分析器
完成 AST 设计与构建
完成基本语义分析
完成 LLVM IR 生成
已验证 add / if / while 示例可通过 lli 执行

---

## 9.后续计划

补充更多测试样例
改进错误提示信息
支持生成 object file
尝试接入 LLVM 优化 pass
持续完善 README 和项目文档