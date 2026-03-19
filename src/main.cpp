#include <iostream>
#include "lexer.h"
#include "parser.h"

int main() {
    std::string code = R"(
int main() {
    int a = 0;

    while (a < 5) {
        a = a + 1;
    }

    if(a == 1)
    {
        b = b + 1; 
    }

    return a;
}
)"; //暂时无法支持语义分析，所以无法判断b未声明

    std::cout << "Source Code:\n" << code << "\n";

    Scanner scanner(code);
    std::vector<Token> tokens = scanner.tokenize();

    std::cout << "--- Tokens ---\n";
    for (const auto& token : tokens) {
        std::cout << "Token: '" << token.value << "'\n";
    }
    std::cout << "\n";

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
        std::cerr << e.what() << '\n';
    }

    return 0;
}