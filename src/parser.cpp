#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

Token Parser::peek() {
    if (pos >= tokens.size()) return tokens.back();
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

// 最底层：数字、变量、括号表达式
std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token token = advance();

    if (token.type == TokenType::Number) {
        return std::make_unique<NumberNode>(token.value);
    }
    else if (token.type == TokenType::Identifier) {
        return std::make_unique<IdentifierNode>(token.value);
    }
    else if (token.type == TokenType::LParen) {
        auto expr = parseExpression();
        if (!match(TokenType::RParen)) {
            throw std::runtime_error("Syntax Error: Expected ')'");
        }
        return expr;
    }

    throw std::runtime_error("Syntax Error: Expected number, identifier, or '('");
}

// 乘除
std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();

    while (peek().type == TokenType::Star || peek().type == TokenType::Slash) {
        Token op = advance();
        auto right = parseFactor();
        left = std::make_unique<BinaryOpNode>(op.value, std::move(left), std::move(right));
    }

    return left;
}

// 加减
std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseTerm();

    while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
        Token op = advance();
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(op.value, std::move(left), std::move(right));
    }

    return left;
}

// 解析代码块：{ statement* }
std::unique_ptr<ASTNode> Parser::parseBlock() {
    if (!match(TokenType::LBrace)) {
        throw std::runtime_error("Syntax Error: Expected '{' at start of block");
    }

    auto block = std::make_unique<BlockNode>();

    while (peek().type != TokenType::RBrace && peek().type != TokenType::EndOfFile) {
        block->addStatement(parseStatement());
    }

    if (!match(TokenType::RBrace)) {
        throw std::runtime_error("Syntax Error: Expected '}' at end of block");
    }

    return block;
}

// 变量声明：int a = expression;
std::unique_ptr<ASTNode> Parser::parseVariableDecl() {
    advance(); // 吃掉 int

    Token idToken = advance();
    if (idToken.type != TokenType::Identifier) {
        throw std::runtime_error("Syntax Error: Expected identifier after 'int'");
    }

    if (!match(TokenType::Assign)) {
        throw std::runtime_error("Syntax Error: Expected '=' in variable declaration");
    }

    auto initExpr = parseExpression();

    if (!match(TokenType::Semicolon)) {
        throw std::runtime_error("Syntax Error: Expected ';' after variable declaration");
    }

    return std::make_unique<VariableDeclNode>("int", idToken.value, std::move(initExpr));
}

// 赋值语句：a = expression;
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    Token idToken = advance(); // 吃掉变量名

    if (idToken.type != TokenType::Identifier) {
        throw std::runtime_error("Syntax Error: Expected identifier at assignment");
    }

    if (!match(TokenType::Assign)) {
        throw std::runtime_error("Syntax Error: Expected '=' in assignment");
    }

    auto expr = parseExpression();

    if (!match(TokenType::Semicolon)) {
        throw std::runtime_error("Syntax Error: Expected ';' after assignment");
    }

    return std::make_unique<AssignmentNode>(idToken.value, std::move(expr));
}

// 函数定义：int main() { ... }
std::unique_ptr<ASTNode> Parser::parseFunction() {
    if (!match(TokenType::Keyword_Int)) {
        throw std::runtime_error("Syntax Error: Expected function return type 'int'");
    }

    Token nameToken = advance();
    if (nameToken.type != TokenType::Identifier) {
        throw std::runtime_error("Syntax Error: Expected function name");
    }

    if (!match(TokenType::LParen)) {
        throw std::runtime_error("Syntax Error: Expected '(' after function name");
    }

    if (!match(TokenType::RParen)) {
        throw std::runtime_error("Syntax Error: Expected ')' after '('");
    }

    auto body = parseBlock();
    return std::make_unique<FunctionNode>("int", nameToken.value, std::move(body));
}

// 语句分发
std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (peek().type == TokenType::Keyword_Int) {
        return parseVariableDecl();
    }
    else if (peek().type == TokenType::Keyword_Return) {
        advance(); // 吃掉 return
        auto expr = parseExpression();
        if (!match(TokenType::Semicolon)) {
            throw std::runtime_error("Syntax Error: Expected ';' after return value");
        }
        return std::make_unique<ReturnNode>(std::move(expr));
    }
    else if (peek().type == TokenType::LBrace) {
        return parseBlock();
    }
    else if (peek().type == TokenType::Identifier) {
        return parseAssignment();
    }

    throw std::runtime_error("Syntax Error: Unknown statement");
}

// 顶层：program := function*
std::unique_ptr<ASTNode> Parser::parse() {
    auto program = std::make_unique<BlockNode>();

    while (peek().type != TokenType::EndOfFile) {
        program->addStatement(parseFunction());
    }

    return program;
}