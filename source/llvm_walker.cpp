#include "syntax_tree.hpp"
#include <functional>
#include <iostream>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/SandboxIR/Utils.h>
#include <llvm/SandboxIR/Value.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>

using namespace llvm;

template <typename T> void dump(T* v) {
  v->print(outs(), true);
  outs() << '\n';
}

namespace Diploma {

class InterpreterWalker : public TreeWalker {
private:
  LLVMContext* llvmContext;
  Module* irModule;
  IRBuilder<>* irBuilder;

  BasicBlock* currBlock;

  llvm::Function* MainFn;
  Function* PrintfFn;

public:
  InterpreterWalker() {
    llvmContext = new LLVMContext();
    irModule = new Module("my module", *llvmContext);
    irBuilder = new IRBuilder<>(*llvmContext);

    currBlock = nullptr;

    auto PrintfTy = FunctionType::get(
      irBuilder->getInt32Ty(),                     // Return type (int)
      PointerType::get(irBuilder->getInt8Ty(), 0), // Format string (char*)
      true                                         // Variadic function
    );
    PrintfFn = Function::Create(PrintfTy, Function::ExternalLinkage, "printf", irModule);

    auto MainFnTy = FunctionType::get(
      irBuilder->getInt32Ty(),                     // Return type
      false                                        // Not variadic
    );
    MainFn = Function::Create(MainFnTy, Function::ExternalLinkage, "main", irModule);

    auto EntryBB = BasicBlock::Create(*llvmContext, "entry", MainFn);
    irBuilder->SetInsertPoint(EntryBB);
  }

  void Do(std::vector<Expr*> syntax) {
    for (auto expr : syntax) {
      expr->visit(this);
    }
  }

  ~InterpreterWalker() {
    irBuilder->CreateRet(irBuilder->getInt32(0));

    if (verifyFunction(*MainFn, &errs())) {
      errs() << "Error verifying function!\n";
    }

    std::error_code EC;
    ToolOutputFile out("output.ir", EC, sys::fs::OF_None);
    if (EC) {
      std::cout << EC.message() << std::endl;
    }
    out.keep();
    irModule->print(out.os(), nullptr);

    delete irBuilder;
    delete irModule;
    delete llvmContext;
  }

  std::any visitBool(BoolExpr* boolExpr) {
    return (Value*)(boolExpr->value ? irBuilder->getTrue() : irBuilder->getFalse());
  }

  std::any visitInt32(Int32Expr* int32Expr) {
    return (Value*)(irBuilder->getInt32(int32Expr->value));
  }

  std::any visitReal64(Real64Expr* real64Expr) {
    return (Value*)(ConstantFP::get(irBuilder->getDoubleTy(), real64Expr->value));
  }

  std::any visitStr(StrExpr* strExpr) {
    return (Value*)(irBuilder->CreateGlobalString(strExpr->value));
  }

  std::any visitNewVar(NewVarExpr* newVarExpr) {
    auto value = std::any_cast<Value*>(newVarExpr->value->visit(this));
    auto valueType = value->getType();
    auto name = newVarExpr->identifier.value;
    auto newVar = (Value*)nullptr;
    if (currBlock == nullptr) {
      newVar = new GlobalVariable(
        *irModule, valueType, false, GlobalValue::ExternalLinkage, Constant::getNullValue(valueType), name
      );
    } else {
      newVar = (Value*)irBuilder->CreateAlloca(valueType, nullptr, name);
    }
    irBuilder->CreateStore(value, newVar);
    return newVar;
  }

  std::any visitVarAssign(VarAssignExpr* varAssignExpr) {
    auto name = varAssignExpr->identifier.value;
    auto newValue = std::any_cast<Value*>(varAssignExpr->value->visit(this));
    auto val = (Value*)nullptr;
    if (currBlock == nullptr) {
      val = irModule->getGlobalVariable(name);
    } else {
      val = currBlock->getValueSymbolTable()->lookup(name);
    }
    irBuilder->CreateStore(newValue, val);
    return newValue;
  }

