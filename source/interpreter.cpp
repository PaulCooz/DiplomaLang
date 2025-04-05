#include "abstract_syntax_tree.hpp"
#include <iostream>
#include <map>

std::map<std::string, ExprResult> variables;

ExprResult PrimaryExpr::evaluate() {
  return result;
}

ExprResult VarExpr::evaluate() {
  auto res = variables.find(identifier.value);
  if (res == variables.end()) {
    std::cout << "not nice enough for \"" << identifier.value << "\" variable" << std::endl;
    return {};
  }
  return (*res).second;
}

ExprResult NewVarExpr::evaluate() {
  variables[identifier.value] = value->evaluate();
  std::cout << "var " << identifier.value << " = " << variables[identifier.value].GetAsString() << std::endl;
  return variables[identifier.value];
}

ExprResult VarAssignExpr::evaluate() {
  auto var = variables.find(identifier.value);
  if (var == variables.end()) {
    std::cout << "there is no var \"" << identifier.value << "\", try ':=' for creating new variables" << std::endl;
  }
  variables[identifier.value] = value->evaluate();
  std::cout << identifier.value << " = " << variables[identifier.value].GetAsString() << std::endl;
  return variables[identifier.value];
}

ExprResult UnaryExpr::evaluate() {
  std::cout << oper.value << std::endl;
  return value->evaluate();
}

ExprResult BinaryExpr::evaluate() {
  auto res = (double_t)0;
  if (oper.grapheme == PLUS) {
    res = left->evaluate().value.Real64 + right->evaluate().value.Real64;
  } else if (oper.grapheme == MINUS) {
    res = left->evaluate().value.Real64 - right->evaluate().value.Real64;
  } else if (oper.grapheme == STAR) {
    res = left->evaluate().value.Real64 * right->evaluate().value.Real64;
  } else if (oper.grapheme == SLASH) {
    res = left->evaluate().value.Real64 / right->evaluate().value.Real64;
  }
  return ExprResult(res);
}

ExprResult PrintExpr::evaluate() {
  auto res = value->evaluate();
  std::cout << "print " << res.GetAsString() << std::endl;
  return res;
}
