#include "abstract_syntax_tree.hpp"
#include "common.hpp"
#include "tokenizer.hpp"
#include <fstream>
#include <streambuf>
#include <vector>

using namespace std;

int main() {
  ifstream input("D:/GSU/diploma/input.txt");

  auto tokens = performTokenization(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
  auto syntaxTree = parseSyntaxTree(tokens);
  for (auto t : syntaxTree) {
    t->evaluate();
  }
}
