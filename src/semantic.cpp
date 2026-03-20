#include "semantic.h"
#include <stdexcept>
#include <typeinfo>

// 开始分析整棵 AST
void SemanticAnalyzer::analyze(const ASTNode* node) {
    scopes.clear();
    analyzeNode(node);
}

// 进入一个新作用域
void SemanticAnalyzer::enterScope() {
    scopes.push_back({});
}

// 退出当前作用域
void SemanticAnalyzer::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

// 在当前作用域声明变量
void SemanticAnalyzer::declareVariable(const std::string& name) {
    if (scopes.empty()) {
        enterScope();
    }

    auto& currentScope = scopes.back();

    // 如果当前作用域已经有同名变量，说明重复定义
    if (currentScope.find(name) != currentScope.end()) {
        throw std::runtime_error("Semantic Error: Redefinition of variable '" + name + "'");
    }

    currentScope.insert(name);
}

// 检查变量是否已经声明
bool SemanticAnalyzer::isDeclared(const std::string& name) const {
    // 从内层作用域往外层找
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }
    return false;
}

// 递归分析 AST
void SemanticAnalyzer::analyzeNode(const ASTNode* node) {
    if (!node) return;

    // NumberNode：数字不需要语义检查
    if (dynamic_cast<const NumberNode*>(node)) {
        return;
    }

    // IdentifierNode：使用变量时，必须已经声明
    if (auto idNode = dynamic_cast<const IdentifierNode*>(node)) {
        if (!isDeclared(idNode->getName())) {
            throw std::runtime_error(
                "Semantic Error: Use of undeclared variable '" + idNode->getName() + "'"
            );
        }
        return;
    }

    // BinaryOpNode：递归检查左右子树
    if (auto binNode = dynamic_cast<const BinaryOpNode*>(node)) {
        analyzeNode(binNode->getLeft());
        analyzeNode(binNode->getRight());
        return;
    }

    // UnaryOpNode：递归检查操作数
    if (auto unaryNode = dynamic_cast<const UnaryOpNode*>(node)) {
        analyzeNode(unaryNode->getOperand());
        return;
    }

    // VariableDeclNode：
    // 1. 先检查初始化表达式
    // 2. 再把变量加入当前作用域
    //
    // 这样 int a = a + 1; 在当前简化规则下会报错，
    // 因为右边的 a 还没完成声明
    if (auto varDecl = dynamic_cast<const VariableDeclNode*>(node)) {
        analyzeNode(varDecl->getInitExpr());
        declareVariable(varDecl->getName());
        return;
    }

    // AssignmentNode：
    // 1. 左边变量必须已声明
    // 2. 右边表达式也要递归检查
    if (auto assignNode = dynamic_cast<const AssignmentNode*>(node)) {
        if (!isDeclared(assignNode->getName())) {
            throw std::runtime_error(
                "Semantic Error: Assignment to undeclared variable '" + assignNode->getName() + "'"
            );
        }

        analyzeNode(assignNode->getValue());
        return;
    }

    // ReturnNode：检查返回值表达式
    if (auto returnNode = dynamic_cast<const ReturnNode*>(node)) {
        analyzeNode(returnNode->getReturnValue());
        return;
    }

    // BlockNode：进入新作用域，检查所有语句，退出作用域
    if (auto blockNode = dynamic_cast<const BlockNode*>(node)) {
        enterScope();

        for (const auto& stmt : blockNode->getStatements()) {
            analyzeNode(stmt.get());
        }

        exitScope();
        return;
    }

    // IfNode：
    // 检查条件、then 分支、else 分支
    if (auto ifNode = dynamic_cast<const IfNode*>(node)) {
        analyzeNode(ifNode->getCondition());
        analyzeNode(ifNode->getThenBlock());
        analyzeNode(ifNode->getElseBlock());
        return;
    }

    // WhileNode：
    // 检查条件和循环体
    if (auto whileNode = dynamic_cast<const WhileNode*>(node)) {
        analyzeNode(whileNode->getCondition());
        analyzeNode(whileNode->getBody());
        return;
    }

    // FunctionNode：
    // 当前简化处理：函数体直接作为一个子节点分析
    if (auto funcNode = dynamic_cast<const FunctionNode*>(node)) {
        enterScope();

        // 先把形参加入当前函数作用域
        for (const auto& param : funcNode->getParameters()) {
            declareVariable(param.name);
        }

        analyzeNode(funcNode->getBody());

        exitScope();
        return;
    }

    throw std::runtime_error("Semantic Error: Unknown AST node type");
}