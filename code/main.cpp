#include "common.cpp"
#include "tokenizer.cpp"
#include <fstream>
#include <streambuf>
#include <vector>

using namespace std;

int main() {
  ifstream input("D:/GSU/diploma/input.txt");

  auto tokens = performTokenization(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
  cout << tokens.size() << endl;

  // auto currToken = tokens[i];
  // while (currToken.sym != END_OF_FILE) {
  //   i++;
  // }
}
