#include "codegen.h"

#include <stdexcept>
#include <vector>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

// ============================================================
// 构造函数：初始化 LLVM Module 和 IRBuilder
// ============================================================
CodeGenerator::CodeGenerator()
    : module_(std::make_unique<llvm::Module>("my_compiler_module", context_)),
      builder_(context_),
      currentFunction_(nullptr) {}

// ============================================================
// enterScope：进入一个新的变量作用域
// ============================================================
void CodeGenerator::enterScope() {
    scopes_.push_back({});
}

// ============================================================
// exitScope：退出当前变量作用域
// ============================================================
void CodeGenerator::exitScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

// ============================================================
// bindVariable：在当前作用域里绑定变量名和它的栈槽地址
// ============================================================
void CodeGenerator::bindVariable(const std::string& name,
                                 llvm::AllocaInst* allocaInst) {
    if (scopes_.empty()) {
        enterScope();
    }

    scopes_.back()[name] = allocaInst;
}

// ============================================================
// lookupVariable：从内层作用域向外查找变量对应的栈槽地址
// ============================================================
llvm::AllocaInst* CodeGenerator::lookupVariable(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    return nullptr;
}

// ============================================================
// createEntryBlockAlloca：在函数入口基本块里创建一个 int 变量栈槽
//
// 这么做的好处是：所有局部变量都集中在 entry block 中分配，
// 更符合 LLVM 前端的常见写法，后续优化也更友好。
// ============================================================
llvm::AllocaInst* CodeGenerator::createEntryBlockAlloca(
    llvm::Function* function,
    const std::string& name) {

    llvm::IRBuilder<> tempBuilder(
        &function->getEntryBlock(),
        function->getEntryBlock().begin()  //
    );

    return tempBuilder.CreateAlloca(
        llvm::Type::getInt32Ty(context_),
        nullptr,
        name
    );
}

// ============================================================
// genExpr：生成表达式对应的 LLVM IR
//
// 统一约定：
// - 所有表达式最终都返回 i32
// - 比较表达式先得到 i1，再扩展成 i32(0 或 1)
// ============================================================
llvm::Value* CodeGenerator::genExpr(const ASTNode* node) {
    if (!node) {
        throw std::runtime_error("CodeGen Error: null expression node");
    }

    // -------- 数字字面量 --------
    if (auto numberNode = dynamic_cast<const NumberNode*>(node)) {
        int value = std::stoi(numberNode->getValue());
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(context_),
            value,
            true
        );
    }

    // -------- 标识符（变量读取）--------
    if (auto idNode = dynamic_cast<const IdentifierNode*>(node)) {
        llvm::AllocaInst* variable = lookupVariable(idNode->getName());
        if (!variable) {
            throw std::runtime_error(
                "CodeGen Error: unknown variable '" + idNode->getName() + "'"
            );
        }

        return builder_.CreateLoad(
            llvm::Type::getInt32Ty(context_),
            variable,
            idNode->getName()
        );
    }

    // -------- 一元运算 --------
    if (auto unaryNode = dynamic_cast<const UnaryOpNode*>(node)) {
        llvm::Value* operand = genExpr(unaryNode->getOperand());

        if (unaryNode->getOp() == "-") {
            return builder_.CreateNeg(operand, "negtmp");
        }

        throw std::runtime_error(
            "CodeGen Error: unsupported unary operator '" + unaryNode->getOp() + "'"
        );
    }

    // -------- 二元运算 --------
    if (auto binNode = dynamic_cast<const BinaryOpNode*>(node)) {
        llvm::Value* left = genExpr(binNode->getLeft());
        llvm::Value* right = genExpr(binNode->getRight());
        const std::string& op = binNode->getOp();

        // 算术运算
        if (op == "+") {
            return builder_.CreateAdd(left, right, "addtmp");
        }
        if (op == "-") {
            return builder_.CreateSub(left, right, "subtmp");
        }
        if (op == "*") {
            return builder_.CreateMul(left, right, "multmp");
        }
        if (op == "/") {
            return builder_.CreateSDiv(left, right, "divtmp");
        }

        // 比较运算：先生成 i1，再转成 i32
        llvm::Value* cmp = nullptr;

        if (op == "<") {
            cmp = builder_.CreateICmpSLT(left, right, "cmptmp");
        } else if (op == ">") {
            cmp = builder_.CreateICmpSGT(left, right, "cmptmp");
        } else if (op == "<=") {
            cmp = builder_.CreateICmpSLE(left, right, "cmptmp");
        } else if (op == ">=") {
            cmp = builder_.CreateICmpSGE(left, right, "cmptmp");
        } else if (op == "==") {
            cmp = builder_.CreateICmpEQ(left, right, "cmptmp");
        } else if (op == "!=") {
            cmp = builder_.CreateICmpNE(left, right, "cmptmp");
        }

        if (cmp) {
            return builder_.CreateZExt(
                cmp,
                llvm::Type::getInt32Ty(context_),
                "booltmp"
            );
        }

        throw std::runtime_error(
            "CodeGen Error: unsupported binary operator '" + op + "'"
        );
    }

    // -------- 函数调用 --------
    if (auto callNode = dynamic_cast<const CallNode*>(node)) {
        llvm::Function* calleeFunc = module_->getFunction(callNode->getCalleeName());
        if (!calleeFunc) {
            throw std::runtime_error(
                "CodeGen Error: unknown function '" + callNode->getCalleeName() + "'"
            );
        }

        if (calleeFunc->arg_size() != callNode->getArguments().size()) {
            throw std::runtime_error(
                "CodeGen Error: wrong number of arguments when calling '" +
                callNode->getCalleeName() + "'"
            );
        }

        std::vector<llvm::Value*> args;
        for (const auto& arg : callNode->getArguments()) {
            args.push_back(genExpr(arg.get()));
        }

        return builder_.CreateCall(calleeFunc, args, "calltmp");
    }

    throw std::runtime_error("CodeGen Error: unsupported expression node");
}

