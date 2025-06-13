#include "syntax_tree.hpp"
#include <functional>
#include <iostream>
#include <map>
#include <optional>

namespace Diploma {

class TypeWalker : public TreeWalker {
  std::map<std::string, Expr*> context;

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
    auto res = context.try_emplace(newVarExpr->identifier.value, initValue);
    if (!res.second) {
      std::cout << "oh no, you should use assign(=) instead of creating(:=) operator\n";
    }
    return (Expr*)initValue;
  }

  std::any visitVarAssign(VarAssignExpr* varAssignExpr) {
    auto newValue = std::any_cast<Expr*>(varAssignExpr->value->visit(this));
    varAssignExpr->type = newValue->type;
    context[varAssignExpr->identifier.value] = newValue;
    return (Expr*)newValue;
  }

  std::any visitVar(VarExpr* varExpr) {
    auto value = context[varExpr->identifier.value];
    varExpr->type = value->type;
    return (Expr*)value;
  }

  std::any visitUnary(UnaryExpr* unaryExpr) {
    auto value = std::any_cast<Expr*>(unaryExpr->value->visit(this));
    unaryExpr->type = value->type;
    return (Expr*)unaryExpr;
  }

  std::any visitComparison(ComparisonExpr* comparisonExpr) {
    auto left = std::any_cast<Expr*>(comparisonExpr->left->visit(this));
    auto right = std::any_cast<Expr*>(comparisonExpr->right->visit(this));
    comparisonExpr->type = BOOL;
    return (Expr*)comparisonExpr;
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

  std::any visitLogical(LogicalExpr* logicalExpr) {
    logicalExpr->left->visit(this);
    logicalExpr->right->visit(this);
    logicalExpr->type = BOOL;
    return (Expr*)logicalExpr;
  }

  std::any visitIfElse(IfElseExpr* ifElseExpr) {
    auto thenRetType = std::any_cast<Expr*>(ifElseExpr->thenBlock->visit(this))->type;
    auto elseRetType = (std::optional<ExprType>)std::nullopt;
    if (ifElseExpr->elseBlock != nullptr) {
      elseRetType = std::any_cast<Expr*>(ifElseExpr->elseBlock->visit(this))->type;
    }
    if (elseRetType.has_value() && thenRetType != elseRetType) {
      std::cout << "it can be ok, but there are different types if-else blocks return\n";
    }
    ifElseExpr->type = thenRetType;
    return (Expr*)ifElseExpr;
  }

  std::any visitBlock(BlockExpr* blockExpr) {
    auto lastValue = (Expr*)nullptr;
    for (auto b : blockExpr->list) {
      lastValue = std::any_cast<Expr*>(b->visit(this));
    }
    blockExpr->type = lastValue->type;
    return (Expr*)lastValue;
  }

  std::any visitFunc(FuncExpr* funcExpr) {
    funcExpr->type = FUNC;
    return (Expr*)funcExpr;
  }

  std::any visitCall(CallExpr* callExpr) {
    auto func = (FuncExpr*)std::any_cast<Expr*>(callExpr->func->visit(this));
    auto oldContext = context;
    context.clear();
    if (func->argsTypes.empty() && !callExpr->args.empty()) {
      for (auto i = 0; i < callExpr->args.size(); i++) {
        auto arg = std::any_cast<Expr*>(callExpr->args[i]->visit(this));
        func->argsTypes.emplace_back(arg->type);
        context[func->args[i].value] = arg;
      }
    }
    if (func->args.size() != func->argsTypes.size()) {
      std::cout << "No no, you call func with " << func->argsTypes.size() << " args of " << func->args.size()
                << ", it not very zingy for now!\n";
    }

    auto result = std::any_cast<Expr*>(func->body->visit(this));
    func->retType = callExpr->type = result->type;
    context = oldContext;
    return result;
  }

  std::any visitPrintln(PrintlnExpr* printlnExpr) {
    for (auto v : printlnExpr->values) {
      v->visit(this);
    }
    printlnExpr->type = I32;
    return (Expr*)printlnExpr;
  }
};

} // namespace Diploma
