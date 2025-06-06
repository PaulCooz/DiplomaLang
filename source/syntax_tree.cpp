#include "syntax_tree.hpp"
#include <iostream>
#include <vector>

namespace Diploma {

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
Expr* handleComparison();
Expr* handleEquality();
Expr* handleLogicalAnd();
Expr* handleLogicalOr();
BlockExpr* handleBlock();
FuncExpr* handleFunc();
Expr* handleIfElse();
Expr* handleExpression();

Expr* handlePrimitive() {
  if (nextSequence(FALSE)) {
    pop();
    return new BoolExpr(false);
  }
  if (nextSequence(TRUE)) {
    pop();
    return new BoolExpr(true);
  }

  if (nextSequence(NUMBER)) {
    auto num = pop();
    auto isReal = num.value.find(".") != std::string::npos;
    return isReal ? (Expr*)new Real64Expr(std::stod(num.value)) : (Expr*)new Int32Expr(std::stoi(num.value));
  }
  if (nextSequence(STRING)) {
    auto str = pop();
    return new StrExpr(str.value);
  }

  if (nextSequence(IDENTIFIER)) {
    auto id = pop();
    return new VarExpr(id);
  }

  if (nextSequence(LEFT_PAREN)) {
    pop();
    auto expr = handleExpression();
    if (!nextSequence(RIGHT_PAREN))
      std::cout << "STOP! Where is my ')'?" << std::endl;
    pop();
    return expr;
  }

  return nullptr;
}

Expr* handleUnary() {
  if (nextSequence(BANG) || nextSequence(MINUS) || nextSequence(PLUS)) {
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

Expr* handleComparison() {
  auto left = handleTerm();
  while (top().grapheme == GREATER || top().grapheme == GREATER_EQUAL || top().grapheme == LESS ||
         top().grapheme == LESS_EQUAL) {
    auto oper = pop();
    auto right = handleTerm();
    left = new ComparisonExpr(oper, left, right);
  }
  return left;
}

Expr* handleEquality() {
  auto left = handleComparison();
  while (top().grapheme == BANG_EQUAL || top().grapheme == EQUAL_EQUAL) {
    auto oper = pop();
    auto right = handleComparison();
    left = new ComparisonExpr(oper, left, right);
  }
  return left;
}

Expr* handleLogicalAnd() {
  auto expr = handleEquality();
  if (nextSequence(AND)) {
    auto oper = pop();
    auto right = handleEquality();
    expr = new LogicalExpr(oper, expr, right);
  }
  return expr;
}

Expr* handleLogicalOr() {
  auto expr = handleLogicalAnd();
  if (nextSequence(OR)) {
    auto oper = pop();
    auto right = handleLogicalAnd();
    expr = new LogicalExpr(oper, expr, right);
  }
  return expr;
}

BlockExpr* handleBlock() {
  std::vector<Expr*> exprs;
  auto blockStartColumn = top().column;
  while (top().column == blockStartColumn) {
    exprs.emplace_back(handleExpression());
  }
  return new BlockExpr(exprs);
}

FuncExpr* handleFunc() {
  std::vector<Token> args;
  auto withParen = top().grapheme == LEFT_PAREN;
  if (withParen)
    pop();                            // (
  while (true) {
    if (top().grapheme != IDENTIFIER) // no args
      break;

    args.emplace_back(top());
    pop();                            // id

    if (top().grapheme == COMMA)
      pop();                          // ,
    else
      break;
  }

  if (withParen) {
    if (top().grapheme == RIGHT_PAREN)
      pop(); // )
    else
      std::cout << "Waiting for an extremely needed token ')'!\n";
  }

  if (top().grapheme != MINUS_GREATER)
    std::cout << "Waited unnecessary '->' token" << std::endl;
  pop();

  return new FuncExpr(args, handleBlock());
}

Expr* handleIfElse() {
  pop(); // if

  auto condition = handleExpression();
  auto thenBlock = handleBlock();
  auto elseBlock = (BlockExpr*)nullptr;
  if (nextSequence(ELSE)) {
    pop(); // else
    elseBlock = handleBlock();
  }

  return new IfElseExpr(condition, thenBlock, elseBlock);
}

bool isNextFunc() {
  auto offset = 0;
  auto withParen = top(offset).grapheme == LEFT_PAREN;
  if (withParen)
    offset++;
  while (true) {
    if (top(offset).grapheme == IDENTIFIER) {
      offset++;
      if (top(offset).grapheme == COMMA) {
        offset++;
      }
    } else {
      break;
    }
  }
  if (withParen) {
    if (top(offset).grapheme == RIGHT_PAREN)
      offset++;
    else
      return false;
  }
  return top(offset).grapheme == MINUS_GREATER;
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

  if (top().grapheme == IDENTIFIER && top().value == "println") {
    pop();     // println
    auto withParen = top().grapheme == LEFT_PAREN;
    if (withParen)
      pop();   // (
    std::vector<Expr*> values;
    while (true) {
      values.emplace_back(handleLogicalOr());
      if (top().grapheme == COMMA)
        pop(); // ,
      else
        break;
    }
    if (withParen) {
      if (top().grapheme == RIGHT_PAREN)
        pop(); // )
      else
        std::cout << "expected a ')' token,"
                     "but if you don't like writing brackets,"
                     "you can remove the '(' that comes after 'println'\n";
    }
    return new PrintlnExpr(values);
  }

  if (isNextFunc()) {
    return handleFunc();
  }

  if (nextSequence(IF)) {
    return handleIfElse();
  }

  return handleLogicalOr();
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

} // namespace Diploma