// ============================================================
// genStmt：生成语句对应的 LLVM IR
// ============================================================
void CodeGenerator::genStmt(const ASTNode* node) {
    if (!node) {
        return;
    }

    // -------- 变量声明 --------
    if (auto varDecl = dynamic_cast<const VariableDeclNode*>(node)) {
        if (!currentFunction_) {
            throw std::runtime_error("CodeGen Error: variable declaration outside function");
        }

        // 先生成初始化表达式
        // 注意：这里先生成右侧，再把变量绑定到当前作用域，
        // 这样和语义分析规则保持一致。
        llvm::Value* initValue = genExpr(varDecl->getInitExpr());

        llvm::AllocaInst* allocaInst =
            createEntryBlockAlloca(currentFunction_, varDecl->getName());

        builder_.CreateStore(initValue, allocaInst);
        bindVariable(varDecl->getName(), allocaInst);
        return;
    }

    // -------- 赋值语句 --------
    if (auto assignNode = dynamic_cast<const AssignmentNode*>(node)) {
        llvm::AllocaInst* variable = lookupVariable(assignNode->getName());
        if (!variable) {
            throw std::runtime_error(
                "CodeGen Error: assignment to unknown variable '" + assignNode->getName() + "'"
            );
        }

        llvm::Value* value = genExpr(assignNode->getValue());
        builder_.CreateStore(value, variable);
        return;
    }

    // -------- 表达式语句 --------
    if (auto exprStmt = dynamic_cast<const ExprStatementNode*>(node)) {
        genExpr(exprStmt->getExpr());
        return;
    }

    // -------- return --------
    if (auto returnNode = dynamic_cast<const ReturnNode*>(node)) {
        llvm::Value* returnValue = genExpr(returnNode->getReturnValue());
        builder_.CreateRet(returnValue);
        return;
    }

    // -------- 代码块 --------
    if (auto blockNode = dynamic_cast<const BlockNode*>(node)) {
        enterScope();

        for (const auto& stmt : blockNode->getStatements()) {
            llvm::BasicBlock* currentBB = builder_.GetInsertBlock();

            // 防御性检查：如果当前没有合法插入块，直接报错
            if (!currentBB) {
                exitScope();
                throw std::runtime_error("CodeGen Error: no valid insert block when generating BlockNode");
            }

            // 如果当前基本块已经结束（比如已经 ret / br），
            // 后续语句不应继续生成
            if (currentBB->getTerminator() != nullptr) {
                break;
            }

            genStmt(stmt.get());
        }

        exitScope();
        return;
    }

    // -------- if / else --------
    if (auto ifNode = dynamic_cast<const IfNode*>(node)) {
        llvm::BasicBlock* currentBB = builder_.GetInsertBlock();
        if (!currentBB) {
            throw std::runtime_error("CodeGen Error: no valid insert block before IfNode");
        }

        llvm::Function* function = currentBB->getParent();
        if (!function) {
            throw std::runtime_error("CodeGen Error: IfNode is not inside a valid function");
        }

        // 1) 生成条件表达式
        llvm::Value* condValue = genExpr(ifNode->getCondition());

        // 2) 统一按“非 0 为真”处理条件
        condValue = builder_.CreateICmpNE(
            condValue,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
            "ifcond"
        );

        // 3) 创建基本块
        llvm::BasicBlock* thenBB  = llvm::BasicBlock::Create(context_, "then", function);
        llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context_, "ifend", function);
        llvm::BasicBlock* elseBB  = nullptr;

        if (ifNode->getElseBlock()) {
            elseBB = llvm::BasicBlock::Create(context_, "else", function);
            builder_.CreateCondBr(condValue, thenBB, elseBB);
        } else {
            builder_.CreateCondBr(condValue, thenBB, mergeBB);
        }

        // 4) 生成 then 分支
        builder_.SetInsertPoint(thenBB);
        genStmt(ifNode->getThenBlock());

        llvm::BasicBlock* thenEndBB = builder_.GetInsertBlock();
        if (thenEndBB && thenEndBB->getTerminator() == nullptr) {
            builder_.CreateBr(mergeBB);
        }

        // 5) 生成 else 分支（如果存在）
        if (ifNode->getElseBlock()) {
            builder_.SetInsertPoint(elseBB);
            genStmt(ifNode->getElseBlock());

            llvm::BasicBlock* elseEndBB = builder_.GetInsertBlock();
            if (elseEndBB && elseEndBB->getTerminator() == nullptr) {
                builder_.CreateBr(mergeBB);
            }
        }

        // 6) 后续代码继续往 mergeBB 里写
        builder_.SetInsertPoint(mergeBB);
        return;
    }

    // -------- while --------
    if (auto whileNode = dynamic_cast<const WhileNode*>(node)) {
        llvm::BasicBlock* currentBB = builder_.GetInsertBlock();
        if (!currentBB) {
            throw std::runtime_error("CodeGen Error: no valid insert block before WhileNode");
        }

        llvm::Function* function = currentBB->getParent();
        if (!function) {
            throw std::runtime_error("CodeGen Error: WhileNode is not inside a valid function");
        }

        // 创建循环相关的基本块
        llvm::BasicBlock* condBB = llvm::BasicBlock::Create(context_, "while.cond", function);
        llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(context_, "while.body", function);
        llvm::BasicBlock* endBB  = llvm::BasicBlock::Create(context_, "while.end", function);

        // 如果当前块还没结束，先跳到条件块
        if (currentBB->getTerminator() == nullptr) {
            builder_.CreateBr(condBB);
        }

        // 1) 生成条件判断块
        builder_.SetInsertPoint(condBB);
        llvm::Value* condValue = genExpr(whileNode->getCondition());
        condValue = builder_.CreateICmpNE(
            condValue,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
            "whilecond"
        );
        builder_.CreateCondBr(condValue, bodyBB, endBB);

        // 2) 生成循环体
        builder_.SetInsertPoint(bodyBB);
        genStmt(whileNode->getBody());

        llvm::BasicBlock* bodyEndBB = builder_.GetInsertBlock();
        if (bodyEndBB && bodyEndBB->getTerminator() == nullptr) {
            builder_.CreateBr(condBB);
        }

        // 3) 后续代码继续写到 endBB
        builder_.SetInsertPoint(endBB);
        return;
    }

    // -------- 直接出现函数定义，不在 genStmt 里处理 --------
    if (dynamic_cast<const FunctionNode*>(node)) {
        throw std::runtime_error("CodeGen Error: function node should not be handled by genStmt");
    }

    throw std::runtime_error("CodeGen Error: unsupported statement node");
}

