#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <iostream>
#include <vector>

// ============================================================
// ASTNode：所有语法树节点的基类
// ============================================================
class ASTNode {
public:
    virtual ~ASTNode() = default;

    // 以树状结构打印 AST
    virtual void print(int indent = 0) const = 0;
};

// ============================================================
// NumberNode：数字字面量节点
// 例如：10、123
// ============================================================
class NumberNode : public ASTNode {
private:
    std::string value;

public:
    explicit NumberNode(std::string val) : value(std::move(val)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "NumberNode: " << value << "\n";
    }

    // 返回数字字符串
    const std::string& getValue() const {
        return value;
    }
};

// ============================================================
// IdentifierNode：标识符节点
// 例如：a、main
// ============================================================
class IdentifierNode : public ASTNode {
private:
    std::string name;

public:
    explicit IdentifierNode(std::string name) : name(std::move(name)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "IdentifierNode: " << name << "\n";
    }

    const std::string& getName() const {
        return name;
    }
};

// ============================================================
// BinaryOpNode：二元运算节点
// 例如：a + 1、b * 2、a < b
// ============================================================
class BinaryOpNode : public ASTNode {
private:
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

        if (left) {
            left->print(indent + 4);
        }
        if (right) {
            right->print(indent + 4);
        }
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

// ============================================================
// UnaryOpNode：一元运算节点
// 例如：-a、-5
// ============================================================
class UnaryOpNode : public ASTNode {
private:
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

// ============================================================
// VariableDeclNode：变量声明节点
// 例如：int a = 10;
// ============================================================
class VariableDeclNode : public ASTNode {
private:
    std::string type;                  // 变量类型，例如 int
    std::string name;                  // 变量名，例如 a
    std::unique_ptr<ASTNode> initExpr; // 初始化表达式

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

// ============================================================
// AssignmentNode：赋值语句节点
// 例如：a = a + 1;
// ============================================================
class AssignmentNode : public ASTNode {
private:
    std::string name;
    std::unique_ptr<ASTNode> value;

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

// ============================================================
// ExprStatementNode：表达式语句节点
// 当前主要用于支持函数调用语句，例如：foo();
// ============================================================
class ExprStatementNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> expr;

public:
    explicit ExprStatementNode(std::unique_ptr<ASTNode> e)
        : expr(std::move(e)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "ExprStatementNode\n";

        if (expr) {
            expr->print(indent + 4);
        }
    }

    const ASTNode* getExpr() const {
        return expr.get();
    }
};

// ============================================================
// ReturnNode：return 语句节点
// 例如：return a + 1;
// ============================================================
class ReturnNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> returnValue;

public:
    explicit ReturnNode(std::unique_ptr<ASTNode> expr)
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

// ============================================================
// IfNode：if / else 语句节点
// 例如：if (a < 10) { ... } else { ... }
// ============================================================
class IfNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBlock;
    std::unique_ptr<ASTNode> elseBlock;

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

// ============================================================
// WhileNode：while 循环节点
// 例如：while (a < 5) { ... }
// ============================================================
class WhileNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

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

// ============================================================
// BlockNode：代码块节点
// 例如：
// {
//     int a = 10;
//     return a;
// }
// ============================================================
class BlockNode : public ASTNode {
private:
    std::vector<std::unique_ptr<ASTNode>> statements;

public:
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

// ============================================================
// Parameter：函数参数结构体
// ============================================================
struct Parameter {
    std::string type;
    std::string name;
};

// ============================================================
// FunctionNode：函数定义节点
// 例如：int main() { ... }
// ============================================================
class FunctionNode : public ASTNode {
private:
    std::string returnType;
    std::string name;
    std::vector<Parameter> parameters;
    std::unique_ptr<ASTNode> body;

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
            std::cout << std::string(indent + 4, ' ')
                      << "Parameters:\n";
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

// ============================================================
// CallNode：函数调用节点
// 例如：add(3, 4)
// ============================================================
class CallNode : public ASTNode {
private:
    std::string calleeName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    CallNode(std::string name,
             std::vector<std::unique_ptr<ASTNode>> args)
        : calleeName(std::move(name)),
          arguments(std::move(args)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ')
                  << "CallNode: " << calleeName << "\n";

        if (!arguments.empty()) {
            std::cout << std::string(indent + 4, ' ')
                      << "Arguments:\n";
            for (const auto& arg : arguments) {
                arg->print(indent + 8);
            }
        }
    }

    const std::string& getCalleeName() const {
        return calleeName;
    }

    const std::vector<std::unique_ptr<ASTNode>>& getArguments() const {
        return arguments;
    }
};

#endif