#ifndef AST
#define AST

#include "expression.hpp"
#include <vector>

void startAST();
std::vector<Expr*> parseSyntaxTree(std::vector<Token> t);
void finishAST();

#endif // AST
