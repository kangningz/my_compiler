#include "parser.h"
#include <stdexcept>

// ============================================================
// 构造函数：保存 token 列表，并把当前位置设为 0
// ============================================================
Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), pos(0) {}

// ============================================================
// peek：查看当前 token，但不消耗它
// ============================================================
Token Parser::peek() {
    if (pos >= tokens.size()) {
        return tokens.back();
    }
    return tokens[pos];
}

// ============================================================
// advance：取出当前 token，并把位置向前移动一格
// ============================================================
Token Parser::advance() {
    if (pos >= tokens.size()) {
        return tokens.back();
    }
    return tokens[pos++];
}

// ============================================================
// match：如果当前 token 类型匹配，就吃掉它并返回 true
// ============================================================
bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

// ============================================================
// parseFactor：处理最基础的表达式单位
// 当前支持：
// 1. 数字               123
// 2. 标识符             a
// 3. 函数调用           add(1, 2)
// 4. 一元负号           -a, -5
// 5. 括号表达式         (a + 1)
// ============================================================
std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token token = peek();

    // 一元负号
    if (token.type == TokenType::Minus) {
        advance(); // 吃掉 '-'
        auto operand = parseFactor();
        return std::make_unique<UnaryOpNode>("-", std::move(operand));
    }

    // 数字
    if (token.type == TokenType::Number) {
        advance();
        return std::make_unique<NumberNode>(token.value);
    }

    // 标识符 或 函数调用
    if (token.type == TokenType::Identifier) {
        return parseIdentifierOrCall();
    }

    // 括号表达式
    if (token.type == TokenType::LParen) {
        advance(); // 吃掉 '('

        auto expr = parseExpression();

        if (!match(TokenType::RParen)) {
            throw std::runtime_error("Syntax Error: Expected ')'");
        }

        return expr;
    }

    throw std::runtime_error("Syntax Error: Expected number, identifier, unary '-', or '('");
}

// ============================================================
// parseTerm：处理乘除
// 例如：a * b / c
// ============================================================
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

// ============================================================
// parseAdditive：处理加减
// 例如：a + b - c
// ============================================================
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

// ============================================================
// parseComparison：处理比较运算
// 例如：a < b、a == b、a != b
// ============================================================
std::unique_ptr<ASTNode> Parser::parseComparison() {
    auto left = parseAdditive();

    while (peek().type == TokenType::Less ||
           peek().type == TokenType::Greater ||
           peek().type == TokenType::LessEqual ||
           peek().type == TokenType::GreaterEqual ||
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

// ============================================================
// parseExpression：表达式总入口
// ============================================================
std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseComparison();
}

// ============================================================
// parseBlock：解析代码块
// 语法：{ statement* }
// ============================================================
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

// ============================================================
// parseVariableDecl：解析变量声明
// 当前支持：int a = expression;
// ============================================================
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

// ============================================================
// parseAssignment：解析赋值语句
// 当前支持：a = expression;
// ============================================================
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

// ============================================================
// parseIdentifierStatement：解析“以标识符开头”的语句
//
// 这里需要区分两种情况：
// 1. 赋值语句：a = 123;
// 2. 表达式语句：foo(1, 2);
//
// 当前版本只允许“函数调用”作为表达式语句，不允许单独写 a;
// ============================================================
std::unique_ptr<ASTNode> Parser::parseIdentifierStatement() {
    // 如果下一个 token 是 '='，说明这是赋值语句
    if (pos + 1 < tokens.size() && tokens[pos + 1].type == TokenType::Assign) {
        return parseAssignment();
    }

    // 否则按“标识符 or 函数调用表达式”来解析
    auto expr = parseIdentifierOrCall();

    if (!match(TokenType::Semicolon)) {
        throw std::runtime_error("Syntax Error: Expected ';' after expression statement");
    }

    // 当前阶段只允许函数调用单独作为一条语句
    if (dynamic_cast<CallNode*>(expr.get()) == nullptr) {
        throw std::runtime_error(
            "Syntax Error: Only function calls can be used as expression statements"
        );
    }

    return std::make_unique<ExprStatementNode>(std::move(expr));
}

// ============================================================
// parseIfStatement：解析 if / else
// 当前支持：
// if (expression) { ... }
// if (expression) { ... } else { ... }
// ============================================================
std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    if (!match(TokenType::Keyword_If)) {
        throw std::runtime_error("Syntax Error: Expected 'if'");
    }

    if (!match(TokenType::LParen)) {
        throw std::runtime_error("Syntax Error: Expected '(' after 'if'");
    }

    auto condition = parseExpression();

    if (!match(TokenType::RParen)) {
        throw std::runtime_error("Syntax Error: Expected ')' after if condition");
    }

    auto thenBlock = parseBlock();

    std::unique_ptr<ASTNode> elseBlock = nullptr;

    if (match(TokenType::Keyword_Else)) {
        elseBlock = parseBlock();
    }

    return std::make_unique<IfNode>(
        std::move(condition),
        std::move(thenBlock),
        std::move(elseBlock)
    );
}

