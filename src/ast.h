#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <iostream>
#include <vector>

// 所有 AST 节点的基类
class ASTNode {
public:
    virtual ~ASTNode() = default;
    // 用于在控制台打印树状结构的虚函数
    virtual void print(int indent = 0) const = 0; 
};

// 数字节点 (例如 "10")
class NumberNode : public ASTNode {
    std::string value;
public:
    NumberNode(std::string val) : value(std::move(val)) {}
    
    void print(int indent = 0) const override {
        // 根据缩进打印空格，形成树状视觉效果
        std::cout << std::string(indent, ' ') << "NumberNode: " << value << "\n";
    }
};

// 变量引用节点 (例如表达式里的 "a")
class IdentifierNode : public ASTNode {
    std::string name;
public:
    IdentifierNode(std::string name) : name(std::move(name)) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "IdentifierNode: " << name << "\n";
    }
};

// 二元操作节点 (例如 "10 + 5")
class BinaryOpNode : public ASTNode {
    std::string op; // 操作符，比如 "+"
    std::unique_ptr<ASTNode> left;  // 左边的表达式
    std::unique_ptr<ASTNode> right; // 右边的表达式
    
public:
    BinaryOpNode(std::string op, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(std::move(op)), left(std::move(l)), right(std::move(r)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "BinaryOpNode: " << op << "\n";
        left->print(indent + 4);  // 打印左孩子
        right->print(indent + 4); // 打印右孩子
    }
};

// 变量声明节点 (例如 "int a = 10;")
class VariableDeclNode : public ASTNode {
    std::string type;   // 比如 "int"
    std::string name;   // 比如 "a"
    std::unique_ptr<ASTNode> initExpr; // 等号右边的表达式 (使用智能指针管理)
    
public:
    VariableDeclNode(std::string t, std::string n, std::unique_ptr<ASTNode> expr)
        : type(std::move(t)), name(std::move(n)), initExpr(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "VariableDeclNode (Type: " << type << ", Name: " << name << ")\n";
        if (initExpr) {
            initExpr->print(indent + 4); // 子节点缩进 4 个空格
        }
    }
};

// 返回语句节点 (例如 "return a + 5;")
class ReturnNode : public ASTNode {
    std::unique_ptr<ASTNode> returnValue;
public:
    ReturnNode(std::unique_ptr<ASTNode> expr) : returnValue(std::move(expr)) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ReturnNode\n";
        if (returnValue) returnValue->print(indent + 4);
    }
};

// 代码块节点 (例如 "{ int a = 1; return a; }")
class BlockNode : public ASTNode {
    // 用一个 vector 装着这个大括号里所有的语句
    std::vector<std::unique_ptr<ASTNode>> statements; 
public:
    // 提供一个向块里添加语句的方法
    void addStatement(std::unique_ptr<ASTNode> stmt) {
        statements.push_back(std::move(stmt));
    }

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "BlockNode {\n";
        // 遍历打印每一条语句
        for (const auto& stmt : statements) {
            stmt->print(indent + 4);
        }
        std::cout << std::string(indent, ' ') << "}\n";
    }
};

#endif