#include "abstract_syntax_tree.hpp"
#include "function.hpp"
#include <format>
#include <functional>
#include <iostream>
#include <map>

std::map<std::string, ExprValue> variables;

void startAST() {}
void finishAST() {}

ExprValue executeBlock(std::map<std::string, ExprValue> vars, BlockExpr* block) {
  auto currVars = variables;
  variables = vars;
  auto result = block->evaluate();
  variables = currVars;
  return result;
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

ExprValue FuncExpr::evaluate() {
  return ExprValue(new Func(this, variables));
}

ExprValue
Func::Call(std::function<ExprValue(std::map<std::string, ExprValue>, BlockExpr*)> execute, std::vector<Expr*> args) {
  std::map<std::string, ExprValue> env;
  for (auto p : closure) {
    env[p.first] = p.second;
  }
  for (int i = 0; i < expr->args.size(); i++) {
    env[expr->args[i].value] = args[i]->evaluate();
  }
  return execute(env, expr->body);
}

ExprValue CallExpr::evaluate() {
  auto callable = func->evaluate();
  if (callable.type != ExprValue::FUNC)
    std::cout << "better NOT call /Saul/ " << callable.GetAsString() << std::endl;
  return callable.value.func->Call(executeBlock, args);
}

ExprValue PrintExpr::evaluate() {
  auto res = value->evaluate();
  std::cout << res.GetAsString() << std::endl;
  return res;
}
