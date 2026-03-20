#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

// Token 类型枚举
enum class TokenType {
    Keyword_Int,     // int
    Keyword_Return,  // return
    Keyword_If,      // if
    Keyword_Else,    // else
    Keyword_While,   // while

    Identifier,      // 标识符，例如 a、main
    Number,          // 数字，例如 123

    Assign,          // =
    EqualEqual,      // ==
    NotEqual,        // !=
    Less,            // <
    Greater,         // >
    LessEqual,       // <=
    GreaterEqual,    // >=

    Semicolon,       // ;
    Comma,           // ,
    Plus,            // +
    Minus,           // -
    Star,            // *
    Slash,           // /

    LParen,          // (
    RParen,          // )
    LBrace,          // {
    RBrace,          // }

    EndOfFile        // 输入结束
};

// Token 结构体
struct Token {
    TokenType type;
    std::string value;
};

// Scanner：词法分析器
class Scanner {
private:
    std::string source;
    size_t pos;

public:
    Scanner(const std::string& src);
    std::vector<Token> tokenize();
};

#endif