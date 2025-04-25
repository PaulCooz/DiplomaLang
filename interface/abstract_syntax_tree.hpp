#ifndef AST
#define AST

#include "expression.hpp"
#include <vector>

std::vector<Expr*> parseSyntaxTree(std::vector<Token> t);

#endif // AST
