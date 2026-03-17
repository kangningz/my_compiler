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

// 解析代码块：一直读，直到遇到 '}'
std::unique_ptr<ASTNode> Parser::parseBlock() {
    advance(); // 吃掉 '{'
    auto block = std::make_unique<BlockNode>();

    // 只要没遇到 '}' 也没遇到文件结尾，就一直循环解析语句
    while (peek().type != TokenType::RBrace && peek().type != TokenType::EndOfFile) {
        block->addStatement(parseStatement()); // 解析一条语句，塞进 block 里
    }

    if (!match(TokenType::RBrace)) {
        throw std::runtime_error("Syntax Error: Expected '}' at the end of block");
    }
    return block;
}

// 语句分发器：根据当前 Token 决定调用哪个解析函数
std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (peek().type == TokenType::Keyword_Int) {
        return parseVariableDecl(); // 我们之前写的变量声明逻辑
    } 
    else if (peek().type == TokenType::Keyword_Return) {
        advance(); // 吃掉 "return"
        auto expr = parseExpression(); // 解析 return 后面的表达式
        if (!match(TokenType::Semicolon)) throw std::runtime_error("Expected ';' after return value");
        return std::make_unique<ReturnNode>(std::move(expr));
    }
    else if (peek().type == TokenType::LBrace) {
        return parseBlock(); // 如果遇到 '{'，说明语句里面套着一个子代码块
    }
    
    throw std::runtime_error("Syntax Error: Unknown statement");
}

// 彻底修改最顶层的 parse 函数
std::unique_ptr<ASTNode> Parser::parse() {
    // 整个文件其实就可以看作是一个隐形的、最大的 Block
    auto program = std::make_unique<BlockNode>();
    while (peek().type != TokenType::EndOfFile) {
        program->addStatement(parseStatement());
    }
    return program;
}

std::unique_ptr<ASTNode> Parser::parseVariableDecl() {
    advance(); // 吃掉 "int"
    Token idToken = advance();
    if (idToken.type != TokenType::Identifier) throw std::runtime_error("Expected identifier");
    if (!match(TokenType::Assign)) throw std::runtime_error("Expected '='");


    // 直接把等号后面的所有东西，交给统一的表达式解析器！
    auto initExpr = parseExpression();

    if (!match(TokenType::Semicolon)) throw std::runtime_error("Expected ';'");
    return std::make_unique<VariableDeclNode>("int", idToken.value, std::move(initExpr));
}