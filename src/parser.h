#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

// Parser：语法分析器
// 输入：token 序列
// 输出：AST（抽象语法树）
class Parser {
public:
    Parser(const std::vector<Token>& tokens);

    // 解析整个程序入口
    std::unique_ptr<ASTNode> parse();

private:
    std::vector<Token> tokens;  // 所有 token
    size_t pos;                 // 当前解析位置

    // 下面这三个函数负责表达式优先级解析
    //
    // parseExpression: 处理 + -
    // parseTerm      : 处理 * /
    // parseFactor    : 处理 数字 / 标识符 / 括号
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();

    // 解析一条语句
    std::unique_ptr<ASTNode> parseStatement();

    // 解析代码块：{ ... }
    std::unique_ptr<ASTNode> parseBlock();

    // 解析变量声明：int a = expr;
    std::unique_ptr<ASTNode> parseVariableDecl();

    // 解析赋值语句：a = expr;
    std::unique_ptr<ASTNode> parseAssignment();

    // 解析函数定义：int main() { ... }
    std::unique_ptr<ASTNode> parseFunction();

    // 看一眼当前 token，但不前进
    Token peek();

    // 取出当前 token，并前进一格
    Token advance();

    // 如果当前 token 类型符合，就吃掉并返回 true
    // 否则不动，返回 false
    bool match(TokenType type);
};

#endif