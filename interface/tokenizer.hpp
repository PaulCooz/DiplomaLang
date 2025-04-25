#ifndef TOKENIZER
#define TOKENIZER

#include "common.hpp"
#include <string>
#include <vector>

// grapheme with a value
class Token {
public:
  Grapheme grapheme;
  std::string value;
  int line;
  int column;

  Token(Grapheme token, std::string value, int ln = -1, int col = -1);

  static Token endOfFile() {
    return Token(END_OF_FILE, "");
  }
};

std::vector<Token> performTokenization(std::istreambuf_iterator<char> begin, std::istreambuf_iterator<char> end);

#endif // TOKENIZER
