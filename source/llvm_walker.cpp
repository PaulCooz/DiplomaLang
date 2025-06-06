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
#include <map>

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
  std::map<std::string, AllocaInst*> localScope;
  std::map<std::string, Function*> funcScope;

  Function* mainFunc;
  BasicBlock* mainBlock;

  Function* printfFunc;

  std::map<std::string, GlobalVariable*> printFormats;

public:
  InterpreterWalker() {
    llvmContext = new LLVMContext();
    irModule = new Module("my module", *llvmContext);
    irBuilder = new IRBuilder<>(*llvmContext);

    currBlock = nullptr;

    auto printfSign = FunctionType::get(irBuilder->getInt32Ty(), PointerType::get(irBuilder->getInt8Ty(), 0), true);
    printfFunc = Function::Create(printfSign, Function::ExternalLinkage, "printf", irModule);

    auto mainSign = FunctionType::get(irBuilder->getInt32Ty(), false);
    mainFunc = Function::Create(mainSign, Function::ExternalLinkage, "main", irModule);

    mainBlock = BasicBlock::Create(*llvmContext, "entry", mainFunc);
    irBuilder->SetInsertPoint(mainBlock);
  }

  void Do(std::vector<Expr*> syntax) {
    for (auto expr : syntax) {
      expr->visit(this);
    }
  }

  ~InterpreterWalker() {
    irBuilder->CreateRet(irBuilder->getInt32(0));

    if (verifyFunction(*mainFunc, &errs())) {
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
      if (isa<Function>(value)) {
        auto func = dyn_cast<Function>(value);
        auto funcPtrType = func->getType();

        newVar = new GlobalVariable(
          *irModule, funcPtrType, false, GlobalValue::ExternalLinkage, ConstantPointerNull::get(funcPtrType), name
        );
        funcScope[name] = func;
      } else {
        newVar = new GlobalVariable(
          *irModule, valueType, false, GlobalValue::ExternalLinkage, Constant::getNullValue(valueType), name
        );
      }
    } else {
      auto a = irBuilder->CreateAlloca(valueType, nullptr, name);
      newVar = a;
      localScope[name] = a;
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
      if (isa<Function>(newValue)) {
        funcScope[name] = dyn_cast<Function>(newValue);
      }
    } else {
      val = localScope[name];
    }
    irBuilder->CreateStore(newValue, val);
    return newValue;
  }

  std::any visitVar(VarExpr* varExpr) {
    auto name = varExpr->identifier.value;
    auto varPtr = (Value*)nullptr;
    auto varType = (Type*)nullptr;
    if (currBlock == nullptr) {
      auto gv = irModule->getGlobalVariable(name);
      varPtr = gv;
      varType = gv->getValueType();
    } else {
      auto lv = localScope[name];
      varPtr = lv;
      varType = lv->getAllocatedType();
    }
    return (Value*)irBuilder->CreateLoad(varType, varPtr, name);
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

  std::any visitLogical(LogicalExpr* logicalExpr) {
    auto oper = logicalExpr->oper.grapheme;
    auto currFunc = irBuilder->GetInsertBlock()->getParent();

    auto leftName = oper == OR ? "orLeft" : "andLeft";
    auto rightName = oper == OR ? "orRight" : "andRight";
    auto endName = oper == OR ? "endOr" : "endAnd";
    auto resName = oper == OR ? "orRes" : "andRes";

    auto leftBlock = BasicBlock::Create(irBuilder->getContext(), leftName, currFunc);
    auto rightBlock = BasicBlock::Create(irBuilder->getContext(), rightName, currFunc);
    auto endBlock = BasicBlock::Create(irBuilder->getContext(), endName, currFunc);

    irBuilder->CreateBr(leftBlock); // enter

    irBuilder->SetInsertPoint(leftBlock);
    auto left = std::any_cast<Value*>(logicalExpr->left->visit(this));
    if (oper == OR) {
      irBuilder->CreateCondBr(left, endBlock, rightBlock);
    } else {
      irBuilder->CreateCondBr(left, rightBlock, endBlock);
    }

    irBuilder->SetInsertPoint(rightBlock);
    auto right = std::any_cast<Value*>(logicalExpr->right->visit(this));
    irBuilder->CreateBr(endBlock);

    irBuilder->SetInsertPoint(endBlock);
    auto res = irBuilder->CreatePHI(irBuilder->getInt1Ty(), 2, resName);
    res->addIncoming(left, leftBlock);
    res->addIncoming(right, rightBlock);

    return (Value*)res;
  }

  std::any visitIfElse(IfElseExpr* ifElseExpr) {
    auto condition = std::any_cast<Value*>(ifElseExpr->condition->visit(this));
    auto currFunc = irBuilder->GetInsertBlock()->getParent();

    auto thenBlock = BasicBlock::Create(irBuilder->getContext(), "then", currFunc);
    auto elseBlock = BasicBlock::Create(irBuilder->getContext(), "else", currFunc);
    auto endifBlock = BasicBlock::Create(irBuilder->getContext(), "endIf", currFunc);

    irBuilder->CreateCondBr(condition, thenBlock, elseBlock);

    irBuilder->SetInsertPoint(thenBlock);
    ifElseExpr->thenBlock->visit(this);
    irBuilder->CreateBr(endifBlock);

    irBuilder->SetInsertPoint(elseBlock);
    ifElseExpr->elseBlock->visit(this);
    irBuilder->CreateBr(endifBlock);

    irBuilder->SetInsertPoint(endifBlock);

    return (Value*)nullptr;
  }

  std::any visitBlock(BlockExpr* blockExpr) {
    auto lastValue = (Value*)nullptr;
    for (auto expr : blockExpr->list) {
      lastValue = std::any_cast<Value*>(expr->visit(this));
    }
    return lastValue;
  }

  std::any visitFunc(FuncExpr* funcExpr) {
    std::vector<Type*> paramTypes;
    for (auto type : funcExpr->argsTypes) {
      paramTypes.emplace_back(ExprToLLVMType(type));
    }

    auto funcSign = FunctionType::get(ExprToLLVMType(funcExpr->retType), paramTypes, false);
    auto function = Function::Create(funcSign, Function::ExternalLinkage, "", *irModule);

    currBlock = BasicBlock::Create(irBuilder->getContext(), "entry", function);
    irBuilder->SetInsertPoint(currBlock);

    for (auto i = 0; i < funcExpr->args.size(); i++) {
      auto arg = function->getArg(i);

      auto name = funcExpr->args[i].value;
      arg->setName(name);

      auto alloca = irBuilder->CreateAlloca(arg->getType(), nullptr, name);
      irBuilder->CreateStore(arg, alloca);
      localScope[name] = alloca;
    }

    auto ret = std::any_cast<Value*>(funcExpr->body->visit(this));
    irBuilder->CreateRet(ret);

    if (verifyFunction(*function, &errs())) {
      errs() << "Error verifying function!\n";
    }

    currBlock = nullptr;
    irBuilder->SetInsertPoint(mainBlock);
    localScope.clear();

    return (Value*)function;
  }

  std::any visitCall(CallExpr* callExpr) {
    std::vector<Value*> args;
    for (auto a : callExpr->args) {
      args.emplace_back(std::any_cast<Value*>(a->visit(this)));
    }

    auto func = std::any_cast<Value*>(callExpr->func->visit(this));
    if (isa<Function>(func)) {
      return (Value*)irBuilder->CreateCall(cast<Function>(func), args);
    } else {
      auto funcLoad = cast<LoadInst>(func)->getPointerOperand();

      if (isa<GlobalVariable>(funcLoad)) {
        auto name = cast<GlobalVariable>(funcLoad)->getName().str(); // TODO !
        return (Value*)irBuilder->CreateCall(funcScope[name], args);
      }

      std::vector<Type*> paramTypes;
      for (auto a : callExpr->args) {
        paramTypes.emplace_back(ExprToLLVMType(a->type));
      }
      auto funcSign = FunctionType::get(ExprToLLVMType(callExpr->type), paramTypes, false); // TODO !
      return (Value*)irBuilder->CreateCall(funcSign, func, args);
    }
  }

  std::any visitPrintln(PrintlnExpr* printlnExpr) {
    std::string format = "";
    std::vector<Value*> args;
    for (auto i = 0; i < printlnExpr->values.size(); i++) {
      auto v = printlnExpr->values[i];
      auto value = std::any_cast<Value*>(v->visit(this));
      switch (v->type) {
      case BOOL:
        format += "%i";
        value = irBuilder->CreateZExt(value, irBuilder->getInt32Ty());
        break;
      case VOID:
      case I32:
        format += "%i";
        break;
      case R64:
        format += "%f";
        break;
      case STR:
        format += "%s";
        break;
      case FUNC:
        format += "%i";
        break;
      }
      if (i != printlnExpr->values.size() - 1)
        format += ", ";
      args.emplace_back(value);
    }
    format += '\n';

    if (printFormats.find(format) == printFormats.end())
      printFormats[format] = irBuilder->CreateGlobalString(format);
    args.emplace(args.begin(), printFormats[format]);

    return (Value*)(irBuilder->CreateCall(printfFunc, args));
  }

private:
  Type* ExprToLLVMType(ExprType type) {
    switch (type) {
    case VOID:
      return irBuilder->getVoidTy();
    case BOOL:
      return irBuilder->getInt1Ty();
    case I32:
      return irBuilder->getInt32Ty();
    case R64:
      return irBuilder->getDoubleTy();
    case STR:
      return PointerType::get(irBuilder->getInt8Ty(), 0);
    case FUNC:
      return irBuilder->getPtrTy(); // TODO FuncType
    }
  }
};

} // namespace Diploma
