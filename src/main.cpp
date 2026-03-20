#include <iostream>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

int main() {
    std::string code = R"(
    int add(int x, int y) {
        int z = x + y;
        return z;
    }

    int main() {
        int a = 3;
        return a;
    }
    )";

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

            std::cout << "\n--- Semantic Analysis ---\n";
            SemanticAnalyzer analyzer;
            analyzer.analyze(ast.get());
            std::cout << "Semantic analysis passed.\n";
        } else {
            std::cout << "Nothing to parse.\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}