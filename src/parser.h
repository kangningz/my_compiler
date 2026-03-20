#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

// Parser：语法分析器
class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ASTNode> parse();

private:
    std::vector<Token> tokens;
    size_t pos;

    // 表达式解析
    std::unique_ptr<ASTNode> parseExpression();   // 最外层入口
    std::unique_ptr<ASTNode> parseComparison();   // 处理 < > == !=
    std::unique_ptr<ASTNode> parseAdditive();     // 处理 + -
    std::unique_ptr<ASTNode> parseTerm();         // 处理 * /
    std::unique_ptr<ASTNode> parseFactor();       // 处理数字、标识符、括号

    // 语句解析
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ASTNode> parseVariableDecl();
    std::unique_ptr<ASTNode> parseAssignment();
    std::unique_ptr<ASTNode> parseIfStatement();
    std::unique_ptr<ASTNode> parseWhileStatement();
    std::vector<Parameter> parseParameterList();  //函数参数
    std::unique_ptr<ASTNode> parseFunction();

    // token 辅助函数
    Token peek();
    Token advance();
    bool match(TokenType type);
};

#endif