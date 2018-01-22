#include <stdio.h>
#include "stream.h"

size_t linenumber = 1;

struct lexeme {
  int category;
  const char *value;
  int line;
  int column;
};

typedef struct lexeme lexeme;

enum id {
  LPAREN,
  RPAREN,
  LCBRACKET,
  RCBRACKET,
  LSQBRACKET,
  RSQBRACKET,
  LARROW,
  RARROW,
  MINUS,
};


// get the next lexeme
int scan(stream *s) {
  int c, d;

  // this should probably become a state machine, or use regex
 scan:
  c = sgetc(s);
  switch(c) {
  case ' ':
  case '\t':
    goto scan;
  case '\n':
  case '\r':
    linenumber++;
    goto scan;
  case '(': return LPAREN;
  case ')': return RPAREN;
  case '[': return LSQBRACKET;
  case ']': return RSQBRACKET;
  case '{': return LCBRACKET;
  case '}': return RCBRACKET;
  case '-':
    switch (d = sgetc(s)) {
    case '>': return LARROW;
    default:
      sputc(d, s);
      return MINUS;
    }
  }
}

int main(int arc, const char *argv[]) {
  stream *s = sopen(argv[1]);

  printf("%d\n", scan(s));
  printf("%d\n", scan(s));
  printf("%d\n", scan(s));
  printf("%d\n", scan(s));
  


  
  sclose(s);
  return 0;
}
