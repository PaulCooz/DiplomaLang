#ifndef FUNCTION
#define FUNCTION

#include "expression.hpp"
#include <functional>
#include <map>
#include <string>

class Func {
public:
  FuncExpr* expr;
  std::map<std::string, ExprResult> closure;

  Func(FuncExpr* expr, std::map<std::string, ExprResult> closure) : expr(expr), closure(closure) {}

  ExprResult
  Call(std::function<ExprResult(std::map<std::string, ExprResult>, BlockExpr*)> execute, std::vector<Expr*> args);
};

#endif // FUNCTION