  std::any visitVar(VarExpr* varExpr) {
    auto name = varExpr->identifier.value;
    if (currBlock == nullptr) {
      auto gVar = irModule->getGlobalVariable(name);
      auto type = gVar->getValueType();
      return (Value*)irBuilder->CreateLoad(type, gVar, name);
    } else {
      auto lVar = currBlock->getValueSymbolTable()->lookup(name);
      return (Value*)irBuilder->CreateLoad(lVar->getType(), lVar, name);
    }
  }

  std::any visitUnary(UnaryExpr* unaryExpr) {
    auto value = std::any_cast<Value*>(unaryExpr->value->visit(this));
    if (unaryExpr->oper.grapheme == PLUS) {
      return value;
    } else if (unaryExpr->oper.grapheme == MINUS) {
      if (value->getType()->isFloatingPointTy())
        return (Value*)irBuilder->CreateFNeg(value);
      else
        return (Value*)irBuilder->CreateNeg(value);
    } else {
      return nullptr;
    }
  }

  Value* createBinOperation(
    Value* left, Value* right, std::function<Value*(Value*, Value*)> binOperI,
    std::function<Value*(Value*, Value*)> binOperF
  ) {
    auto leftType = left->getType();
    auto rightType = right->getType();
    auto leftIsFloating = leftType->isFloatingPointTy();
    auto rightIsFloating = rightType->isFloatingPointTy();

    auto binOper = binOperI;
    if (leftIsFloating || rightIsFloating) {
      if (!leftIsFloating) {
        left = irBuilder->CreateSIToFP(left, rightType);
      } else if (!rightIsFloating) {
        right = irBuilder->CreateSIToFP(right, leftType);
      }
      binOper = binOperF;
    }

    return binOper(left, right);
  }

  std::any visitBinary(BinaryExpr* binaryExpr) {
    auto left = std::any_cast<Value*>(binaryExpr->left->visit(this));
    auto right = std::any_cast<Value*>(binaryExpr->right->visit(this));
    if (binaryExpr->oper.grapheme == STAR) {
      return createBinOperation(
        left,
        right,
        [&](Value* l, Value* r) { return irBuilder->CreateMul(l, r); },
        [&](Value* l, Value* r) { return irBuilder->CreateFMul(l, r); }
      );
    } else if (binaryExpr->oper.grapheme == SLASH) {
      return createBinOperation(
        left,
        right,
        [&](Value* l, Value* r) { return irBuilder->CreateSDiv(l, r); },
        [&](Value* l, Value* r) { return irBuilder->CreateFDiv(l, r); }
      );
    } else if (binaryExpr->oper.grapheme == PLUS) {
      return createBinOperation(
        left,
        right,
        [&](Value* l, Value* r) { return irBuilder->CreateAdd(l, r); },
        [&](Value* l, Value* r) { return irBuilder->CreateFAdd(l, r); }
      );
    } else if (binaryExpr->oper.grapheme == MINUS) {
      return createBinOperation(
        left,
        right,
        [&](Value* l, Value* r) { return irBuilder->CreateSub(l, r); },
        [&](Value* l, Value* r) { return irBuilder->CreateFSub(l, r); }
      );
    }
    return nullptr;
  }

  std::any visitBlock(BlockExpr* BlockExpr) {}
  std::any visitFunc(FuncExpr* FuncExpr) {} // and remember main func ☝️🤓
  std::any visitCall(CallExpr* CallExpr) {}

  std::any visitPrintln(PrintlnExpr* printlnExpr) {
    auto format = std::any_cast<Value*>(printlnExpr->format->visit(this));
    auto output = std::any_cast<Value*>(printlnExpr->value->visit(this));
    return (Value*)(irBuilder->CreateCall(PrintfFn, {format, output})); // TODO \n
  }
};

} // namespace Diploma
