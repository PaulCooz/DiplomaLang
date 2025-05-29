#ifndef INTERPRETER
#define INTERPRETER

#include "syntax_tree.hpp"

namespace Diploma {

class Interpreter : public TreeWalker {
public:
  Interpreter(std::vector<Expr*> syntax);
  ~Interpreter();

  std::any visitBool(BoolExpr* boolExpr) override;
  std::any visitInt32(Int32Expr* int32Expr) override;
  std::any visitReal64(Real64Expr* real64Expr) override;
  std::any visitStr(StrExpr* strExpr) override;
  std::any visitNewVar(NewVarExpr* newVarExpr) override;
  std::any visitVarAssign(VarAssignExpr* varAssignExpr) override;
  std::any visitVar(VarExpr* varExpr) override;
  std::any visitUnary(UnaryExpr* unaryExpr) override;
  std::any visitBinary(BinaryExpr* binaryExpr) override;
  std::any visitBlock(BlockExpr* blockExpr) override;
  std::any visitFunc(FuncExpr* funcExpr) override;
  std::any visitCall(CallExpr* callExpr) override;
  std::any visitPrintln(PrintlnExpr* printlnExpr) override;
};

} // namespace Diploma

#endif // INTERPRETER
