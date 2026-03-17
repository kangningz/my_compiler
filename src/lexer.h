#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

// 定义 Token 的类型
enum class TokenType {
    Keyword_Int, 
    Keyword_Return, // 【新增】"return"
    Identifier, Number, Assign, Semicolon,
    Plus, Minus, Star, Slash, LParen, RParen,
    LBrace,         // 【新增】"{"
    RBrace,         // 【新增】"}"
    EndOfFile, Unknown
};
// Token 结构体
struct Token {
    TokenType type;
    std::string value;
};

// 词法扫描器类
class Scanner {
public:
    Scanner(const std::string& source);
    std::vector<Token> tokenize(); // 核心方法：将源码转换为 Token 数组

private:
    std::string src;
    size_t pos;      // 当前读取到的字符位置
    
    char peek();     // 查看当前字符（不移动指针）
    char advance();  // 读取当前字符，并把指针向后移动一位
    void skipWhitespace(); // 跳过空格和换行
};

#endif