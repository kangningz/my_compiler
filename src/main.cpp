#include <iostream>
#include "lexer.h"
#include "parser.h"

int main() {
    std::string code = R"(
int main() {
    int a = 10;
    a = a + 1;
    int b = 1 + 5 * 2;
    return a + b;
}
)";

    std::cout << "Source Code:\n" << code << "\n";

    // 第一步：词法分析
    // 把源码拆成 token 序列
    Scanner scanner(code);
    std::vector<Token> tokens = scanner.tokenize();

    // 打印 token，方便检查 lexer 是否正常
    std::cout << "--- Tokens ---\n";
    for (const auto& token : tokens) {
        std::cout << "Token: '" << token.value << "'\n";
    }
    std::cout << "\n";

    // 第二步：语法分析
    // 把 token 序列转换成 AST
    std::cout << "--- Abstract Syntax Tree ---\n";
    Parser parser(tokens);

    try {
        std::unique_ptr<ASTNode> ast = parser.parse();

        if (ast) {
            ast->print();
        } else {
            std::cout << "Nothing to parse.\n";
        }
    }
    catch (const std::exception& e) {
        // 如果语法分析中报错，就把错误信息打印出来
        std::cerr << e.what() << '\n';
    }

    return 0;
}