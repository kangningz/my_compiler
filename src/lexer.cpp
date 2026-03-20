#include "lexer.h"
#include <cctype>

// 构造函数：保存源码，并把扫描位置设为 0
Scanner::Scanner(const std::string& src) : source(src), pos(0) {}

// tokenize：把源码拆成 token 序列
std::vector<Token> Scanner::tokenize() {
    std::vector<Token> tokens;

    while (pos < source.length()) {
        char current = source[pos];

        // 跳过空白字符
        if (std::isspace(current)) {
            pos++;
            continue;
        }

        // 识别标识符或关键字
        if (std::isalpha(current)) {
            std::string word;

            while (pos < source.length() && std::isalnum(source[pos])) {
                word += source[pos];
                pos++;
            }

            if (word == "int") {
                tokens.push_back({TokenType::Keyword_Int, word});
            }
            else if (word == "return") {
                tokens.push_back({TokenType::Keyword_Return, word});
            }
            else if (word == "if") {
                tokens.push_back({TokenType::Keyword_If, word});
            }
            else if (word == "else") {
                tokens.push_back({TokenType::Keyword_Else, word});
            }
            else if (word == "while") {
                tokens.push_back({TokenType::Keyword_While, word});
            }
            else {
                tokens.push_back({TokenType::Identifier, word});
            }

            continue;
        }

        // 识别数字
        if (std::isdigit(current)) {
            std::string number;

            while (pos < source.length() && std::isdigit(source[pos])) {
                number += source[pos];
                pos++;
            }

            tokens.push_back({TokenType::Number, number});
            continue;
        }

        // 双字符运算符：== 和 !=
        if (current == '=' && pos + 1 < source.length() && source[pos + 1] == '=') {
            tokens.push_back({TokenType::EqualEqual, "=="});
            pos += 2;
            continue;
        }

        if (current == '!' && pos + 1 < source.length() && source[pos + 1] == '=') {
            tokens.push_back({TokenType::NotEqual, "!="});
            pos += 2;
            continue;
        }
        if (current == '<' && pos + 1 < source.length() && source[pos + 1] == '=') {
        tokens.push_back({TokenType::LessEqual, "<="});
        pos += 2;
        continue;
        }

        if (current == '>' && pos + 1 < source.length() && source[pos + 1] == '=') {
        tokens.push_back({TokenType::GreaterEqual, ">="});
        pos += 2;
        continue;
        }

        // 单字符符号
        switch (current) {
            case '=':
                tokens.push_back({TokenType::Assign, "="});
                break;
            case '<':
                tokens.push_back({TokenType::Less, "<"});
                break;
            case '>':
                tokens.push_back({TokenType::Greater, ">"});
                break;
            case ';':
                tokens.push_back({TokenType::Semicolon, ";"});
                break;
            case ',':
                tokens.push_back({TokenType::Comma, ","});
                break;
            case '+':
                tokens.push_back({TokenType::Plus, "+"});
                break;
            case '-':
                tokens.push_back({TokenType::Minus, "-"});
                break;
            case '*':
                tokens.push_back({TokenType::Star, "*"});
                break;
            case '/':
                tokens.push_back({TokenType::Slash, "/"});
                break;
            case '(':
                tokens.push_back({TokenType::LParen, "("});
                break;
            case ')':
                tokens.push_back({TokenType::RParen, ")"});
                break;
            case '{':
                tokens.push_back({TokenType::LBrace, "{"});
                break;
            case '}':
                tokens.push_back({TokenType::RBrace, "}"});
                break;
            default:
                break;
        }

        pos++;
    }

    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}