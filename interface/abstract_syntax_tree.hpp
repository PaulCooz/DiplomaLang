#ifndef AST
#define AST

#include "tokenizer.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct ExprResult {
  union Value {
    bool boolean;
    int32_t SInt32;
    int64_t SInt64;
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

  ExprResult(double d) {
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
      return std::to_string(value.SInt32);
    case INT_64:
      return std::to_string(value.SInt64);
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
  Lexeme identifier;

  VarExpr(Lexeme identifier) : identifier(identifier) {}

  ExprResult evaluate() override;
};

class NewVarExpr : public Expr {
public:
  Lexeme identifier;
  Expr* value;

  NewVarExpr(Lexeme identifier, Expr* value) : identifier(identifier), value(value) {}

  ExprResult evaluate() override;
};

class VarAssignExpr : public Expr {
public:
  Lexeme identifier;
  Expr* value;

  VarAssignExpr(Lexeme identifier, Expr* value) : identifier(identifier), value(value) {}

  ExprResult evaluate() override;
};

class UnaryExpr : public Expr {
public:
  Lexeme oper;
  Expr* value;

  UnaryExpr(Lexeme oper, Expr* value) : oper(oper), value(value) {}

  ExprResult evaluate() override;
};

class BinaryExpr : public Expr {
public:
  Lexeme oper;
  Expr* left;
  Expr* right;

  BinaryExpr(Lexeme oper, Expr* left, Expr* right) : oper(oper), left(left), right(right) {}

  ExprResult evaluate() override;
};

class PrintExpr : public Expr {
public:
  Expr* value;

  PrintExpr(Expr* value) : value(value) {}

  ExprResult evaluate() override;
};

std::vector<Expr*> parseSyntaxTree(std::vector<Lexeme> t);

#endif // AST
