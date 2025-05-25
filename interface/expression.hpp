#ifndef EXPR
#define EXPR

class Expr;
class FuncExpr;
class BlockExpr;
struct ExprResult;

#include "function.hpp"
#include "tokenizer.hpp"
#include <cstdint>
#include <format>
#include <string>
#include <vector>

struct ExprResult {
  union Value {
    bool boolean;
    int32_t sint32;
    float_t real32;
    const char* CStr;

    Func* func;
  };

  enum Type {
    VOID,
    BOOL,
    INT_32,
    REAL_32,
    CStr,

    FUNC,
  };

  Type type;
  Value value;

  ExprResult() {
    type = VOID;
  }

  ExprResult(bool b) {
    type = BOOL;
    value.boolean = b;
  }

  ExprResult(int i) {
    type = INT_32;
    value.sint32 = i;
  }

  ExprResult(float_t f) {
    type = REAL_32;
    value.real32 = f;
  }

  ExprResult(std::string s) {
    type = CStr;
    value.CStr = s.c_str();
  }

  ExprResult(Func* func) {
    type = FUNC;
    value.func = func;
  }

  std::string GetAsString() {
    switch (type) {
    case BOOL:
      return std::to_string(value.boolean);
    case INT_32:
      return std::to_string(value.sint32);
    case REAL_32:
      return std::to_string(value.real32);
    case CStr:
      return value.CStr;

    case FUNC:
      return "function";

    default:
      return "";
    }
  }

  ExprResult operator-() {
    switch (type) {
    case INT_32:
      return ExprResult(-value.sint32);
    case REAL_32:
      return ExprResult(-value.real32);
    default:
      throw std::exception(std::format("there is no operator {} for {} type", '-', (int)type).c_str());
    }
  }

#define binOperator(x) \
  ExprResult operator x(const ExprResult other) { \
    std::pair<std::pair<Type, Type>, ExprResult (*)(ExprResult, ExprResult)> overloads[] = { \
      {{INT_32, INT_32},   [](auto l, auto r) { return ExprResult(l.value.sint32 x r.value.sint32); }            }, \
      {{INT_32, REAL_32},  [](auto l, auto r) { return ExprResult((float_t)l.value.sint32 x r.value.real32); }  }, \
      {{REAL_32, REAL_32}, [](auto l, auto r) { return ExprResult(l.value.real32 x r.value.real32); }          }, \
    }; \
    for (auto t : overloads) { \
      if (type == t.first.first && other.type == t.first.second) { \
        return t.second(*this, other); \
      } else if (other.type == t.first.first && type == t.first.second) { \
        return t.second(other, *this); \
      } \
    } \
    throw std::exception( \
      std::format("can't find overload for type({}) {} type({})", (int)type, #x, (int)other.type).c_str() \
    ); \
  }

  binOperator(+);
  binOperator(-);
  binOperator(*);
  binOperator(/);

#undef binOperator
};

class Expr {
public:
  virtual ExprResult evaluate() = 0;
};

class PrimaryExpr : public Expr {
public:
  ExprResult result;

  PrimaryExpr(ExprResult result) : result(result) {}

  ExprResult evaluate() override;
};

class VarExpr : public Expr {
public:
  Token identifier;

  VarExpr(Token identifier) : identifier(identifier) {}

  ExprResult evaluate() override;
};

class NewVarExpr : public Expr {
public:
  Token identifier;
  Expr* value;

  NewVarExpr(Token identifier, Expr* value) : identifier(identifier), value(value) {}

  ExprResult evaluate() override;
};

class VarAssignExpr : public Expr {
public:
  Token identifier;
  Expr* value;

  VarAssignExpr(Token identifier, Expr* value) : identifier(identifier), value(value) {}

  ExprResult evaluate() override;
};

class UnaryExpr : public Expr {
public:
  Token oper;
  Expr* value;

  UnaryExpr(Token oper, Expr* value) : oper(oper), value(value) {}

  ExprResult evaluate() override;
};

class BinaryExpr : public Expr {
public:
  Token oper;
  Expr* left;
  Expr* right;

  BinaryExpr(Token oper, Expr* left, Expr* right) : oper(oper), left(left), right(right) {}

  ExprResult evaluate() override;
};

class BlockExpr : public Expr {
public:
  std::vector<Expr*> list;

  BlockExpr(std::vector<Expr*> list) : list(list) {}

  ExprResult evaluate() override {
    auto lastResult = ExprResult();
    for (const auto e : list) {
      lastResult = e->evaluate();
    }
    return lastResult;
  }
};

class FuncExpr : public Expr {
public:
  std::vector<Token> args;
  BlockExpr* body;

  FuncExpr(std::vector<Token> args, BlockExpr* body) : args(args), body(body) {}

  ExprResult evaluate() override;
};

class CallExpr : public Expr {
public:
  Expr* func;
  std::vector<Expr*> args;

  CallExpr(Expr* func, std::vector<Expr*> args) : func(func), args(args) {}

  ExprResult evaluate() override;
};

class PrintExpr : public Expr {
public:
  Expr* value;

  PrintExpr(Expr* value) : value(value) {}

  ExprResult evaluate() override;
};

#endif // EXPR
