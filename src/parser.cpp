#include "parser.h"
#include <stdexcept>

// 构造函数：保存 token 列表，并把当前位置设为 0
Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

// peek：查看当前 token，但不消耗它
Token Parser::peek() {
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos];
}

// advance：取出当前 token，并把位置向前移动一格
Token Parser::advance() {
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos++];
}

// match：如果当前 token 类型匹配，就吃掉它并返回 true
bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

// parseFactor：处理最基础的表达式单位
// 支持：数字、标识符、括号表达式
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

// parseTerm：处理乘除
// 例如：a * b / c
std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();

    while (peek().type == TokenType::Star ||
           peek().type == TokenType::Slash) {
        Token op = advance();
        auto right = parseFactor();

        left = std::make_unique<BinaryOpNode>(
            op.value,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// parseAdditive：处理加减
// 例如：a + b - c
std::unique_ptr<ASTNode> Parser::parseAdditive() {
    auto left = parseTerm();

    while (peek().type == TokenType::Plus ||
           peek().type == TokenType::Minus) {
        Token op = advance();
        auto right = parseTerm();

        left = std::make_unique<BinaryOpNode>(
            op.value,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// parseComparison：处理比较运算
// 例如：a < b、a == b、a != b
std::unique_ptr<ASTNode> Parser::parseComparison() {
    auto left = parseAdditive();

    while (peek().type == TokenType::Less ||
           peek().type == TokenType::Greater ||
           peek().type == TokenType::EqualEqual ||
           peek().type == TokenType::NotEqual) {
        Token op = advance();
        auto right = parseAdditive();

        left = std::make_unique<BinaryOpNode>(
            op.value,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// parseExpression：表达式总入口
std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseComparison();
}

// parseBlock：解析代码块
// 语法：{ statement* }
std::unique_ptr<ASTNode> Parser::parseBlock() {
    if (!match(TokenType::LBrace)) {
        throw std::runtime_error("Syntax Error: Expected '{' at start of block");
    }

    auto block = std::make_unique<BlockNode>();

    while (peek().type != TokenType::RBrace &&
           peek().type != TokenType::EndOfFile) {
        block->addStatement(parseStatement());
    }

    if (!match(TokenType::RBrace)) {
        throw std::runtime_error("Syntax Error: Expected '}' at end of block");
    }

    return block;
}

// parseVariableDecl：解析变量声明
// 当前支持：int a = expression;
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

    return std::make_unique<VariableDeclNode>(
        "int",
        idToken.value,
        std::move(initExpr)
    );
}

// parseAssignment：解析赋值语句
// 当前支持：a = expression;
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    Token idToken = advance();

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

// parseIfStatement：解析 if / else 语句
// 当前支持：
// if (expression) { ... }
// if (expression) { ... } else { ... }
std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    if (!match(TokenType::Keyword_If)) {
        throw std::runtime_error("Syntax Error: Expected 'if'");
    }

    if (!match(TokenType::LParen)) {
        throw std::runtime_error("Syntax Error: Expected '(' after 'if'");
    }

    // 解析条件表达式
    auto condition = parseExpression();

    if (!match(TokenType::RParen)) {
        throw std::runtime_error("Syntax Error: Expected ')' after if condition");
    }

    // 解析 then 部分
    auto thenBlock = parseBlock();

    // 先默认没有 else
    std::unique_ptr<ASTNode> elseBlock = nullptr;

    // 如果后面跟着 else，就继续解析 else block
    if (match(TokenType::Keyword_Else)) {
        elseBlock = parseBlock();
    }

    return std::make_unique<IfNode>(
        std::move(condition),
        std::move(thenBlock),
        std::move(elseBlock)
    );
}

// parseWhileStatement：解析 while 循环
// 当前支持：while (expression) { ... }
std::unique_ptr<ASTNode> Parser::parseWhileStatement() {
    if (!match(TokenType::Keyword_While)) {
        throw std::runtime_error("Syntax Error: Expected 'while'");
    }

    if (!match(TokenType::LParen)) {
        throw std::runtime_error("Syntax Error: Expected '(' after 'while'");
    }

    // 解析 while 的条件
    auto condition = parseExpression();

    if (!match(TokenType::RParen)) {
        throw std::runtime_error("Syntax Error: Expected ')' after while condition");
    }

    // 当前版本要求 while 后面必须是一个 block
    auto body = parseBlock();

    return std::make_unique<WhileNode>(
        std::move(condition),
        std::move(body)
    );
}

// parseFunction：解析函数定义
// 当前支持：int main() { ... }
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

    return std::make_unique<FunctionNode>(
        "int",
        nameToken.value,
        std::move(body)
    );
}

// parseStatement：根据当前 token 类型分发到不同语句解析函数
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
    else if (peek().type == TokenType::Keyword_If) {
        return parseIfStatement();
    }
    else if (peek().type == TokenType::Keyword_While) {
        return parseWhileStatement();
    }
    else if (peek().type == TokenType::LBrace) {
        return parseBlock();
    }
    else if (peek().type == TokenType::Identifier) {
        return parseAssignment();
    }

    throw std::runtime_error("Syntax Error: Unknown statement");
}

// parse：解析整个程序
// 当前顶层语法：program := function*
std::unique_ptr<ASTNode> Parser::parse() {
    auto program = std::make_unique<BlockNode>();

    while (peek().type != TokenType::EndOfFile) {
        program->addStatement(parseFunction());
    }

    return program;
}