// ============================================================
// parseWhileStatement：解析 while 循环
// 当前支持：while (expression) { ... }
// ============================================================
std::unique_ptr<ASTNode> Parser::parseWhileStatement() {
    if (!match(TokenType::Keyword_While)) {
        throw std::runtime_error("Syntax Error: Expected 'while'");
    }

    if (!match(TokenType::LParen)) {
        throw std::runtime_error("Syntax Error: Expected '(' after 'while'");
    }

    auto condition = parseExpression();

    if (!match(TokenType::RParen)) {
        throw std::runtime_error("Syntax Error: Expected ')' after while condition");
    }

    auto body = parseBlock();

    return std::make_unique<WhileNode>(
        std::move(condition),
        std::move(body)
    );
}

// ============================================================
// parseParameterList：解析函数形参列表
// 当前仅支持 int 参数
// 例如：int x, int y
// ============================================================
std::vector<Parameter> Parser::parseParameterList() {
    std::vector<Parameter> params;

    // 空参数列表，例如 foo()
    if (peek().type == TokenType::RParen) {
        return params;
    }

    while (true) {
        if (!match(TokenType::Keyword_Int)) {
            throw std::runtime_error("Syntax Error: Expected parameter type 'int'");
        }

        Token nameToken = advance();
        if (nameToken.type != TokenType::Identifier) {
            throw std::runtime_error("Syntax Error: Expected parameter name");
        }

        params.push_back({"int", nameToken.value});

        if (match(TokenType::Comma)) {
            continue;
        }

        break;
    }

    return params;
}

// ============================================================
// parseArgumentList：解析函数调用时的实参列表
// 例如：foo(a, 1, b + 2)
// ============================================================
std::vector<std::unique_ptr<ASTNode>> Parser::parseArgumentList() {
    std::vector<std::unique_ptr<ASTNode>> args;

    // 空实参列表，例如 foo()
    if (peek().type == TokenType::RParen) {
        return args;
    }

    while (true) {
        args.push_back(parseExpression());

        if (match(TokenType::Comma)) {
            continue;
        }

        break;
    }

    return args;
}

// ============================================================
// parseIdentifierOrCall：解析“变量引用”或“函数调用”
// 例如：a
//       add(1, 2)
// ============================================================
std::unique_ptr<ASTNode> Parser::parseIdentifierOrCall() {
    Token idToken = advance();

    if (idToken.type != TokenType::Identifier) {
        throw std::runtime_error("Syntax Error: Expected identifier");
    }

    // 如果后面跟着 '('，说明是函数调用
    if (match(TokenType::LParen)) {
        auto args = parseArgumentList();

        if (!match(TokenType::RParen)) {
            throw std::runtime_error("Syntax Error: Expected ')' after argument list");
        }

        return std::make_unique<CallNode>(idToken.value, std::move(args));
    }

    // 否则就是普通变量引用
    return std::make_unique<IdentifierNode>(idToken.value);
}

// ============================================================
// parseFunction：解析函数定义
// 当前支持：int main() { ... }
// ============================================================
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

    auto params = parseParameterList();

    if (!match(TokenType::RParen)) {
        throw std::runtime_error("Syntax Error: Expected ')' after parameter list");
    }

    auto body = parseBlock();

    return std::make_unique<FunctionNode>(
        "int",
        nameToken.value,
        std::move(params),
        std::move(body)
    );
}

// ============================================================
// parseStatement：根据当前 token 类型分发到不同语句解析函数
// ============================================================
std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (peek().type == TokenType::Keyword_Int) {
        return parseVariableDecl();
    }

    if (peek().type == TokenType::Keyword_Return) {
        advance(); // 吃掉 return

        auto expr = parseExpression();

        if (!match(TokenType::Semicolon)) {
            throw std::runtime_error("Syntax Error: Expected ';' after return value");
        }

        return std::make_unique<ReturnNode>(std::move(expr));
    }

    if (peek().type == TokenType::Keyword_If) {
        return parseIfStatement();
    }

    if (peek().type == TokenType::Keyword_While) {
        return parseWhileStatement();
    }

    if (peek().type == TokenType::LBrace) {
        return parseBlock();
    }

    if (peek().type == TokenType::Identifier) {
        return parseIdentifierStatement();
    }

    throw std::runtime_error("Syntax Error: Unknown statement");
}

// ============================================================
// parse：解析整个程序
// 当前顶层语法：program := function*
// ============================================================
std::unique_ptr<ASTNode> Parser::parse() {
    auto program = std::make_unique<BlockNode>();

    while (peek().type != TokenType::EndOfFile) {
        program->addStatement(parseFunction());
    }

    return program;
}