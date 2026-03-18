#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <iostream>
#include <vector>

// ASTNode：所有语法树节点的基类
// 以后所有具体节点都继承它
class ASTNode {
public:
    virtual ~ASTNode() = default;

    // print 用来把语法树打印出来
    // indent 表示缩进层级，方便看树状结构
    virtual void print(int indent = 0) const = 0;
};

// NumberNode：数字字面量节点
// 例如：10、123、42
class NumberNode : public ASTNode {
    std::string value;

public:
    NumberNode(std::string val) : value(std::move(val)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "NumberNode: " << value << "\n";
    }
};

// IdentifierNode：变量名/标识符节点
// 例如：a、main、result
class IdentifierNode : public ASTNode {
    std::string name;

public:
    IdentifierNode(std::string name) : name(std::move(name)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "IdentifierNode: " << name << "\n";
    }
};

// BinaryOpNode：二元运算节点
// 例如：a + 1、b * 2
//
// 结构上它会有：
// - 运算符 op
// - 左子树 left
// - 右子树 right
class BinaryOpNode : public ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

public:
    BinaryOpNode(std::string op,
                 std::unique_ptr<ASTNode> l,
                 std::unique_ptr<ASTNode> r)
        : op(std::move(op)), left(std::move(l)), right(std::move(r)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "BinaryOpNode: " << op << "\n";

        // 继续打印左右子树
        left->print(indent + 4);
        right->print(indent + 4);
    }
};

// VariableDeclNode：变量声明节点
// 例如：int a = 10;
class VariableDeclNode : public ASTNode {
    std::string type;                     // 变量类型，例如 int
    std::string name;                     // 变量名，例如 a
    std::unique_ptr<ASTNode> initExpr;    // 初始化表达式，例如 10 或 1 + 2

public:
    VariableDeclNode(std::string t,
                     std::string n,
                     std::unique_ptr<ASTNode> expr)
        : type(std::move(t)), name(std::move(n)), initExpr(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "VariableDeclNode (Type: " << type
                  << ", Name: " << name << ")\n";

        // 如果有初始化表达式，就继续打印
        if (initExpr) {
            initExpr->print(indent + 4);
        }
    }
};

// AssignmentNode：赋值语句节点
// 例如：a = a + 1;
class AssignmentNode : public ASTNode {
    std::string name;                  // 被赋值的变量名
    std::unique_ptr<ASTNode> value;    // 右边的表达式

public:
    AssignmentNode(std::string n, std::unique_ptr<ASTNode> v)
        : name(std::move(n)), value(std::move(v)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "AssignmentNode (Name: " << name << ")\n";

        if (value) {
            value->print(indent + 4);
        }
    }
};

// ReturnNode：return 语句节点
// 例如：return a + 1;
class ReturnNode : public ASTNode {
    std::unique_ptr<ASTNode> returnValue;

public:
    ReturnNode(std::unique_ptr<ASTNode> expr)
        : returnValue(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "ReturnNode\n";

        if (returnValue) {
            returnValue->print(indent + 4);
        }
    }
};

// BlockNode：代码块节点
// 例如：
// {
//     int a = 10;
//     return a;
// }
class BlockNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;

public:
    // 往代码块里加入一条语句
    void addStatement(std::unique_ptr<ASTNode> stmt) {
        statements.push_back(std::move(stmt));
    }

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "BlockNode {\n";

        // 依次打印 block 里的每条语句
        for (const auto& stmt : statements) {
            stmt->print(indent + 4);
        }

        std::cout << std::string(indent, ' ')
                  << "}\n";
    }
};

// FunctionNode：函数定义节点
// 例如：int main() { ... }
class FunctionNode : public ASTNode {
    std::string returnType;             // 返回类型，例如 int
    std::string name;                   // 函数名，例如 main
    std::unique_ptr<ASTNode> body;      // 函数体，本质上是一个 BlockNode

public:
    FunctionNode(std::string retType,
                 std::string funcName,
                 std::unique_ptr<ASTNode> funcBody)
        : returnType(std::move(retType)),
          name(std::move(funcName)),
          body(std::move(funcBody)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "FunctionNode (ReturnType: " << returnType
                  << ", Name: " << name << ")\n";

        if (body) {
            body->print(indent + 4);
        }
    }
};

#endif