// ============================================================
// genFunction：为一个函数定义生成 LLVM IR
// ============================================================
llvm::Function* CodeGenerator::genFunction(const FunctionNode* node) {
    if (!node) {
        return nullptr;
    }

    // 当前阶段只支持 int 返回值、int 参数
    std::vector<llvm::Type*> argTypes(
        node->getParameters().size(),
        llvm::Type::getInt32Ty(context_)
    );

    llvm::FunctionType* functionType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context_),
        argTypes,
        false
    );

    llvm::Function* function = llvm::Function::Create(
        functionType,
        llvm::Function::ExternalLinkage,
        node->getName(),
        module_.get()
    );

    // 给函数参数命名
    size_t index = 0;
    for (auto& arg : function->args()) {
        arg.setName(node->getParameters()[index].name);
        ++index;
    }

    // 创建入口基本块
    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(context_, "entry", function);
    builder_.SetInsertPoint(entryBB);

    currentFunction_ = function;

    // 进入函数级作用域
    enterScope();

    // 把所有形参保存到栈槽里，后续统一按变量方式读取
    for (auto& arg : function->args()) {
        llvm::AllocaInst* allocaInst =
            createEntryBlockAlloca(function, std::string(arg.getName()));

        builder_.CreateStore(&arg, allocaInst);
        bindVariable(std::string(arg.getName()), allocaInst);
    }

    // 生成函数体
    genStmt(node->getBody());

    // 如果函数末尾没有显式 return，这里补一个默认 return 0
    // 这样第一版更容易跑通，后面你也可以升级成“无 return 报错”
    if (builder_.GetInsertBlock()->getTerminator() == nullptr) {
        builder_.CreateRet(
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0)
        );
    }

    // 退出函数作用域
    exitScope();
    currentFunction_ = nullptr;

    // 验证函数是否是合法的 LLVM IR
    if (llvm::verifyFunction(*function, &llvm::errs())) {
        function->eraseFromParent();
        throw std::runtime_error(
            "CodeGen Error: generated invalid LLVM IR for function '" +
            node->getName() + "'"
        );
    }

    return function;
}

