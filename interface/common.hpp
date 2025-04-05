#ifndef COMMON
#define COMMON

// the smallest functional unit of a writing system
enum Grapheme {
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

#endif // COMMON