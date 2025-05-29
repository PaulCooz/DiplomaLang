#include "interpreter.hpp"
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
  auto runner = Diploma::Interpreter(syntaxTree);

  cout << "done." << endl;
}
