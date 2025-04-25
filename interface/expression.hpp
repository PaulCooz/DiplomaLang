#ifndef EXPR
#define EXPR

#include "tokenizer.hpp"
#include <cstdint>
#include <format>
#include <string>

struct ExprResult {
  union Value {
    bool boolean;
    int32_t Int32;
    int64_t Int64;
    float_t Real32;
    double_t Real64;
    const char* CStr;
  };

  enum Type {
    VOID,
    BOOL,
    INT_32,
    INT_64,
    REAL_32,
    REAL_64,
    CStr,
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
    value.Int32 = i;
  }

  ExprResult(int64_t i) {
    type = INT_64;
    value.Int64 = i;
  }

  ExprResult(float_t f) {
    type = REAL_32;
    value.Real32 = f;
  }

  ExprResult(double_t d) {
    type = REAL_64;
    value.Real64 = d;
  }

  ExprResult(std::string s) {
    type = CStr;
    value.CStr = s.c_str();
  }

  std::string GetAsString() {
    switch (type) {
    case BOOL:
      return std::to_string(value.boolean);
    case INT_32:
      return std::to_string(value.Int32);
    case INT_64:
      return std::to_string(value.Int64);
    case REAL_32:
      return std::to_string(value.Real32);
    case REAL_64:
      return std::to_string(value.Real64);
    case CStr:
      return value.CStr;

    default:
      return "";
    }
  }

  ExprResult operator-() {
    switch (type) {
    case INT_32:
      return ExprResult(-value.Int32);
    case INT_64:
      return ExprResult(-value.Int64);
    case REAL_32:
      return ExprResult(-value.Real32);
    case REAL_64:
      return ExprResult(-value.Real64);
    default:
      throw std::exception(std::format("there is no operator {} for {} type", '-', (int)type).c_str());
    }
  }

#define binOperator(x) \
  ExprResult operator x(const ExprResult other) { \
    std::pair<std::pair<Type, Type>, ExprResult (*)(ExprResult, ExprResult)> overloads[] = { \
      {{INT_32, INT_32},   [](auto l, auto r) { return ExprResult(l.value.Int32 x r.value.Int32); }            }, \
      {{INT_32, INT_64},   [](auto l, auto r) { return ExprResult((int64_t)l.value.Int32 x r.value.Int64); }   }, \
      {{INT_32, REAL_32},  [](auto l, auto r) { return ExprResult((float_t)l.value.Int32 x r.value.Real32); }  }, \
      {{INT_32, REAL_64},  [](auto l, auto r) { return ExprResult((double_t)l.value.Int32 x r.value.Real64); } }, \
      {{INT_64, INT_64},   [](auto l, auto r) { return ExprResult(l.value.Int64 x r.value.Int64); }            }, \
      {{INT_64, REAL_32},  [](auto l, auto r) { return ExprResult((float_t)l.value.Int64 x r.value.Real32); }  }, \
      {{INT_64, REAL_64},  [](auto l, auto r) { return ExprResult((double_t)l.value.Int64 x r.value.Real64); } }, \
      {{REAL_32, REAL_32}, [](auto l, auto r) { return ExprResult(l.value.Real32 x r.value.Real32); }          }, \
      {{REAL_32, REAL_64}, [](auto l, auto r) { return ExprResult((double_t)l.value.Real32 x r.value.Real64); }}, \
      {{REAL_64, REAL_64}, [](auto l, auto r) { return ExprResult(l.value.Real64 x r.value.Real64); }          }, \
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

class PrintExpr : public Expr {
public:
  Expr* value;

  PrintExpr(Expr* value) : value(value) {}

  ExprResult evaluate() override;
};

#endif // EXPR
