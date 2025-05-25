#include "abstract_syntax_tree.hpp"
#include "common.hpp"
#include <iostream>
#include <vector>

std::vector<Token> tokens;
std::vector<Expr*> expressions;

int currToken;

Token top(int offset = 0) { // get i-th or EOF
  if (tokens.empty())
    return Token::endOfFile();
  int i = currToken + offset;
  return 0 <= i && i < tokens.size() ? tokens[i] : tokens.back();
}

Token pop() {
  auto t = top();
  currToken++;
  return t;
}

template <typename... Args> bool nextSequence(Args... args) {
  Grapheme graphemes[] = {args...};
  for (int i = 0; i < sizeof...(args); i++) {
    if (top(i).grapheme != graphemes[i])
      return false;
  }
  return true;
}

bool topIsEnd() {
  return top().grapheme == END_OF_FILE;
}

Expr* handlePrimitive();
Expr* handleUnary();
Expr* handleFactor();
Expr* handleTerm();
BlockExpr* handleBlock();
Expr* handleFunc();
Expr* handleExpression();

Expr* handlePrimitive() {
  if (nextSequence(FALSE)) {
    pop();
    return new PrimaryExpr(ExprValue(false));
  }
  if (nextSequence(TRUE)) {
    pop();
    return new PrimaryExpr(ExprValue(true));
  }

  if (nextSequence(NUMBER)) {
    auto num = pop();
    if (num.value.contains("."))
      return new PrimaryExpr(ExprValue(std::stof(num.value)));
    else
      return new PrimaryExpr(ExprValue(std::stoi(num.value)));
  }
  if (nextSequence(STRING)) {
    auto str = pop();
    return new PrimaryExpr(ExprValue(str.value));
  }

  if (nextSequence(IDENTIFIER)) {
    auto id = pop();
    return new VarExpr(id);
  }

  if (nextSequence(LEFT_PAREN)) {
    pop();
    auto expr = handleTerm();
    if (!nextSequence(RIGHT_PAREN))
      std::cout << "STOP! Where is my ')'?" << std::endl;
    pop();
    return expr;
  }

  return nullptr;
}

Expr* handleUnary() {
  if (nextSequence(BANG) || nextSequence(MINUS)) {
    auto oper = pop();
    return new UnaryExpr(oper, handleUnary());
  }

  auto prim = handlePrimitive();
  if (nextSequence(LEFT_PAREN)) {
    pop(); // (
    std::vector<Expr*> args;
    while (!nextSequence(RIGHT_PAREN)) {
      args.emplace_back(handleExpression());

      if (nextSequence(COMMA))
        pop(); // ,
    }
    pop();     // )
    prim = new CallExpr(prim, args);
  }

  return prim;
}

Expr* handleFactor() {
  auto left = handleUnary();
  while (top().grapheme == STAR || top().grapheme == SLASH) {
    auto oper = pop();
    auto right = handleUnary();
    left = new BinaryExpr(oper, left, right);
  }
  return left;
}

Expr* handleTerm() {
  auto left = handleFactor();
  while (top().grapheme == PLUS || top().grapheme == MINUS) {
    auto oper = pop();
    auto right = handleFactor();
    left = new BinaryExpr(oper, left, right);
  }
  return left;
}

BlockExpr* handleBlock() {
  std::vector<Expr*> exprs;
  auto blockStartColumn = top().column;
  while (top().column == blockStartColumn) {
    exprs.emplace_back(handleExpression());
  }
  return new BlockExpr(exprs);
}

Expr* handleFunc() {
  std::vector<Token> args;
  while (true) {
    args.emplace_back(top());
    pop();

    if (top().grapheme == COMMA)
      pop();
    else
      break;
  }

  if (top().grapheme != MINUS_GREATER)
    std::cout << "Waited unnecessary '->' token" << std::endl;
  pop();

  return new FuncExpr(args, handleBlock());
}

Expr* handleExpression() {
  if (nextSequence(IDENTIFIER, COLON_EQUAL)) {
    auto identifier = pop();
    pop();
    auto value = handleExpression();
    return new NewVarExpr(identifier, value);
  }

  if (nextSequence(IDENTIFIER, EQUAL)) {
    auto identifier = pop();
    pop();
    auto value = handleExpression();
    return new VarAssignExpr(identifier, value);
  }

  if (nextSequence(IDENTIFIER, COLON_EQUAL)) {
    auto identifier = pop();
    pop();
    auto value = handleExpression();
    return new NewVarExpr(identifier, value);
  }

  if (top().grapheme == IDENTIFIER && top().value == "print") {
    pop();
    return new PrintExpr(handleExpression());
  }

  if (nextSequence(IDENTIFIER, COMMA)) {
    return handleFunc();
  }

  return handleTerm();
}

std::vector<Expr*> parseSyntaxTree(std::vector<Token> t) {
  tokens = t;
  expressions = {};
  currToken = 0;
  while (!topIsEnd()) {
    auto exp = handleExpression();
    if (exp)
      expressions.emplace_back(exp);
    else
      currToken++;
  }
  return expressions;
}
