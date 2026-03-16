#include "lexer.h"
#include <cctype> // 用于 std::isalpha, std::isdigit 等函数

Scanner::Scanner(const std::string& source) : src(source), pos(0) {}

char Scanner::peek() {
    if (pos >= src.length()) return '\0';
    return src[pos];
}

char Scanner::advance() {
    if (pos >= src.length()) return '\0';
    return src[pos++];
}

void Scanner::skipWhitespace() {
    // 如果当前字符是空白符（空格、回车、制表符等），就一直往后吞
    while (std::isspace(peek())) {
        advance();
    }
}

std::vector<Token> Scanner::tokenize() {
    std::vector<Token> tokens;
    
    while (pos < src.length()) {
        skipWhitespace();
        if (pos >= src.length()) break;

        char c = peek();

        // 1. 识别字母打头的：可能是关键字 "int" 或普通变量名
        if (std::isalpha(c)) {
            std::string str;
            while (std::isalnum(peek())) { // 只要是字母或数字就一直读
                str += advance();
            }
            if (str == "int") {
                tokens.push_back({TokenType::Keyword_Int, str});
            } else {
                tokens.push_back({TokenType::Identifier, str});
            }
        }
        // 2. 识别数字打头的：提取出完整的整数
        else if (std::isdigit(c)) {
            std::string num;
            while (std::isdigit(peek())) {
                num += advance();
            }
            tokens.push_back({TokenType::Number, num});
        }
        // 3. 识别特定符号
        else if (c == '=') { tokens.push_back({TokenType::Assign, "="}); advance(); }
        else if (c == '+') { tokens.push_back({TokenType::Plus, "+"}); advance(); }
        else if (c == '-') { tokens.push_back({TokenType::Minus, "-"}); advance(); }
        else if (c == '*') { tokens.push_back({TokenType::Star, "*"}); advance(); }
        else if (c == '/') { tokens.push_back({TokenType::Slash, "/"}); advance(); }
        else if (c == '(') { tokens.push_back({TokenType::LParen, "("}); advance(); }
        else if (c == ')') { tokens.push_back({TokenType::RParen, ")"}); advance(); }
        else if (c == ';') { tokens.push_back({TokenType::Semicolon, ";"}); advance(); }
        // 4. 其他不认识的字符
        else {
            tokens.push_back({TokenType::Unknown, std::string(1, c)});
            advance();
        }
    }
    
    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}