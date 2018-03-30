#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"
#include "lexer.h"
#include "parser.h"

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

  printf("%s\n", LA(&p, 0)->content);
  match(&p, LexDecInteger);
  match(&p, LexPlus);
  match(&p, LexDecInteger);
  match(&p, LexAsterisk);
  match(&p, LexIdentifier);
  match(&p, LexCaret);
  
  sclose(s);
}

int main(int arc, const char *argv[]) {
  return 0;
}
