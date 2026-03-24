#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

// ============================================================
// CodeGenerator：把 AST 翻译成 LLVM IR
//
// 当前阶段支持：
// 1. int 函数
// 2. int 局部变量
// 3. 算术表达式
// 4. 比较表达式
// 5. 赋值
// 6. return
// 7. 函数调用
// 8. if / else
// 9. while
// ============================================================
class CodeGenerator {
public:
    CodeGenerator();

    // 生成整棵 AST 对应的 LLVM IR
    bool generate(const ASTNode* root);

    // 打印 LLVM IR 到终端
    void printIR() const;

    // 把 LLVM IR 写入文件
    bool dumpIR(const std::string& outputPath) const;

private:
    llvm::LLVMContext context_;
    std::unique_ptr<llvm::Module> module_;
    llvm::IRBuilder<> builder_;

    // 当前正在生成的函数
    llvm::Function* currentFunction_;

    // 变量作用域栈：
    // 每层作用域记录 变量名 -> 对应的 alloca 地址
    std::vector<std::unordered_map<std::string, llvm::AllocaInst*>> scopes_;

    // ---------- 表达式 / 语句 / 函数 ----------
    llvm::Value* genExpr(const ASTNode* node);
    void genStmt(const ASTNode* node);
    llvm::Function* genFunction(const FunctionNode* node);

    // 在函数 entry block 中创建局部变量栈槽
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function,
                                             const std::string& name);

    // ---------- 作用域管理 ----------
    void enterScope();
    void exitScope();
    void bindVariable(const std::string& name, llvm::AllocaInst* allocaInst);
    llvm::AllocaInst* lookupVariable(const std::string& name) const;
};

#endif