// ============================================================
// generate：生成整棵 AST 对应的 LLVM IR
//
// 顶层 root 目前应当是一个 BlockNode，里面装的是所有 FunctionNode
// ============================================================
bool CodeGenerator::generate(const ASTNode* root) {
    if (!root) {
        return false;
    }

    auto program = dynamic_cast<const BlockNode*>(root);
    if (!program) {
        throw std::runtime_error("CodeGen Error: program root must be a BlockNode");
    }

    // 依次生成每个顶层函数
    for (const auto& stmt : program->getStatements()) {
        auto funcNode = dynamic_cast<const FunctionNode*>(stmt.get());
        if (!funcNode) {
            throw std::runtime_error("CodeGen Error: only function definitions are allowed at top level");
        }

        genFunction(funcNode);
    }

    // 整个模块也做一次验证
    if (llvm::verifyModule(*module_, &llvm::errs())) {
        throw std::runtime_error("CodeGen Error: generated invalid LLVM module");
    }

    return true;
}

// ============================================================
// printIR：把 LLVM IR 打印到终端
// ============================================================
void CodeGenerator::printIR() const {
    module_->print(llvm::outs(), nullptr);
}

// ============================================================
// dumpIR：把 LLVM IR 写入文件
// ============================================================
bool CodeGenerator::dumpIR(const std::string& outputPath) const {
    std::error_code ec;
    llvm::raw_fd_ostream outFile(outputPath, ec, llvm::sys::fs::OF_None);

    if (ec) {
        return false;
    }

    module_->print(outFile, nullptr);
    return true;
}