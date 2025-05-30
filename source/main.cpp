#include "llvm_walker.cpp"
#include "type_walker.cpp"
#include <fstream>
#include <iostream>
#include <streambuf>
#include <vector>

using namespace std;
using namespace Diploma;

int main() {
  ifstream input("D:/GSU/diploma/input.txt");

  auto tokens = performTokenization(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
  auto syntaxTree = parseSyntaxTree(tokens);

  TreeWalker* walkers[] = {
    new TypeWalker(),
    new InterpreterWalker(),
  };
  for (auto walker : walkers) {
    walker->Do(syntaxTree);
  }

  for (auto walker : walkers) {
    delete walker;
  }

  cout << "done." << endl;
}
