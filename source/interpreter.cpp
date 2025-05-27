#include "interpreter.hpp"
#include <iostream>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/SandboxIR/Utils.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>

using namespace llvm;

namespace Diploma {

LLVMContext* llvmContext;
Module* irModule;
IRBuilder<>* irBuilder;

BasicBlock* currBlock;

Function* PrintfFn;

Interpreter::Interpreter(std::vector<Expr*> syntax) {
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
  auto MainFn = Function::Create(MainFnTy, Function::ExternalLinkage, "main", irModule);

  auto EntryBB = BasicBlock::Create(*llvmContext, "entry", MainFn);
  irBuilder->SetInsertPoint(EntryBB);

  for (auto expr : syntax) {
    expr->visit(this);
  }

  irBuilder->CreateRet(irBuilder->getInt32(0));

  if (verifyFunction(*MainFn, &errs())) {
    errs() << "Error verifying function!\n";
  }
}

Interpreter::~Interpreter() {
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

std::any Interpreter::visitBool(BoolExpr* boolExpr) {
  return (Value*)(boolExpr->value ? irBuilder->getTrue() : irBuilder->getFalse());
}

std::any Interpreter::visitInt32(Int32Expr* int32Expr) {
  return (Value*)(irBuilder->getInt32(int32Expr->value));
}

std::any Interpreter::visitReal32(Real32Expr* real32Expr) {
  return (Value*)(ConstantFP::get(irBuilder->getFloatTy(), real32Expr->value));
}

std::any Interpreter::visitStr(StrExpr* strExpr) {
  return (Value*)(irBuilder->CreateGlobalString(strExpr->value));
}

std::any Interpreter::visitNewVar(NewVarExpr* newVarExpr) {
  auto value = std::any_cast<Value*>(newVarExpr->value->visit(this));
  auto name = newVarExpr->identifier.value;
  if (currBlock == nullptr) {
    auto gVar =
      new GlobalVariable(*irModule, value->getType(), false, GlobalValue::ExternalLinkage, (Constant*)value, name);
    return (Value*)gVar;
  } else {
    auto lVar = (Value*)irBuilder->CreateAlloca(value->getType(), nullptr, name);
    irBuilder->CreateStore(value, lVar);
    return (Value*)lVar;
  }
}

std::any Interpreter::visitVarAssign(VarAssignExpr* VarAssignExpr) {}

std::any Interpreter::visitVar(VarExpr* varExpr) {
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

std::any Interpreter::visitUnary(UnaryExpr* UnaryExpr) {}

std::any Interpreter::visitBinary(BinaryExpr* binaryExpr) {
  auto left = std::any_cast<Value*>(binaryExpr->left->visit(this));
  auto right = std::any_cast<Value*>(binaryExpr->right->visit(this));
  if (binaryExpr->oper.grapheme == STAR) {
    return (Value*)(irBuilder->CreateMul(left, right));
  } else if (binaryExpr->oper.grapheme == PLUS) {
    return (Value*)(irBuilder->CreateAdd(left, right));
  }
  return nullptr;
}

std::any Interpreter::visitBlock(BlockExpr* BlockExpr) {}
std::any Interpreter::visitFunc(FuncExpr* FuncExpr) {}
std::any Interpreter::visitCall(CallExpr* CallExpr) {}

std::any Interpreter::visitPrint(PrintExpr* printExpr) {
  auto format = std::any_cast<Value*>(printExpr->format->visit(this));
  auto output = std::any_cast<Value*>(printExpr->value->visit(this));
  return (Value*)(irBuilder->CreateCall(PrintfFn, {format, output}));
}

} // namespace Diploma
