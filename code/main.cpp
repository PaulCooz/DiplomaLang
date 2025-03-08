#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <streambuf>
#include <string>
#include <vector>

using namespace std;

enum Token {
  END_OF_FILE,

  LEFT_PAREN,    // (
  RIGHT_PAREN,   // )
  LEFT_BRACE,    // {
  RIGHT_BRACE,   // }
  COMMA,         // ,

  STAR,          // *
  PLUS,          // +
  MINUS,         // -
  DOT,           // .

  BANG,          // !
  BANG_EQUAL,    // !=
  EQUAL,         // =
  EQUAL_EQUAL,   // ==
  GREATER,       // >
  GREATER_EQUAL, // >=
  LESS,          // <
  LESS_EQUAL,    // <=
  COLON,         // :
  COLON_EQUAL,   // :=
  SLASH,         // /
  SLASH_SLASH,   // //

  IDENTIFIER,    // \w[\w\d]*
  STRING,        // ".*"
  NUMBER,        // [+-]?\d*.?\d*

  TRUE,
  FALSE,
  AND,
  OR,
  IS,
  AS,
  OF,
  FOR,
  WHILE,
  IF,
  ELSE,
};

int currLine, currColumn;

class Word {
public:
  Token token;
  string value;
  int line;
  int column;

  Word(Token token, string value, int ln = -1, int col = -1)
      : token(token), value(value), line(ln >= 0 ? ln : currLine), column(col >= 0 ? col : currColumn) {}
};

bool is_digit(string str, int i) {
  return i < str.length() ? isdigit(str[i]) : false;
}

bool is_alpha(string str, int i) {
  return i < str.length() ? isalpha(str[i]) || str[i] == '_' : false;
}

bool is_space_or_eof(string str, int i) {
  return i < str.length() ? isspace(str[i]) : true;
}

bool is_sub(string str, int from, string substr) {
  return str.substr(from, substr.length()) == substr;
}

bool is_next_word(string str, int from, string word) {
  return is_sub(str, from, word) && is_space_or_eof(str, from + word.length());
}

void inc_cursor(string str, int& i, int count = 1) {
  for (; i < str.length() && count > 0; count--) {
    currColumn++;
    if (str[i] == '\n') { // TODO ignore while string parsing
      currColumn = 0;
      currLine++;
    }

    i++;
  }
}

function<optional<Word>(string str, int& i)> symbol_handler(Token token, string symbol) {
  return [token, symbol](string str, int& i) {
    optional<Word> result = nullopt;
    if (is_sub(str, i, symbol)) {
      result = Word(token, symbol);
      inc_cursor(str, i, symbol.length());
    }
    return result;
  };
}

function<optional<Word>(string str, int& i)> keyword_handler(Token token, string keyword) {
  return [token, keyword](string str, int& i) {
    optional<Word> result = nullopt;
    if (is_next_word(str, i, keyword)) {
      result = Word(token, keyword);
      inc_cursor(str, i, keyword.length());
    }
    return result;
  };
}

optional<Word> number_handler(string str, int& i) {
  optional<Word> result = nullopt;
  if (is_digit(str, i)) {
    bool has_dot = false;
    string value = "";
    int sLn = currLine, sCol = currColumn;
    while (i < str.length()) {
      if (str[i] == '.') {
        if (has_dot) {
          cout << "Too much dots for one number, I know you love it but don't overdo" << endl;
        } else {
          value += '.';
          has_dot = true;
        }
      } else if (is_digit(str, i)) {
        value += str[i];
      } else {
        break;
      }
      inc_cursor(str, i);
    }
    result = Word(Token::NUMBER, value, sLn, sCol);
  }
  return result;
}

optional<Word> identifier_handler(string str, int& i) {
  optional<Word> result = nullopt;
  if (is_alpha(str, i)) {
    string value = "";
    int sLn = currLine, sCol = currColumn;
    do {
      value += str[i];
      inc_cursor(str, i);
      if (!is_digit(str, i) && !is_alpha(str, i))
        break;
    } while (i < str.length());
    result = Word(Token::IDENTIFIER, value, sLn, sCol);
  }
  return result;
}

function<optional<Word>(string str, int& i)> tokenHandlers[] = {
    symbol_handler(Token::LEFT_PAREN, "("),
    symbol_handler(Token::RIGHT_PAREN, ")"),
    symbol_handler(Token::LEFT_BRACE, "{"),
    symbol_handler(Token::RIGHT_BRACE, "}"),
    symbol_handler(Token::COMMA, ","),
    symbol_handler(Token::BANG_EQUAL, "!="),
    symbol_handler(Token::EQUAL_EQUAL, "=="),
    symbol_handler(Token::GREATER_EQUAL, ">="),
    symbol_handler(Token::LESS_EQUAL, "<="),
    symbol_handler(Token::COLON_EQUAL, ":="),
    symbol_handler(Token::PLUS, "+"),
    symbol_handler(Token::MINUS, "-"),
    symbol_handler(Token::STAR, "*"),
    symbol_handler(Token::EQUAL, "="),
    symbol_handler(Token::BANG, "!"),
    symbol_handler(Token::GREATER, ">"),
    symbol_handler(Token::LESS, "<"),
    symbol_handler(Token::COLON, ":"),
    symbol_handler(Token::SLASH, "/"),

    keyword_handler(Token::TRUE, "true"),
    keyword_handler(Token::FALSE, "false"),
    keyword_handler(Token::AND, "and"),
    keyword_handler(Token::OR, "or"),
    keyword_handler(Token::IS, "is"),
    keyword_handler(Token::AS, "as"),
    keyword_handler(Token::OF, "of"),
    keyword_handler(Token::FOR, "for"),
    keyword_handler(Token::WHILE, "while"),
    keyword_handler(Token::IF, "if"),
    keyword_handler(Token::ELSE, "else"),

    number_handler,
    identifier_handler,
};

int main() {
  ifstream input("D:/GSU/diploma/input.txt");
  string str((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());

  vector<Word> tokens;
  int i = 0;
  currLine = currColumn = 0;
  while (i < str.length()) {
    bool handle = false;

    if (is_sub(str, i, "//")) {
      while (i < str.length() && str[i] != '\n') {
        inc_cursor(str, i);
      }
    }

    for (auto handler : tokenHandlers) {
      auto res = handler(str, i);
      if (res.has_value()) {
        tokens.emplace_back(res.value());
        cout << res.value().value << "\t" << res.value().line << "\t" << res.value().column << ";\n";
        handle = true;
        break;
      }
    }
    if (!handle) {
      inc_cursor(str, i);
    }
  }
}
