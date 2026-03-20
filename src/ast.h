#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <iostream>
#include <vector>

// ASTNode：所有语法树节点的基类
class ASTNode {
public:
    virtual ~ASTNode() = default;

    // print：以树状结构打印 AST
    virtual void print(int indent = 0) const = 0;
};

// NumberNode：数字字面量节点
// 例如：10、123
class NumberNode : public ASTNode {
    std::string value;

public:
    NumberNode(std::string val) : value(std::move(val)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "NumberNode: " << value << "\n";
    }
};

// IdentifierNode：标识符节点
// 例如：a、main
class IdentifierNode : public ASTNode {
    std::string name;

public:
    IdentifierNode(std::string name) : name(std::move(name)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "IdentifierNode: " << name << "\n";
    }

    const std::string& getName() const {
    return name;
}
};

// BinaryOpNode：二元运算节点
// 例如：a + 1、b * 2
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

        left->print(indent + 4);
        right->print(indent + 4);
    }

    const std::string& getOp() const {
    return op;
    }

    const ASTNode* getLeft() const {
        return left.get();
    }

    const ASTNode* getRight() const {
        return right.get();
    }
};


// UnaryOpNode：一元运算节点
// 例如：-a、-5
class UnaryOpNode : public ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> operand;

public:
    UnaryOpNode(std::string oper, std::unique_ptr<ASTNode> expr)
        : op(std::move(oper)), operand(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "UnaryOpNode: " << op << "\n";

        if (operand) {
            operand->print(indent + 4);
        }
    }

    const std::string& getOp() const {
        return op;
    }

    const ASTNode* getOperand() const {
        return operand.get();
    }
};

// VariableDeclNode：变量声明节点
// 例如：int a = 10;
class VariableDeclNode : public ASTNode {
    std::string type;                     // 类型，例如 int
    std::string name;                     // 变量名，例如 a
    std::unique_ptr<ASTNode> initExpr;    // 初始化表达式

public:
    VariableDeclNode(std::string t,
                     std::string n,
                     std::unique_ptr<ASTNode> expr)
        : type(std::move(t)), name(std::move(n)), initExpr(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "VariableDeclNode (Type: " << type
                  << ", Name: " << name << ")\n";

        if (initExpr) {
            initExpr->print(indent + 4);
        }
    }

    const std::string& getType() const {
    return type;
    }   

    const std::string& getName() const {
        return name;
    }

    const ASTNode* getInitExpr() const {
        return initExpr.get();
    }
};

// AssignmentNode：赋值语句节点
// 例如：a = a + 1;
class AssignmentNode : public ASTNode {
    std::string name;                  // 左边被赋值的变量名
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

    const std::string& getName() const {
    return name;
    }

    const ASTNode* getValue() const {
        return value.get();
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

    const ASTNode* getReturnValue() const {
        return returnValue.get();
    }
};

// IfNode：if / else 语句节点
// 例如：if (a < 10) { ... } else { ... }
class IfNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;   // if 条件
    std::unique_ptr<ASTNode> thenBlock;   // 条件成立时执行的语句块
    std::unique_ptr<ASTNode> elseBlock;   // else 部分，可为空

public:
    IfNode(std::unique_ptr<ASTNode> cond,
           std::unique_ptr<ASTNode> thenStmt,
           std::unique_ptr<ASTNode> elseStmt = nullptr)
        : condition(std::move(cond)),
          thenBlock(std::move(thenStmt)),
          elseBlock(std::move(elseStmt)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "IfNode\n";

        std::cout << std::string(indent + 4, ' ')
                  << "Condition:\n";
        if (condition) {
            condition->print(indent + 8);
        }

        std::cout << std::string(indent + 4, ' ')
                  << "Then:\n";
        if (thenBlock) {
            thenBlock->print(indent + 8);
        }

        // 如果有 else，就继续打印
        if (elseBlock) {
            std::cout << std::string(indent + 4, ' ')
                      << "Else:\n";
            elseBlock->print(indent + 8);
        }
    }


    const ASTNode* getCondition() const {
    return condition.get(); 
    }

    const ASTNode* getThenBlock() const {
        return thenBlock.get();
    }

    const ASTNode* getElseBlock() const {
        return elseBlock.get();
    }
};

// WhileNode：while 循环节点
// 例如：while (a < 5) { ... }
class WhileNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;   // 循环条件
    std::unique_ptr<ASTNode> body;        // 循环体

public:
    WhileNode(std::unique_ptr<ASTNode> cond,
              std::unique_ptr<ASTNode> loopBody)
        : condition(std::move(cond)),
          body(std::move(loopBody)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "WhileNode\n";

        std::cout << std::string(indent + 4, ' ')
                  << "Condition:\n";
        if (condition) {
            condition->print(indent + 8);
        }

        std::cout << std::string(indent + 4, ' ')
                  << "Body:\n";
        if (body) {
            body->print(indent + 8);
        }
    }

    const ASTNode* getCondition() const {
    return condition.get();
    }

    const ASTNode* getBody() const {
        return body.get();
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
    // 添加一条语句到 block 中
    void addStatement(std::unique_ptr<ASTNode> stmt) {
        statements.push_back(std::move(stmt));
    }

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "BlockNode {\n";

        for (const auto& stmt : statements) {
            stmt->print(indent + 4);
        }

        std::cout << std::string(indent, ' ')
                  << "}\n";
    }

    const std::vector<std::unique_ptr<ASTNode>>& getStatements() const {
    return statements;
    }   
};

//参数结构体
struct Parameter {
    std::string type;
    std::string name;
};

// FunctionNode：函数定义节点
// 例如：int main() { ... }
class FunctionNode : public ASTNode {
    std::string returnType;                   // 返回类型
    std::string name;                         // 函数名
    std::vector<Parameter> parameters;        // 参数列表
    std::unique_ptr<ASTNode> body;            // 函数体

public:
    FunctionNode(std::string retType,
                 std::string funcName,
                 std::vector<Parameter> params,
                 std::unique_ptr<ASTNode> funcBody)
        : returnType(std::move(retType)),
          name(std::move(funcName)),
          parameters(std::move(params)),
          body(std::move(funcBody)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "FunctionNode (ReturnType: " << returnType
                  << ", Name: " << name << ")\n";

        if (!parameters.empty()) {
            std::cout << std::string(indent + 4, ' ') << "Parameters:\n";
            for (const auto& param : parameters) {
                std::cout << std::string(indent + 8, ' ')
                          << param.type << " " << param.name << "\n";
            }
        }

        if (body) {
            body->print(indent + 4);
        }
    }

    const std::string& getReturnType() const {
        return returnType;
    }

    const std::string& getName() const {
        return name;
    }

    const std::vector<Parameter>& getParameters() const {
        return parameters;
    }

    const ASTNode* getBody() const {
        return body.get();
    }
};

#endif