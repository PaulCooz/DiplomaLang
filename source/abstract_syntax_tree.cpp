#include "abstract_syntax_tree.hpp"
#include "common.hpp"
#include <iostream>
#include <vector>

std::vector<Lexeme> tokens;
std::vector<Expr*> expressions;

int currToken = 0;

Lexeme top(int offset = 0) { // get i-th or EOF
  if (tokens.empty())
    return Lexeme::end_of_file();
  int i = currToken + offset;
  return 0 <= i && i < tokens.size() ? tokens[i] : tokens.back();
}

Lexeme pop() {
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
Expr* handleExpression();

Expr* handlePrimitive() {
  if (nextSequence(FALSE)) {
    pop();
    return new PrimaryExpr(ExprResult(false));
  }
  if (nextSequence(TRUE)) {
    pop();
    return new PrimaryExpr(ExprResult(true));
  }

  if (nextSequence(NUMBER)) {
    auto num = pop();
    return new PrimaryExpr(ExprResult(std::stod(num.value)));
  }
  if (nextSequence(STRING)) {
    auto str = pop();
    return new PrimaryExpr(ExprResult(str.value));
  }

  if (nextSequence(IDENTIFIER)) {
    auto id = pop();
    return new VarExpr(id);
  }

  if (nextSequence(LEFT_PAREN)) {
    pop();
    auto expr = handleTerm();
    if (!nextSequence(RIGHT_PAREN))
      std::cout << "STOP! Where is my ')'" << std::endl;
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
  return handlePrimitive();
}

Expr* handleFactor() {
  auto left = handleUnary();
  if (top().grapheme == STAR || top().grapheme == SLASH) {
    auto oper = pop();
    auto right = handleUnary();
    return new BinaryExpr(oper, left, right);
  }
  return left;
}

Expr* handleTerm() {
  auto left = handleFactor();
  if (top().grapheme == PLUS || top().grapheme == MINUS) {
    auto oper = pop();
    auto right = handleFactor();
    return new BinaryExpr(oper, left, right);
  }
  return left;
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

  return handleTerm();
}

std::vector<Expr*> parseSyntaxTree(std::vector<Lexeme> t) {
  tokens = t;
  expressions = {};
  while (!topIsEnd()) {
    auto exp = handleExpression();
    if (exp)
      expressions.emplace_back(exp);
    else
      currToken++;
  }
  return expressions;
}
