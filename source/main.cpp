#if USE_LLVM
#include "llvm_walker.cpp"
#endif
#include "type_walker.cpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <vector>

using namespace std;
using namespace Diploma;

int main(int argc, char* argv[]) {
  string path = "D:/GSU/diploma/input.txt";
  if (argc > 1) {
    path = argv[1];
  }
  ifstream input(path);

  auto tokens = performTokenization(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
  auto syntaxTree = parseSyntaxTree(tokens);

  TreeWalker* walkers[] = {
    new TypeWalker(),
#if USE_LLVM
    new LLVMWalker(),
#endif
  };
  for (auto walker : walkers) {
    walker->Do(syntaxTree);
  }

  for (auto walker : walkers) {
    delete walker;
  }
}
