#ifndef INTERPRETER
#define INTERPRETER

#include "syntax_tree.hpp"

namespace Diploma {

class Interpreter : public TreeWalker {
public:
  Interpreter(std::vector<Expr*> syntax);
  ~Interpreter();

  std::any visitBool(BoolExpr* BoolExpr) override;
  std::any visitInt32(Int32Expr* Int32Expr) override;
  std::any visitReal32(Real32Expr* Real32Expr) override;
  std::any visitStr(StrExpr* StrExpr) override;
  std::any visitNewVar(NewVarExpr* NewVarExpr) override;
  std::any visitVarAssign(VarAssignExpr* VarAssignExpr) override;
  std::any visitVar(VarExpr* VarExpr) override;
  std::any visitUnary(UnaryExpr* UnaryExpr) override;
  std::any visitBinary(BinaryExpr* BinaryExpr) override;
  std::any visitBlock(BlockExpr* BlockExpr) override;
  std::any visitFunc(FuncExpr* FuncExpr) override;
  std::any visitCall(CallExpr* CallExpr) override;
  std::any visitPrint(PrintExpr* PrintExpr) override;
};

} // namespace Diploma

#endif // INTERPRETER
