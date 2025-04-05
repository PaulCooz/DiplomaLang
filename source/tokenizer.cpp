#include "tokenizer.hpp"
#include "common.hpp"
#include <functional>
#include <iostream>
#include <optional>
#include <string>

int currLine, currColumn;

Lexeme::Lexeme(Grapheme token, std::string value, int ln, int col)
    : grapheme(token), value(value), line(ln >= 0 ? ln : currLine), column(col >= 0 ? col : currColumn) {}

bool isDigit(std::string str, int i) {
  return i < str.length() ? isdigit(str[i]) : false;
}

bool isAlpha(std::string str, int i) {
  return i < str.length() ? isalpha(str[i]) || str[i] == '_' : false;
}

bool isSpaceOrEOF(std::string str, int i) {
  return i < str.length() ? isspace(str[i]) : true;
}

bool isSub(std::string str, int from, std::string substr) {
  return str.substr(from, substr.length()) == substr;
}

void incCursor(std::string str, int& i, int count = 1) {
  for (; i < str.length() && count > 0; count--) {
    currColumn++;
    if (str[i] == '\n') { // TODO ignore while string parsing
      currColumn = 0;
      currLine++;
    }

    i++;
  }
}

std::function<std::optional<Lexeme>(std::string str, int& i)>
wordHandler(Grapheme grapheme, std::string word, bool checkSpaceOrEOF = false) {
  return [grapheme, word, checkSpaceOrEOF](std::string str, int& i) {
    std::optional<Lexeme> result = std::nullopt;
    if (isSub(str, i, word) && (!checkSpaceOrEOF || isSpaceOrEOF(str, i + word.length()))) {
      result = Lexeme(grapheme, word);
      incCursor(str, i, word.length());
    }
    return result;
  };
}

std::optional<Lexeme> numberHandler(std::string str, int& i) {
  std::optional<Lexeme> result = std::nullopt;
  if (isDigit(str, i)) {
    bool has_dot = false;
    std::string value = "";
    int sLn = currLine, sCol = currColumn;
    while (i < str.length()) {
      if (str[i] == '.') {
        if (has_dot) {
          std::cout << "Too much dots for one number, I know you love it but don't overdo" << std::endl;
        } else {
          value += '.';
          has_dot = true;
        }
      } else if (isDigit(str, i)) {
        value += str[i];
      } else {
        break;
      }
      incCursor(str, i);
    }
    result = Lexeme(NUMBER, value, sLn, sCol);
  }
  return result;
}

std::optional<Lexeme> identifierHandler(std::string str, int& i) {
  std::optional<Lexeme> result = std::nullopt;
  if (isAlpha(str, i)) {
    std::string value = "";
    int sLn = currLine, sCol = currColumn;
    do {
      value += str[i];
      incCursor(str, i);
      if (!isDigit(str, i) && !isAlpha(str, i))
        break;
    } while (i < str.length());
    result = Lexeme(IDENTIFIER, value, sLn, sCol);
  }
  return result;
}

std::function<std::optional<Lexeme>(std::string str, int& i)> tokenHandlers[] = {
    wordHandler(LEFT_PAREN, "("),
    wordHandler(RIGHT_PAREN, ")"),
    wordHandler(LEFT_BRACE, "{"),
    wordHandler(RIGHT_BRACE, "}"),
    wordHandler(COMMA, ","),
    wordHandler(BANG_EQUAL, "!="),
    wordHandler(EQUAL_EQUAL, "=="),
    wordHandler(GREATER_EQUAL, ">="),
    wordHandler(LESS_EQUAL, "<="),
    wordHandler(COLON_EQUAL, ":="),
    wordHandler(PLUS, "+"),
    wordHandler(MINUS, "-"),
    wordHandler(STAR, "*"),
    wordHandler(EQUAL, "="),
    wordHandler(BANG, "!"),
    wordHandler(GREATER, ">"),
    wordHandler(LESS, "<"),
    wordHandler(COLON, ":"),
    wordHandler(SLASH, "/"),

    wordHandler(TRUE, "true", true),
    wordHandler(FALSE, "false", true),
    wordHandler(AND, "and", true),
    wordHandler(OR, "or", true),
    wordHandler(IS, "is", true),
    wordHandler(AS, "as", true),
    wordHandler(OF, "of", true),
    wordHandler(FOR, "for", true),
    wordHandler(WHILE, "while", true),
    wordHandler(IF, "if", true),
    wordHandler(ELSE, "else", true),

    numberHandler,
    identifierHandler,
};

std::vector<Lexeme> performTokenization(std::istreambuf_iterator<char> begin, std::istreambuf_iterator<char> end) {
  std::string str(begin, end);

  std::vector<Lexeme> tokens;
  int i = 0;
  currLine = currColumn = 0;
  while (i < str.length()) {
    bool handle = false;

    if (isSub(str, i, "//")) {
      while (i < str.length() && str[i] != '\n') {
        incCursor(str, i);
      }
    }

    for (auto handler : tokenHandlers) {
      auto res = handler(str, i);
      if (res.has_value()) {
        tokens.emplace_back(res.value());
        handle = true;
        break;
      }
    }
    if (!handle) {
      incCursor(str, i);
    }
  }
  tokens.emplace_back(Lexeme::end_of_file());

  return tokens;
}