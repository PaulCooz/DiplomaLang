#include "syntax_tree.hpp"
#include <functional>
#include <iostream>
#include <map>

namespace Diploma {

class TypeWalker : public TreeWalker {
public:
  void Do(std::vector<Expr*> syntax) {
    for (auto expr : syntax) {
      expr->visit(this);
    }
  }

  std::any visitBool(BoolExpr* boolExpr) {
    boolExpr->type = BOOL;
    return (Expr*)boolExpr;
  }

  std::any visitInt32(Int32Expr* int32Expr) {
    int32Expr->type = I32;
    return (Expr*)int32Expr;
  }

  std::any visitReal64(Real64Expr* real64Expr) {
    real64Expr->type = R64;
    return (Expr*)real64Expr;
  }

  std::any visitStr(StrExpr* strExpr) {
    strExpr->type = STR;
    return (Expr*)strExpr;
  }

  std::any visitNewVar(NewVarExpr* newVarExpr) {
    auto initValue = std::any_cast<Expr*>(newVarExpr->value->visit(this));
    newVarExpr->type = initValue->type;
    return (Expr*)initValue;
  }

  std::any visitVarAssign(VarAssignExpr* varAssignExpr) {
    auto newValue = std::any_cast<Expr*>(varAssignExpr->value->visit(this));
    varAssignExpr->type = newValue->type;
    return (Expr*)newValue;
  }

  std::any visitVar(VarExpr* varExpr) {
    return (Expr*)varExpr; // TODO
  }

  std::any visitUnary(UnaryExpr* unaryExpr) {
    auto value = std::any_cast<Expr*>(unaryExpr->value->visit(this));
    unaryExpr->type = value->type;
    return (Expr*)unaryExpr;
  }

  std::any visitBinary(BinaryExpr* binaryExpr) {
    auto left = std::any_cast<Expr*>(binaryExpr->left->visit(this));
    auto right = std::any_cast<Expr*>(binaryExpr->right->visit(this));

    if (left->type == R64 || right->type == R64)
      binaryExpr->type = R64;
    else
      binaryExpr->type = I32;

    return (Expr*)binaryExpr;
  }

  std::any visitBlock(BlockExpr* BlockExpr) {}
  std::any visitFunc(FuncExpr* FuncExpr) {}
  std::any visitCall(CallExpr* CallExpr) {}

  std::any visitPrintln(PrintlnExpr* printlnExpr) {
    printlnExpr->type = I32;
    return (Expr*)printlnExpr;
  }
};

} // namespace Diploma
