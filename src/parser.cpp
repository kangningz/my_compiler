#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

Token Parser::peek() {
    if (pos >= tokens.size()) return tokens.back(); // 如果越界，返回最后一个 Token (EOF)
    return tokens[pos];
}

Token Parser::advance() {
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos++];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

// 最底层的因子：它可以是数字、变量，或者是括号里的表达式
std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token token = advance();
    
    if (token.type == TokenType::Number) {
        return std::make_unique<NumberNode>(token.value);
    } 
    else if (token.type == TokenType::Identifier) {
        return std::make_unique<IdentifierNode>(token.value);
    } 
    else if (token.type == TokenType::LParen) { // 遇到左括号
        auto expr = parseExpression();          // 递归调用，把括号里的当成全新表达式解析
        if (!match(TokenType::RParen)) {
            throw std::runtime_error("Syntax Error: Expected ')'");
        }
        return expr;
    }
    throw std::runtime_error("Syntax Error: Expected number, identifier, or '('");
}

// 项：处理乘除法 (优先级较高，所以放在 Expression 下层)
std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor(); // 先解析左边的一个因子
    
    // 如果紧跟着的是 * 或 /，就不断组合它们
    while (peek().type == TokenType::Star || peek().type == TokenType::Slash) {
        Token op = advance(); // 吃掉操作符
        auto right = parseFactor(); // 解析右边的一个因子
        left = std::make_unique<BinaryOpNode>(op.value, std::move(left), std::move(right));
    }
    return left;
}

// 表达式：处理加减法 (优先级较低)
std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseTerm(); // 先解析左边的一个项 (可能已经包含了乘除法)
    
    // 如果紧跟着的是 + 或 -，就不断组合它们
    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        Token op = advance();
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(op.value, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parse() {
    // 我们的微型编译器目前只认得 int 开头的变量声明
    if (peek().type == TokenType::Keyword_Int) {
        return parseVariableDecl();
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseVariableDecl() {
    advance(); // 吃掉 "int"
    Token idToken = advance();
    if (idToken.type != TokenType::Identifier) throw std::runtime_error("Expected identifier");
    if (!match(TokenType::Assign)) throw std::runtime_error("Expected '='");

    // 【魔法发生在这里】
    // 直接把等号后面的所有东西，交给统一的表达式解析器！
    auto initExpr = parseExpression();

    if (!match(TokenType::Semicolon)) throw std::runtime_error("Expected ';'");
    return std::make_unique<VariableDeclNode>("int", idToken.value, std::move(initExpr));
}