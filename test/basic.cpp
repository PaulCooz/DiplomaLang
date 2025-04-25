#include "abstract_syntax_tree.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <optional>
#include <string>

using namespace std;
using namespace testing;

TEST(Basic, CalcAOBOC) {
  stringstream resultStream;
  auto originalOutput = cout.rdbuf();
  cout.rdbuf(resultStream.rdbuf());

  auto operations = {'+', '-', '*', '/'};
  auto numbers = {-2, -1, 0, +1, +2};
  auto binaryOperation = [](int l, int r, char o) {
    if (o == '+') {
      return l + r;
    } else if (o == '-') {
      return l - r;
    } else if (o == '*') {
      return l * r;
    } else if (o == '/') {
      return l / r;
    } else {
      throw new exception("bad operator in test setup");
    }
  };
  map<string, int> exprWithResult;
  for (auto oper1 : operations)
    for (auto oper2 : operations)
      for (auto a : numbers)
        for (auto b : numbers)
          for (auto c : numbers) {
            if ((oper1 == '/' && b == 0) || (oper2 == '/' && c == 0)) {
              continue;
            }
            if ((oper1 == '+' || oper1 == '-') && (oper2 == '*' || oper2 == '/')) {
              swap(oper1, oper2);
              swap(a, b);
              swap(b, c);
            }
            auto left = binaryOperation(a, b, oper1);
            auto right = binaryOperation(left, c, oper2);

            std::ostringstream exprStr;
            exprStr << '(' << a << ')' << oper1 << '(' << b << ')' << oper2 << '(' << c << ')';
            exprWithResult[exprStr.str()] = right;
          }

  for (auto p : exprWithResult) {
    std::istringstream stream("print " + p.first);
    auto tokens = performTokenization(istreambuf_iterator<char>(stream), istreambuf_iterator<char>());
    auto syntaxTree = parseSyntaxTree(tokens);
    for (auto t : syntaxTree) {
      t->evaluate();
    }

    string value;
    resultStream >> value;
    if (stoi(value) != p.second) {
      FAIL() << ("print " + p.first) << " is " << value << " but " << p.second << " needed";
    }
  }

  cout.rdbuf(originalOutput);
}
