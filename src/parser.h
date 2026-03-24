#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

// ============================================================
// Parser：递归下降语法分析器
// ============================================================
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    // 解析整个程序
    std::unique_ptr<ASTNode> parse();

private:
    std::vector<Token> tokens;
    size_t pos;

    // ---------------- 表达式解析 ----------------
    std::unique_ptr<ASTNode> parseExpression();   // 表达式总入口
    std::unique_ptr<ASTNode> parseComparison();   // 处理比较运算
    std::unique_ptr<ASTNode> parseAdditive();     // 处理 + -
    std::unique_ptr<ASTNode> parseTerm();         // 处理 * /
    std::unique_ptr<ASTNode> parseFactor();       // 处理数字、标识符、括号、一元负号

    // ---------------- 语句解析 ----------------
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ASTNode> parseVariableDecl();
    std::unique_ptr<ASTNode> parseAssignment();
    std::unique_ptr<ASTNode> parseIdentifierStatement(); // 新增：解析“以标识符开头”的语句
    std::unique_ptr<ASTNode> parseIfStatement();
    std::unique_ptr<ASTNode> parseWhileStatement();

    // ---------------- 函数相关解析 ----------------
    std::vector<Parameter> parseParameterList();
    std::vector<std::unique_ptr<ASTNode>> parseArgumentList();
    std::unique_ptr<ASTNode> parseIdentifierOrCall();
    std::unique_ptr<ASTNode> parseFunction();

    // ---------------- token 辅助函数 ----------------
    Token peek();
    Token advance();
    bool match(TokenType type);
};

#endif