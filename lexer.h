#pragma once
#include "stream.h"

#define LEXEME_STACK_DEPTH 256

/* EQUIVALENCE CLASSES */
#define LEXEME_DIGIT '0':\
  case '1':\
  case '2':\
  case '3':\
  case '4':\
  case '5':\
  case '6':\
  case '7':\
  case '8':\
  case '9'

#define LEXEME_NONZERO_DIGIT '1':\
  case '2':\
  case '3':\
  case '4':\
  case '5':\
  case '6':\
  case '7':\
  case '8':\
  case '9'

#define LEXEME_HEX_DIGIT '0':\
  case '1':\
  case '2':\
  case '3':\
  case '4':\
  case '5':\
  case '6':\
  case '7':\
  case '8':\
  case '9':\
  case 'a':\
  case 'b':\
  case 'c':\
  case 'd':\
  case 'e':\
  case 'f':\
  case 'A':\
  case 'B':\
  case 'C':\
  case 'D':\
  case 'E':\
  case 'F'

#define LEXEME_WHITESPACE ' ':\
  case '\t':\
  case '\n':\
  case '\r'

#define LEXEME_WHITESPACE_NO_LINE_BREAK ' ':\
  case '\t'

#define LEXEME_LINE_BREAK '\n':\
  case '\r'

struct lexeme {
  int category;
  char *content;
  int line;
  int column;
};

typedef struct lexeme lexeme;

lexeme *lexeme_new(int category, const char* content, int l, int c);

void lexeme_delete(lexeme *l);

lexeme *scan(stream *s);

enum category {
  INTEGER,
  HEX_INTEGER,
  BIN_INTEGER,
  FLOAT,
  LPAREN,
  RPAREN,
  LCBRACKET,
  RCBRACKET,
  LSQBRACKET,
  RSQBRACKET,
  LARROW,
  RARROW,
  MINUS,
  END_OF_FILE,
};
