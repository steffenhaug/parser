#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"
#include "lexer.h"
#include "parser.h"

void test_lex_str(const char* str) {
  // test lexing a constant string
  printf("lexed string: ");
  printf("\"%s\", of length %lu\n", str, strlen(str));
  stream *s = stream_fromstr(str);
  lexeme *a;
  int line = 1;
  do {
    
    // get next lexeme
    a = scan(s);
    if (a->line > line) {
      printf("\n");
      line = a->line;

    }
    printf("(lexeme) %-20s %10s (line %2d, column %2d)\n",
	   lexeme_class_tostr(a->type), a->content, a->line, a->column);
    if (a->type == EndOfFile) {
      free_lexeme(a);
      break;
  
    }
    free_lexeme(a);
  } while (1);
  sclose(s);
}

void test_parse_str(const char* str) {
  stream *s = stream_fromstr(str);
  parser p;
  init_parser(&p, s);

  if (1) // Print lookahead buffer.
    for (int i = 0; i < MAX_LOOKAHEAD; i++) {
      lexeme *a = p.lookahead[i];
      printf("(lexeme) %-20s %10s (line %2d, column %2d)\n",
	     lexeme_class_tostr(a->type), a->content, a->line, a->column);
    }

  match(&p, DecInteger);
  match(&p, Plus);
  match(&p, DecInteger);
  match(&p, Asterisk);
  match(&p, Identifier);
  match(&p, Caret);
  
  sclose(s);
}

int main(int arc, const char *argv[]) {
  test_parse_str("666 + 42 * e^(-x^2) mod 12 - 420 / 69");
  return 0;
}
