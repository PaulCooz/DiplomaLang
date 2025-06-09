#include "llvm_walker.cpp"
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

  auto IRFile = filesystem::path(path).replace_extension(".ir").string();
  TreeWalker* walkers[] = {
    new TypeWalker(),
    new LLVMWalker(IRFile),
  };
  for (auto walker : walkers) {
    walker->Do(syntaxTree);
  }

  for (auto walker : walkers) {
    delete walker;
  }

  cout << "done." << endl;
}
