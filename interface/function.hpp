#ifndef FUNCTION
#define FUNCTION

#include "expression.hpp"
#include <functional>
#include <map>
#include <string>

class Func {
public:
  FuncExpr* expr;
  std::map<std::string, ExprValue> closure;

  Func(FuncExpr* expr, std::map<std::string, ExprValue> closure) : expr(expr), closure(closure) {}

  ExprValue
  Call(std::function<ExprValue(std::map<std::string, ExprValue>, BlockExpr*)> execute, std::vector<Expr*> args);
};

#endif // FUNCTION