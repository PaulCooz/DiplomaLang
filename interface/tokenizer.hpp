#ifndef TOKENIZER
#define TOKENIZER

#include "common.hpp"
#include <string>
#include <vector>

// grapheme with a value
class Lexeme {
public:
  Grapheme grapheme;
  std::string value;
  int line;
  int column;

  Lexeme(Grapheme token, std::string value, int ln = -1, int col = -1);

  static Lexeme end_of_file() {
    return Lexeme(END_OF_FILE, "");
  }
};

std::vector<Lexeme> performTokenization(std::istreambuf_iterator<char> begin, std::istreambuf_iterator<char> end);

#endif // TOKENIZER
