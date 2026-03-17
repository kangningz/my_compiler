#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ASTNode> parse(); // 核心方法：开始解析并返回 AST 根节点

private:
    std::vector<Token> tokens;
    size_t pos;

    std::unique_ptr<ASTNode> parseExpression(); // 解析加减
    std::unique_ptr<ASTNode> parseTerm();       // 解析乘除
    std::unique_ptr<ASTNode> parseFactor();     // 解析数字、变量、括号
    std::unique_ptr<ASTNode> parseStatement(); // 解析单条语句（比如变量声明、或 return）
    std::unique_ptr<ASTNode> parseBlock();     // 解析代码块 { ... }

    Token peek();             // 查看当前 Token
    Token advance();          // 消耗并返回当前 Token
    bool match(TokenType type); // 如果当前 Token 类型匹配，则消耗它并返回 true

    // 专门用于解析变量声明的函数
    std::unique_ptr<ASTNode> parseVariableDecl(); 
};

#endif