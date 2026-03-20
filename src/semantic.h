#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include <string>
#include <vector>
#include <unordered_set>

// SemanticAnalyzer：语义分析器
// 当前先支持：
// 1. 检查变量是否先声明后使用
// 2. 检查同一作用域内是否重复定义
class SemanticAnalyzer {
public:
    // 对整棵 AST 做语义分析
    void analyze(const ASTNode* node);

private:
    // 作用域栈
    // 每一层作用域用一个 unordered_set 保存变量名
    std::vector<std::unordered_set<std::string>> scopes;

    // 进入/退出作用域
    void enterScope();
    void exitScope();

    // 在当前作用域声明变量
    void declareVariable(const std::string& name);

    // 检查变量是否已经声明
    bool isDeclared(const std::string& name) const;

    // 递归分析各种节点
    void analyzeNode(const ASTNode* node);
};

#endif