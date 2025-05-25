#include "abstract_syntax_tree.hpp"
#include <format>
#include <functional>
#include <iostream>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <map>

using namespace llvm;

LLVMContext* context;
Module* module;
IRBuilder<>* builder;

std::map<std::string, ExprValue> variables;

void startAST() {
  context = new LLVMContext();
  module = new Module("my cool module", *context);
  builder = new IRBuilder<>(*context);
}

void finishAST() {
  module->print(outs(), nullptr);
}

ExprValue PrimaryExpr::evaluate() {
  return result;
}

ExprValue VarExpr::evaluate() {
  auto res = variables.find(identifier.value);
  if (res == variables.end()) {
    std::cout << "not nice enough for \"" << identifier.value << "\" variable" << std::endl;
    return {};
  }
  return (*res).second;
}

ExprValue NewVarExpr::evaluate() {
  variables[identifier.value] = value->evaluate();
  return variables[identifier.value];
}

ExprValue VarAssignExpr::evaluate() {
  auto var = variables.find(identifier.value);
  if (var == variables.end()) {
    std::cout << "there is no var \"" << identifier.value << "\", try ':=' for creating new variables" << std::endl;
  }
  variables[identifier.value] = value->evaluate();
  return variables[identifier.value];
}

ExprValue UnaryExpr::evaluate() {
  if (oper.grapheme == MINUS) {
    return ExprValue(-(value->evaluate()));
  } else {
    throw new std::exception("not implemented");
  }
}

ExprValue BinaryExpr::evaluate() {
  if (oper.grapheme == PLUS) {
    return left->evaluate() + right->evaluate();
  } else if (oper.grapheme == MINUS) {
    return left->evaluate() - right->evaluate();
  } else if (oper.grapheme == STAR) {
    return left->evaluate() * right->evaluate();
  } else if (oper.grapheme == SLASH) {
    return left->evaluate() / right->evaluate();
  }

  throw std::exception(std::format("no overload for '{}' as {}", oper.value, (int)oper.grapheme).c_str());
}

ExprValue PrintExpr::evaluate() {
  auto res = value->evaluate();
  std::cout << res.GetAsString() << std::endl;
  return res;
}
