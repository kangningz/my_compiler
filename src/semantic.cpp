#include "semantic.h"
#include <stdexcept>

// ============================================================
// analyze：语义分析总入口
// ============================================================
void SemanticAnalyzer::analyze(const ASTNode* node) {
    // 每次分析前先清空状态
    scopes.clear();
    functions.clear();

    // 第一步：预扫描函数定义
    collectFunctions(node);

    // 第二步：递归分析整棵 AST
    analyzeNode(node);
}

// ============================================================
// enterScope：进入一个新作用域
// ============================================================
void SemanticAnalyzer::enterScope() {
    scopes.push_back({});
}

// ============================================================
// exitScope：退出当前作用域
// ============================================================
void SemanticAnalyzer::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

// ============================================================
// declareVariable：在当前作用域声明变量
// ============================================================
void SemanticAnalyzer::declareVariable(const std::string& name) {
    // 如果当前还没有作用域，就先创建一个
    if (scopes.empty()) {
        enterScope();
    }

    auto& currentScope = scopes.back();

    // 当前作用域已经存在同名变量，属于重复定义
    if (currentScope.find(name) != currentScope.end()) {
        throw std::runtime_error(
            "Semantic Error: Redefinition of variable '" + name + "'"
        );
    }

    currentScope.insert(name);
}

// ============================================================
// isDeclared：检查变量是否已经声明
// 从内层作用域往外层作用域查找
// ============================================================
bool SemanticAnalyzer::isDeclared(const std::string& name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }
    return false;
}

// ============================================================
// analyzeNode：递归分析 AST
// ============================================================
void SemanticAnalyzer::analyzeNode(const ASTNode* node) {
    if (!node) {
        return;
    }

    // 数字字面量：不需要额外语义检查
    if (dynamic_cast<const NumberNode*>(node)) {
        return;
    }

    // 标识符：必须先声明后使用
    if (auto idNode = dynamic_cast<const IdentifierNode*>(node)) {
        if (!isDeclared(idNode->getName())) {
            throw std::runtime_error(
                "Semantic Error: Use of undeclared variable '" + idNode->getName() + "'"
            );
        }
        return;
    }

    // 二元表达式：递归检查左右子树
    if (auto binNode = dynamic_cast<const BinaryOpNode*>(node)) {
        analyzeNode(binNode->getLeft());
        analyzeNode(binNode->getRight());
        return;
    }

    // 一元表达式：递归检查操作数
    if (auto unaryNode = dynamic_cast<const UnaryOpNode*>(node)) {
        analyzeNode(unaryNode->getOperand());
        return;
    }

    // 函数调用：
    // 1. 检查函数是否存在
    // 2. 检查参数个数是否匹配
    // 3. 检查每个实参表达式是否合法
    if (auto callNode = dynamic_cast<const CallNode*>(node)) {
        auto it = functions.find(callNode->getCalleeName());
        if (it == functions.end()) {
            throw std::runtime_error(
                "Semantic Error: Call to undefined function '" + callNode->getCalleeName() + "'"
            );
        }

        size_t expectedCount = it->second;
        size_t actualCount = callNode->getArguments().size();

        if (expectedCount != actualCount) {
            throw std::runtime_error(
                "Semantic Error: Function '" + callNode->getCalleeName() +
                "' expects " + std::to_string(expectedCount) +
                " argument(s), but got " + std::to_string(actualCount)
            );
        }

        for (const auto& arg : callNode->getArguments()) {
            analyzeNode(arg.get());
        }

        return;
    }

    // 变量声明：
    // 先检查初始化表达式，再把变量加入当前作用域
    if (auto varDecl = dynamic_cast<const VariableDeclNode*>(node)) {
        analyzeNode(varDecl->getInitExpr());
        declareVariable(varDecl->getName());
        return;
    }

    // 赋值语句：
    // 左边变量必须已经声明
    if (auto assignNode = dynamic_cast<const AssignmentNode*>(node)) {
        if (!isDeclared(assignNode->getName())) {
            throw std::runtime_error(
                "Semantic Error: Assignment to undeclared variable '" + assignNode->getName() + "'"
            );
        }

        analyzeNode(assignNode->getValue());
        return;
    }

    // 表达式语句：继续检查其中的表达式
    if (auto exprStmt = dynamic_cast<const ExprStatementNode*>(node)) {
        analyzeNode(exprStmt->getExpr());
        return;
    }

    // return：检查返回值表达式
    if (auto returnNode = dynamic_cast<const ReturnNode*>(node)) {
        analyzeNode(returnNode->getReturnValue());
        return;
    }

    // 代码块：创建新作用域，再分析所有语句
    if (auto blockNode = dynamic_cast<const BlockNode*>(node)) {
        enterScope();

        for (const auto& stmt : blockNode->getStatements()) {
            analyzeNode(stmt.get());
        }

        exitScope();
        return;
    }

    // if：检查条件、then 分支、else 分支
    if (auto ifNode = dynamic_cast<const IfNode*>(node)) {
        analyzeNode(ifNode->getCondition());
        analyzeNode(ifNode->getThenBlock());
        analyzeNode(ifNode->getElseBlock());
        return;
    }

    // while：检查条件和循环体
    if (auto whileNode = dynamic_cast<const WhileNode*>(node)) {
        analyzeNode(whileNode->getCondition());
        analyzeNode(whileNode->getBody());
        return;
    }

    // 函数定义：
    // 进入函数作用域 -> 注册参数 -> 分析函数体 -> 退出作用域
    if (auto funcNode = dynamic_cast<const FunctionNode*>(node)) {
        enterScope();

        for (const auto& param : funcNode->getParameters()) {
            declareVariable(param.name);
        }

        analyzeNode(funcNode->getBody());

        exitScope();
        return;
    }

    throw std::runtime_error("Semantic Error: Unknown AST node type");
}

// ============================================================
// collectFunctions：预扫描整棵 AST，收集所有函数定义
// ============================================================
void SemanticAnalyzer::collectFunctions(const ASTNode* node) {
    if (!node) {
        return;
    }

    // 顶层 BlockNode 中保存的是所有函数定义
    if (auto blockNode = dynamic_cast<const BlockNode*>(node)) {
        for (const auto& stmt : blockNode->getStatements()) {
            collectFunctions(stmt.get());
        }
        return;
    }

    // 函数定义节点：加入函数表
    if (auto funcNode = dynamic_cast<const FunctionNode*>(node)) {
        const std::string& name = funcNode->getName();

        if (functions.find(name) != functions.end()) {
            throw std::runtime_error(
                "Semantic Error: Redefinition of function '" + name + "'"
            );
        }

        functions[name] = funcNode->getParameters().size();
        return;
    }
}