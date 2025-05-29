#ifndef AST
#define AST

#include "tokenizer.hpp"
#include <any>
#include <vector>

namespace Diploma {

class Expr;
class BoolExpr;
class Int32Expr;
class Real64Expr;
class StrExpr;
class NewVarExpr;
class VarAssignExpr;
class VarExpr;
class UnaryExpr;
class BinaryExpr;
class BlockExpr;
class FuncExpr;
class CallExpr;
class PrintlnExpr;

class TreeWalker {
public:
  virtual std::any visitBool(BoolExpr*) = 0;
  virtual std::any visitInt32(Int32Expr*) = 0;
  virtual std::any visitReal64(Real64Expr*) = 0;
  virtual std::any visitStr(StrExpr*) = 0;
  virtual std::any visitNewVar(NewVarExpr*) = 0;
  virtual std::any visitVarAssign(VarAssignExpr*) = 0;
  virtual std::any visitVar(VarExpr*) = 0;
  virtual std::any visitUnary(UnaryExpr*) = 0;
  virtual std::any visitBinary(BinaryExpr*) = 0;
  virtual std::any visitBlock(BlockExpr*) = 0;
  virtual std::any visitFunc(FuncExpr*) = 0;
  virtual std::any visitCall(CallExpr*) = 0;
  virtual std::any visitPrintln(PrintlnExpr*) = 0;
};

class Expr {
public:
  virtual std::any visit(TreeWalker* walker) = 0;
};

class BoolExpr : public Expr {
public:
  bool value;

  BoolExpr(bool value) : value(value) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitBool(this);
  }
};

class Int32Expr : public Expr {
public:
  int32_t value;

  Int32Expr(int32_t value) : value(value) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitInt32(this);
  }
};

class Real64Expr : public Expr {
public:
  float value;

  Real64Expr(float value) : value(value) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitReal64(this);
  }
};

class StrExpr : public Expr {
public:
  std::string value;

  StrExpr(std::string value) : value(value) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitStr(this);
  }
};

class VarExpr : public Expr {
public:
  Token identifier;

  VarExpr(Token identifier) : identifier(identifier) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitVar(this);
  }
};

class NewVarExpr : public Expr {
public:
  Token identifier;
  Expr* value;

  NewVarExpr(Token identifier, Expr* value) : identifier(identifier), value(value) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitNewVar(this);
  }
};

class VarAssignExpr : public Expr {
public:
  Token identifier;
  Expr* value;

  VarAssignExpr(Token identifier, Expr* value) : identifier(identifier), value(value) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitVarAssign(this);
  }
};

class UnaryExpr : public Expr {
public:
  Token oper;
  Expr* value;

  UnaryExpr(Token oper, Expr* value) : oper(oper), value(value) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitUnary(this);
  }
};

class BinaryExpr : public Expr {
public:
  Token oper;
  Expr* left;
  Expr* right;

  BinaryExpr(Token oper, Expr* left, Expr* right) : oper(oper), left(left), right(right) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitBinary(this);
  }
};

class BlockExpr : public Expr {
public:
  std::vector<Expr*> list;

  BlockExpr(std::vector<Expr*> list) : list(list) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitBlock(this);
  }
};

class FuncExpr : public Expr {
public:
  std::vector<Token> args;
  Expr* body;

  FuncExpr(std::vector<Token> args, Expr* body) : args(args), body(body) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitFunc(this);
  }
};

class CallExpr : public Expr {
public:
  Expr* func;
  std::vector<Expr*> args;

  CallExpr(Expr* func, std::vector<Expr*> args) : func(func), args(args) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitCall(this);
  }
};

class PrintlnExpr : public Expr {
public:
  Expr* format;
  Expr* value;

  PrintlnExpr(Expr* format, Expr* value) : format(format), value(value) {}

  std::any visit(TreeWalker* walker) override {
    return walker->visitPrintln(this);
  }
};

std::vector<Expr*> parseSyntaxTree(std::vector<Token> t);

} // namespace Diploma

#endif // AST
