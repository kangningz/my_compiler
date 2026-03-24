#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

// ============================================================
// 读取整个文件内容到字符串
// ============================================================
std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Error: cannot open input file: " + path);
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

// ============================================================
// 把 TokenType 转成可读字符串，便于 --tokens 输出
// ============================================================
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Keyword_Int:      return "Keyword_Int";
        case TokenType::Keyword_Return:   return "Keyword_Return";
        case TokenType::Keyword_If:       return "Keyword_If";
        case TokenType::Keyword_Else:     return "Keyword_Else";
        case TokenType::Keyword_While:    return "Keyword_While";
        case TokenType::Identifier:       return "Identifier";
        case TokenType::Number:           return "Number";
        case TokenType::Assign:           return "Assign";
        case TokenType::EqualEqual:       return "EqualEqual";
        case TokenType::NotEqual:         return "NotEqual";
        case TokenType::Less:             return "Less";
        case TokenType::Greater:          return "Greater";
        case TokenType::LessEqual:        return "LessEqual";
        case TokenType::GreaterEqual:     return "GreaterEqual";
        case TokenType::Semicolon:        return "Semicolon";
        case TokenType::Comma:            return "Comma";
        case TokenType::Plus:             return "Plus";
        case TokenType::Minus:            return "Minus";
        case TokenType::Star:             return "Star";
        case TokenType::Slash:            return "Slash";
        case TokenType::LParen:           return "LParen";
        case TokenType::RParen:           return "RParen";
        case TokenType::LBrace:           return "LBrace";
        case TokenType::RBrace:           return "RBrace";
        case TokenType::EndOfFile:        return "EndOfFile";
        default:                          return "Unknown";
    }
}

// ============================================================
// 打印使用说明
// ============================================================
void printUsage() {
    std::cout
        << "Usage:\n"
        << "  ./my_compiler <input-file> [options]\n\n"
        << "Options:\n"
        << "  --tokens        Print token list\n"
        << "  --ast           Print AST\n"
        << "  --sem           Print semantic analysis result\n"
        << "  --emit-llvm     Generate LLVM IR\n"
        << "  -o <file>       Output LLVM IR to file\n";
}

// ============================================================
// main：编译器主入口
// ============================================================
int main(int argc, char* argv[]) {
    try {
        // 至少需要一个输入文件
        if (argc < 2) {
            printUsage();
            return 1;
        }

        std::string inputFile;
        std::string outputFile;

        bool showTokens = false;
        bool showAst = false;
        bool showSem = false;
        bool emitLLVM = false;

        // 第一个非程序名参数视为输入文件
        inputFile = argv[1];

        // 解析后续命令行参数
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg == "--tokens") {
                showTokens = true;
            } else if (arg == "--ast") {
                showAst = true;
            } else if (arg == "--sem") {
                showSem = true;
            } else if (arg == "--emit-llvm") {
                emitLLVM = true;
            } else if (arg == "-o") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Error: missing output file after -o");
                }
                outputFile = argv[++i];
            } else {
                throw std::runtime_error("Error: unknown option: " + arg);
            }
        }

        // 读取源码文件
        std::string code = readFile(inputFile);

        // 词法分析
        Scanner scanner(code);
        std::vector<Token> tokens = scanner.tokenize();

        if (showTokens) {
            std::cout << "--- Tokens ---\n";
            for (const auto& token : tokens) {
                std::cout << tokenTypeToString(token.type)
                          << " : " << token.value << "\n";
            }
            std::cout << "\n";
        }

        // 语法分析
        Parser parser(tokens);
        std::unique_ptr<ASTNode> ast = parser.parse();

        if (showAst) {
            std::cout << "--- Abstract Syntax Tree ---\n";
            ast->print();
            std::cout << "\n";
        }

        // 语义分析
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast.get());

        if (showSem) {
            std::cout << "--- Semantic Analysis ---\n";
            std::cout << "Semantic analysis passed.\n\n";
        }

        // 生成 LLVM IR
        if (emitLLVM) {
            CodeGenerator codegen;
            codegen.generate(ast.get());

            if (!outputFile.empty()) {
                if (!codegen.dumpIR(outputFile)) {
                    throw std::runtime_error("Error: failed to write LLVM IR to file: " + outputFile);
                }
                std::cout << "LLVM IR written to " << outputFile << "\n";
            } else {
                codegen.printIR();
            }
        }

        // 如果用户没有显式指定输出行为，
        // 默认给一个成功提示，表示“前端 + 语义分析”已经通过。
        if (!showTokens && !showAst && !showSem && !emitLLVM) {
            std::cout << "Compilation pipeline finished successfully.\n";
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}