#include <iostream>
#include "lexer.h"
#include "parser.h"

int main() {
    std::string code = "int a = 10 + 5 * (2 - b);";
    std::cout << "Source Code: " << code << "\n\n";

    // 1. 词法分析 (Lexer)
    Scanner scanner(code);
    std::vector<Token> tokens = scanner.tokenize();

    std::cout << "--- Tokens ---\n";
    for (const auto& token : tokens) {
        std::cout << "Token: '" << token.value << "'\n";
    }
    std::cout << "\n";

    // 2. 语法分析 (Parser) 建立 AST
    std::cout << "--- Abstract Syntax Tree ---\n";
    Parser parser(tokens);
    try {
        std::unique_ptr<ASTNode> ast = parser.parse();
        if (ast) {
            ast->print(); // 打印树结构
        } else {
            std::cout << "Nothing to parse.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n'; // 如果语法错误，打印报错信息
    }

    return 0;
}
//cmake -G "MinGW Makefiles" ..
//mingw32-make
//.\my_compiler.exe